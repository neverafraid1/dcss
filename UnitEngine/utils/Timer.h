//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_TIMER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_TIMER_H

#include <memory>
#include <cstring>
#include "UnitDeclare.h"

UNIT_NAMESPACE_START

class NanoTimer
{
public:
    static NanoTimer* GetInstance();

    long GetNano() const;

    NanoTimer& operator=(const NanoTimer& source) = delete;

private:
    NanoTimer();

    long GetLocalDiff();

private:
    static std::unique_ptr<NanoTimer> ptr;
    long secDiff;
};

/**
 *
 * @return current nano time in long
 */
inline long GetNanoTime()
{
    return NanoTimer::GetInstance()->GetNano();
}

/**
 * parse struct tm to nano time
 * @param _tm ctime struct
 * @return nano time in long
 */
inline long ParseTm(struct tm& _tm)
{
    return timelocal(&_tm) * NANOSECONDS_PER_SECOND;
}

/**
 * parse string time to nano time
 * @param timeStr string-format time
 * @param format eg: %Y%m%d-%H:%M:%S
 * @return nano time in long
 */
inline long ParseTime(const char* timeStr, const char* format)
{
    struct tm _tm;
    strptime(timeStr, format, &_tm);
    return ParseTm(_tm);
}

/**
 * long time to string with format
 * @param nano nano time in long
 * @param format eg: %Y%m%d-%H:%M:%S
 * @return string-format time
 */
inline std::string ParseNano(long nano, const char* format)
{
    if (nano <= 0)
        return std::string("NULL");

    nano /= NANOSECONDS_PER_SECOND;
    struct tm* dt;
    char buffer[30];
    bzero(buffer, sizeof(buffer));
    dt = localtime(&nano);
    strftime(buffer, sizeof(buffer), format, dt);
    return std::string(buffer);
}

/*
#if defined(__i386__)
static __inline__ unsigned long long GetCycleCount()
{
    unsigned long long int x;
    __asm__ volatile("rdtsc":"=A"(x));
    return x;
}
#elif defined(__x86_64__)
static __inline__ unsigned long long GetCycleCount()
{
    unsigned hi, lo;
    __asm__ volatile("rdtsc":"=a"(lo),"=d"(hi));
    return ((unsigned long long)lo)|(((unsigned long long)hi)<<32);
}
#endif
 */

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_TIMER_H
