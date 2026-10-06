#include "sender.hpp"
#include <httplib.h>
#include <nlohmann/json.hpp>

bool sendMetric( const std::string& apiHost, int apiPort,
                const std::string& hostName,
                double cpu, long long ramUsed, long long ramTotal,
                std::string& error)
{
    nlohmann::json body = {
        {"host", hostName},
        {"cpu_percent", cpu},
        {"ram_used_mb", ramUsed},
        {"ram_total_mb", ramTotal}
    };

    httplib::Client cli(apiHost, apiPort);
    cli.set_connection_timeout(3);
    cli.set_read_timeout(3);
    
    auto res = cli.Post("/ingest", body.dump(), "application/json");
    if (!res) {
        error = httplib::to_string(res.error());
        return false;
    }
    if (res->status !=201)
    {
        error = "API returned " + std::to_string(res->status) + ": " + res->body;
        return false;
    }
    return true;
}