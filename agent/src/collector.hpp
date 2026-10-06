#pragma once
#include <string>

struct CpuTimes 
{
    unsigned long long idle=0;
    unsigned long long total=0;
};

bool parseCpuLine(const std::string& line, CpuTimes& out);

bool readCpuTimes(CpuTimes& out);

double cpuPercent(const CpuTimes& prev, const CpuTimes& cur);