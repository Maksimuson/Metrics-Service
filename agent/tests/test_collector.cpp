#include <gtest/gtest.h>
#include <string>
#include "collector.hpp"

// ---------- parseCpuLine ----------

TEST(ParseCpuLine, ParsesValidLine) {
    CpuTimes t;
    ASSERT_TRUE(parseCpuLine("cpu  100 20 30 400 50 6 7 8 0 0", t));
    EXPECT_EQ(t.idle, 450u);   // idle 400 + iowait 50
    EXPECT_EQ(t.total, 621u);  // 100+20+30+400+50+6+7+8
}

TEST(ParseCpuLine, RejectsWrongLabel) {
    CpuTimes t;
    EXPECT_FALSE(parseCpuLine("cpu0 1 2 3 4", t));
}

TEST(ParseCpuLine, RejectsTooFewNumbers) {
    CpuTimes t;
    EXPECT_FALSE(parseCpuLine("cpu 1 2 3", t));
}

// ---------- cpuPercent ----------

TEST(CpuPercent, HalfLoaded) {
    CpuTimes a{100, 200};  // idle, total
    CpuTimes b{150, 300};
    EXPECT_DOUBLE_EQ(cpuPercent(a, b), 50.0);
}

TEST(CpuPercent, FullyIdle) {
    CpuTimes a{100, 200};
    CpuTimes b{200, 300};
    EXPECT_DOUBLE_EQ(cpuPercent(a, b), 0.0);
}

TEST(CpuPercent, FullyBusy) {
    CpuTimes a{100, 200};
    CpuTimes b{100, 300};
    EXPECT_DOUBLE_EQ(cpuPercent(a, b), 100.0);
}

TEST(CpuPercent, NoChangeGivesZero) {
    CpuTimes a{100, 200};
    EXPECT_DOUBLE_EQ(cpuPercent(a, a), 0.0);
}

TEST(CpuPercent, CounterResetGivesZero) {
    CpuTimes a{500, 1000};
    CpuTimes b{100, 200};  // счётчики стали меньше, например после перезагрузки
    EXPECT_DOUBLE_EQ(cpuPercent(a, b), 0.0);
}

// ---------- parseMemInfo ----------

TEST(ParseMemInfo, ParsesValidText) {
    std::string text =
        "MemTotal:        8192000 kB\n"
        "MemFree:         1000000 kB\n"
        "MemAvailable:    4096000 kB\n";
    MemInfo m;
    ASSERT_TRUE(parseMemInfo(text, m));
    EXPECT_EQ(m.totalMb, 8000);
    EXPECT_EQ(m.usedMb, 4000);
}

TEST(ParseMemInfo, MissingAvailableFails) {
    std::string text =
        "MemTotal:        8192000 kB\n"
        "MemFree:         1000000 kB\n";
    MemInfo m;
    EXPECT_FALSE(parseMemInfo(text, m));
}

TEST(ParseMemInfo, GarbageFails) {
    MemInfo m;
    EXPECT_FALSE(parseMemInfo("hello world", m));
}

TEST(ParseMemInfo, AvailableGreaterThanTotalFails) {
    std::string text =
        "MemTotal:        1000 kB\n"
        "MemAvailable:    2000 kB\n";
    MemInfo m;
    EXPECT_FALSE(parseMemInfo(text, m));
}