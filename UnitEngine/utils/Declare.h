//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_DECLARE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_DECLARE_H

#include <string>
#include <vector>
#include <map>
#include <memory>

typedef std::pair<int, int> IntPair;

typedef std::shared_ptr<std::thread> ThreadPtr;

#define DECLARE_PTR(x) typedef std::shared_ptr<x> x##Ptr

#define ADDRESS_ADD(x, delta) (void*)((uintptr_t)x + delta)

#define DCSS_FOLDER "/shared/dcss/"
#define DCSS_UNIT_FOLDER DCSS_FOLDER "unit/"
#define DCSS_SOCKET_FOLDER DCSS_FOLDER "socket/"
#define DCSS_LOG_FOLDER DCSS_FOLDER "log/"
#define PAGED_UNIT_FOLDER DCSS_UNIT_FOLDER "system/"
#define PAGED_UNIT_NAME "SYSTEM"

#define

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_DECLARE_H
