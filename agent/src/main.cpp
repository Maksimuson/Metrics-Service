#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include "collector.hpp"

int main() {
    CpuTimes prev;
    if (!readCpuTimes(prev)) {
        std::cerr << "Cannot read /proc/stat\n";
        return 1;
    }

    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        CpuTimes cur;
        MemInfo mem;
        if (!readCpuTimes(cur)) {
            std::cerr << "Cannot read /proc/stat\n";
            return 1;
        }
        if (!readMemInfo(mem)) {
            std::cerr << "Cannot read /proc/meminfo\n";
            return 1;
        }

        std::cout << std::fixed << std::setprecision(1)
                  << "CPU: " << cpuPercent(prev, cur) << "%  "
                  << "RAM: " << mem.usedMb << "/" << mem.totalMb << " MB\n";
        prev = cur;
    }
    return 0;
}