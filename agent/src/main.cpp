#include <atomic>
#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <thread>
#include "collector.hpp"
#include "config.hpp"
#include "sender.hpp"

static std::atomic<bool> running{true};

static void onSignal(int) { running = false; }

// Спим интервал, но просыпаемся сразу, если пришёл Ctrl+C
static void sleepInterruptible(int seconds) {
    for (int i = 0; i < seconds * 10 && running; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    const Config& cfg = config();
    std::cout << "Agent started: host=" << cfg.hostName
              << " api=" << cfg.apiHost << ":" << cfg.apiPort
              << " interval=" << cfg.intervalSec << "s\n";

    CpuTimes prev;
    if (!readCpuTimes(prev)) {
        std::cerr << "Cannot read /proc/stat\n";
        return 1;
    }

    while (true) {
        sleepInterruptible(cfg.intervalSec);
        if (!running) break;

        CpuTimes cur;
        MemInfo mem;
        if (!readCpuTimes(cur) || !readMemInfo(mem)) {
            std::cerr << "Cannot read /proc, skipping this round\n";
            continue;
        }

        double cpu = cpuPercent(prev, cur);
        prev = cur;

        std::string error;
        if (sendMetric(cfg.apiHost, cfg.apiPort, cfg.hostName,
                       cpu, mem.usedMb, mem.totalMb, error)) {
            std::cout << std::fixed << std::setprecision(1)
                      << "sent: cpu=" << cpu << "% ram="
                      << mem.usedMb << "/" << mem.totalMb << " MB\n";
        } else {
            std::cerr << "send failed: " << error << "\n";
        }
    }

    std::cout << "Agent stopped\n";
    return 0;
}