//
// Created by wangzhen on 18-6-19.
//

#include "Timer.h"
#include <chrono>

USING_UNIT_NAMESPACE

std::unique_ptr<NanoTimer> NanoTimer::ptr =  std::unique_ptr<NanoTimer>(new NanoTimer());

NanoTimer* NanoTimer::GetInstance()
{
    return ptr.get();
}

inline std::chrono::steady_clock::time_point GetTimeNow()
{
    timespec tp = {};
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return std::chrono::steady_clock::time_point(
            std::chrono::steady_clock::duration(
                    std::chrono::seconds(tp.tv_sec)
                    + std::chrono::nanoseconds(tp.tv_nsec)
                    )
            );
}

NanoTimer::NanoTimer()
{
    secDiff = GetLocalDiff();
}

long NanoTimer::GetNano() const
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
            GetTimeNow().time_since_epoch()
            ).count() + secDiff;
}

long NanoTimer::GetLocalDiff()
{
    long unix_second_num = std::chrono::seconds(std::time(nullptr)).count();
    long tick_second_num = std::chrono::duration_cast<std::chrono::seconds>(
            GetTimeNow().time_since_epoch()
    ).count();
    return (unix_second_num - tick_second_num) * NANOSECONDS_PER_SECOND;
}