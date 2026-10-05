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

     svr.Get("/hosts", [](const httplib::Request&, httplib::Response& res) {
        std::vector<std::string> hosts;
        std::string error;
        if (!listHosts(hosts, error)) {
            std::cerr << "DB error: " << error << "\n";
            res.status = 500;
            res.set_content(R"({"error":"database error"})", "application/json");
            return;
        }
        res.set_content(json(hosts).dump(), "application/json");
    });

        svr.Get("/metrics", [](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("host")) {
            res.status = 400;
            res.set_content(R"({"error":"host is required"})", "application/json");
            return;
        }
        std::string host = req.get_param_value("host");
        std::string from = req.has_param("from") ? req.get_param_value("from") : "";
        std::string to = req.has_param("to") ? req.get_param_value("to") : "";

        int limit = 100;
        if (req.has_param("limit")) {
            try {
                limit = std::stoi(req.get_param_value("limit"));
            } catch (...) {
                res.status = 400;
                res.set_content(R"({"error":"limit must be a number"})", "application/json");
                return;
            }
        }
        if (limit < 1) limit = 1;
        if (limit > 1000) limit = 1000;

        std::vector<MetricRow> rows;
        std::string error;
        if (!queryMetrics(host, from, to, limit, rows, error)) {
            std::cerr << "DB error: " << error << "\n";
            res.status = 500;
            res.set_content(R"({"error":"database error"})", "application/json");
            return;
        }

        json arr = json::array();
        for (const auto& r : rows) {
            arr.push_back({{"ts", r.ts},
                           {"cpu_percent", r.cpu},
                           {"ram_used_mb", r.ramUsed},
                           {"ram_total_mb", r.ramTotal}});
        }
        res.set_content(arr.dump(), "application/json");
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