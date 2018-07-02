//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_TIMER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_TIMER_H

#include <memory>

const long MILLISECONDS_PER_SECOND = 1000;
const long MICROSECONDS_PER_MILLISECOND = 1000;
const long NANOSECONDS_PER_MICROSECOND = 1000;

const long MICROSECONDS_PER_SECOND = MICROSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND;
const long NANOSECONDS_PER_MILLISECOND = NANOSECONDS_PER_MICROSECOND * MICROSECONDS_PER_MILLISECOND;
const long NANOSECONDS_PER_SECOND = NANOSECONDS_PER_MILLISECOND * MILLISECONDS_PER_SECOND;

const int  SECONDS_PER_MINUTE = 60;
const int  MINUTES_PER_HOUR = 60;
const int  HOURS_PER_DAY = 24;
const int  SECONDS_PER_HOUR = SECONDS_PER_MINUTE * MINUTES_PER_HOUR;

const long MILLISECONDS_PER_MINUTE = MILLISECONDS_PER_SECOND * SECONDS_PER_MINUTE;
const long NANOSECONDS_PER_MINUTE = NANOSECONDS_PER_SECOND * SECONDS_PER_MINUTE;
const long NANOSECONDS_PER_HOUR = NANOSECONDS_PER_SECOND * SECONDS_PER_HOUR;
const long NANOSECONDS_PER_DAY = NANOSECONDS_PER_HOUR * HOURS_PER_DAY;

namespace MemoryBuffer
{

class NanoTimer
{
public:
    static NanoTimer* GetInstance();

    long GetNano() const;

private:
    NanoTimer() = default;
    NanoTimer& operator=(const NanoTimer& source) = delete;

private:
    static std::shared_ptr<NanoTimer> ptr;
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
}

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_TIMER_H