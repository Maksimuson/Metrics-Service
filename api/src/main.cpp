#include <httplib.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include "db.hpp"

using json = nlohmann::json;

int main() {
    std::string dbError;
    if (!dbCheck(dbError)) {
        std::cerr << "DB connection failed: " << dbError << "\n";
        return 1;
    }
    std::cout << "Connected to MySQL\n";

    httplib::Server svr;

    svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("ok", "text/plain");
    });

    svr.Post("/ingest", [](const httplib::Request& req, httplib::Response& res) {
        json body = json::parse(req.body, nullptr, false);
        if (body.is_discarded() || !body.is_object()) {
            res.status = 400;
            res.set_content(R"({"error":"invalid json"})", "application/json");
            return;
        }

        if (!body.contains("host") || !body["host"].is_string() ||
            !body.contains("cpu_percent") || !body["cpu_percent"].is_number() ||
            !body.contains("ram_used_mb") || !body["ram_used_mb"].is_number_integer() ||
            !body.contains("ram_total_mb") || !body["ram_total_mb"].is_number_integer()) {
            res.status = 400;
            res.set_content(R"({"error":"missing or wrong fields"})", "application/json");
            return;
        }

        std::string host = body["host"];
        double cpu = body["cpu_percent"];
        long long ramUsed = body["ram_used_mb"];
        long long ramTotal = body["ram_total_mb"];

        if (host.empty() || host.size() > 255 ||
            cpu < 0 || cpu > 100 || ramUsed < 0 || ramTotal <= 0 || ramUsed > ramTotal) {
            res.status = 400;
            res.set_content(R"({"error":"values out of range"})", "application/json");
            return;
        }

        std::string error;
        if (!saveMetric(host, cpu, ramUsed, ramTotal, error)) {
            std::cerr << "DB error: " << error << "\n";
            res.status = 500;
            res.set_content(R"({"error":"database error"})", "application/json");
            return;
        }

        res.status = 201;
        res.set_content(R"({"status":"saved"})", "application/json");
    });

    std::cout << "API started on port 8080\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}