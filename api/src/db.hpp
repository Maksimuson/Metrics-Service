#pragma once 
#include <string>
#include <vector>

bool dbCheck(std::string& error);

bool saveMetric (const std::string& host, double cpu,
                long long ramUsed, long long ramTotal,
                 std::string& error);

bool listHosts (std::vector<std::string>& out, std::string& error);

struct MetricRow
{
    std::string ts;
    double cpu;
    long long ramUsed;
    long long ramTotal;
};

bool queryMetrics(const std::string& host,
                  const std::string& from,
                  const std::string& to,
                  int limit,
                  std::vector<MetricRow>& out,
                  std::string& error);