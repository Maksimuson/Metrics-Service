#pragma once
#include <cstdlib>
#include <string>

struct Config {
    std::string dbHost;
    unsigned int dbPort;
    std::string dbUser;
    std::string dbPass;
    std::string dbName;
    int apiPort;
};

inline std::string envOr(const char* name, const char* fallback) {
    const char* v = std::getenv(name);
    return (v && *v) ? v : fallback;
}

inline int envInt(const char* name, int fallback) {
    int v = std::atoi(envOr(name, "").c_str());
    return v > 0 ? v : fallback;
}

inline const Config& config() {
    static const Config c{
        envOr("DB_HOST", "127.0.0.1"),
        static_cast<unsigned int>(envInt("DB_PORT", 3306)),
        envOr("DB_USER", "app"),
        envOr("DB_PASS", ""),
        envOr("DB_NAME", "metrics"),
        envInt("PORT", 8080)
    };
    return c;
}