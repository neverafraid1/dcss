//
// Created by wangzhen on 18-7-4.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_UNITDECLARE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_UNITDECLARE_H

#include <unistd.h>
#include <vector>
#include <map>
#include <array>
#include <memory>
#include <cstdint>
#include <thread>

#define MAX_UNIT_NAME_LENGTH    30
#define NAX_UNIT_FOLDER_LENGTH  100

#define TIME_TO_LAST       -1
#define TIME_FROM_FIRST     0

const size_t KB = 1024;
const size_t MB = KB * KB;
const size_t UNIT_PAGE_SIZE = 64 * MB;
const size_t PAGE_MIN_HEADROOM = 2 * MB;

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

#define UNIT_NAMESPACE_START namespace UnitEngine {
#define UNIT_NAMESPACE_END };
#define USING_UNIT_NAMESPACE using namespace UnitEngine;

#define DECLARE_PTR(x) typedef std::shared_ptr<x> x##Ptr;
#define PRE_DECLARE_PTR(x) class x; DECLARE_PTR(x)

#define DCSS_FOLDER "/shared/dcss/"
#define DCSS_UNIT_FOLDER DCSS_FOLDER "unit/"
#define DCSS_SOCKET_FOLDER DCSS_FOLDER "socket/"
#define STRATEGY_BASE_FOLDER DCSS_UNIT_FOLDER "strategy/"
#define DCSS_LOG_FOLDER DCSS_FOLDER "log/"
#define PAGED_UNIT_FOLDER DCSS_UNIT_FOLDER "system/"
#define PAGED_UNIT_NAME "SYSTEM"

#define ADDRESS_ADD(x, delta) (void*)((uintptr_t)x + delta)

typedef std::pair<int, int> IntPair;

typedef std::shared_ptr<std::thread> ThreadPtr;

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_UNITDECLARE_H
