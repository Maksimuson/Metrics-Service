#pragma once 
#include <string>
#include <cstdlib>
#include <unistd.h>

struct Config 
{
    std::string apiHost;
    int apiPort;
    int intervalSec;
    std::string hostName;
};

inline std::string envOr (const char* name, const std::string& fallback)
{
    const char* v = std::getenv(name);
    return (v && *v) ? std::string(v) : fallback; 
}

inline  int envInt(const char* name, int fallback)
{
    int v = std::atoi(envOr(name, "").c_str());
    return (v > 0) ? v : fallback;
}

inline std::string defaultHostName()
{
    char buf[256];
    if (gethostname(buf, sizeof(buf)) == 0)
    {
        buf[255] = '\0';
        return buf;
    }
    return "unknown";
}

inline const Config& config()
{
    static const Config c
    {
        envOr("API_HOST", "127.0.0.1"),
        envInt("API_PORT", 8080),
        envInt("INTERVAL_SEC", 5),
        envOr("HOST_NAME", defaultHostName())
    };
    return c;
}