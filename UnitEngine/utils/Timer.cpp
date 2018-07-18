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

long NanoTimer::GetNano() const
{
    long nano = std::chrono::duration_cast<std::chrono::nanoseconds>(
            GetTimeNow().time_since_epoch()
            ).count();
    return nano + secDiff;
}