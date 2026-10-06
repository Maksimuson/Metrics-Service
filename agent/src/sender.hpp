#pragma once
#include <string>

bool sendMetric( const std::string& apiHost, int apiPort,
                 const std::string& hostName, 
                double cpu, long long ramUsed, long long ramTotal,
                std::string& error);
                