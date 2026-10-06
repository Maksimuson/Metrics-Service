#include <iostream>
#include <chrono>
#include <iomanip>
#include "collector.hpp"
#include <thread>

int main()
{
    CpuTimes prev;
    if (!readCpuTimes(prev)) 
    {
        std::cerr << "Cannot read /proc/stat\n";
        return 1;
    }

    for (int i=0; i <5; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        CpuTimes cur;
        if (!readCpuTimes(cur))
        {
            std::cerr << "Cannot read /proc/stat\n";
            return 1;
        }

        std::cout << std::fixed << std:: setprecision(1)
                    << "CPU: " << cpuPercent(prev, cur) << "%\n";
        prev = cur;
    }
    return 0;

}


