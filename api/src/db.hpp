#pragma once 
#include <string>
#include <vector>

bool dbCheck(std::string& error);

bool saveMetric (const std::string& host, double cpu,
                long long ramUsed, long long ramTotal,
                 std::string& error);
                 
bool listHosts (std::vector<std::string>& out, std::string& error);