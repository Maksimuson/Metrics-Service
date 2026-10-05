#pragma once 
#include <string>

bool dbCheck(std::string& error);

bool saveMetric (const std::string& host, double cpu,
                long long ramUsed, long long ramTotal,
                 std::string& error);