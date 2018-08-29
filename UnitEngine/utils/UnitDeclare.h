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
