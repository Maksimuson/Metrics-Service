#include "collector.hpp"
#include <fstream>
#include <sstream>

bool parseCpuLine(const std::string& line, CpuTimes& out)
{
    std::istringstream in(line);
    std::string label;
    in >> label;
    if (label != "cpu") return false;

    // user nice system idle iowait irq softirq steal
    unsigned long long v[8] ={0};
    for (int i = 0; i < 8; ++i) 
    {
        if (!(in>>v[i])) 
        {
            if (i < 4) return false;
            break;
        }
    }
    out.idle = v[3]+v[4];
    out.total = 0;
    for (int i = 0; i < 8; ++i) out.total += v[i];
    return true;    
}

bool readCpuTimes(CpuTimes& out)
{
    std::ifstream f("/proc/stat");
    std::string line;
    if (!f || !std::getline(f, line)) return false;
    return parseCpuLine(line, out);
}

double cpuPercent(const CpuTimes& prev, const CpuTimes& cur)
{
    if (cur.total <= prev.total || cur.idle < prev.idle) return 0.0;

    double dTotal = static_cast<double> (cur.total - prev.total);
    double dIdle = static_cast<double> (cur.idle - prev.idle);
    double usage = 100.0 * (1.0 - dIdle / dTotal);

    if (usage < 0.0) usage = 0.0;
    if (usage > 100.0) usage = 100.0;
    return usage;
}


bool parseMemInfo(const std::string& text, MemInfo& out)
{

    std::istringstream in(text);
    std::string line;
    long long totalKb = -1;
    long long availKb = -1;

    while (std::getline(in, line))
    {
        std::istringstream ls(line);
        std::string key;
        long long value=0;
        if(!(ls >> key >> value)) continue;

        if(key=="MemTotal:") totalKb = value;
        else if(key == "MemAvailable:") availKb = value;
    }

    if(totalKb <= 0 || availKb < 0 || availKb > totalKb) return false;

    out.totalMb = totalKb / 1024;
    out.usedMb = (totalKb - availKb) / 1024;
    return true;
}

bool readMemInfo(MemInfo& out)
{
    std::ifstream f("/proc/meminfo");
    if(!f) return false;

    std::string text((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return parseMemInfo(text, out);
}
