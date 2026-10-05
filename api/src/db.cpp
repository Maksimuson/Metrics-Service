#include "db.hpp"
#include <mysql/mysql.h>
#include <cstring>

static MYSQL* connectDb(std::string& error) {
    MYSQL* conn = mysql_init(nullptr);
    if (!mysql_real_connect(conn, "127.0.0.1", "app", "apppass",
                            "metrics", 3306, nullptr, 0)) {
        error = mysql_error(conn);
        mysql_close(conn);
        return nullptr;
    }
    return conn;
}

static bool execPrepared(MYSQL* conn, const char* sql,
                         MYSQL_BIND* binds, std::string& error) {
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        error = "stmt init failed";
        return false;
    }
    bool ok = mysql_stmt_prepare(stmt, sql, std::strlen(sql)) == 0
           && mysql_stmt_bind_param(stmt, binds) == 0
           && mysql_stmt_execute(stmt) == 0;
    if (!ok) error = mysql_stmt_error(stmt);
    mysql_stmt_close(stmt);
    return ok;
}

bool dbCheck(std::string& error) {
    MYSQL* conn = connectDb(error);
    if (!conn) return false;
    mysql_close(conn);
    return true;
}

bool saveMetric(const std::string& host, double cpu,
                long long ramUsed, long long ramTotal,
                std::string& error) {
    MYSQL* conn = connectDb(error);
    if (!conn) return false;

    unsigned long hostLen = host.size();
    MYSQL_BIND hostBind[1];
    std::memset(hostBind, 0, sizeof(hostBind));
    hostBind[0].buffer_type = MYSQL_TYPE_STRING;
    hostBind[0].buffer = const_cast<char*>(host.c_str());
    hostBind[0].buffer_length = hostLen;
    hostBind[0].length = &hostLen;

    // 1) создаём хост, если его ещё нет
    bool ok = execPrepared(conn,
        "INSERT INTO hosts (name) VALUES (?) "
        "ON DUPLICATE KEY UPDATE name = name",
        hostBind, error);

    // 2) записываем замер
    if (ok) {
        MYSQL_BIND b[4];
        std::memset(b, 0, sizeof(b));
        b[0].buffer_type = MYSQL_TYPE_DOUBLE;
        b[0].buffer = &cpu;
        b[1].buffer_type = MYSQL_TYPE_LONGLONG;
        b[1].buffer = &ramUsed;
        b[2].buffer_type = MYSQL_TYPE_LONGLONG;
        b[2].buffer = &ramTotal;
        b[3] = hostBind[0];

        ok = execPrepared(conn,
            "INSERT INTO metrics (host_id, ts, cpu_percent, ram_used_mb, ram_total_mb) "
            "SELECT id, NOW(), ?, ?, ? FROM hosts WHERE name = ?",
            b, error);
    }

    mysql_close(conn);
    return ok;
}