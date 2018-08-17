//
// Created by wangzhen on 18-7-2.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_UTIL_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_UTIL_H

#include "Constants.h"
#include <string>

typedef std::pair<std::string, std::string> UnitPair;

inline UnitPair GetMdUnitPair(uint8_t source)
{
    switch (source)
    {
    case EXCHANGE_OKCOIN:
        return {"/home/wangzhen/dcss/unit/MD/OKEX", "MD_OKEX"};
    default:
        return {"", ""};
    }
}

std::string GetMdString(uint8_t source)
{
    switch (source)
    {
    case EXCHANGE_OKCOIN:
        return "okex";
    default:
        return "";
    }
}

inline UnitPair GetTdUnitPair(uint8_t source)
{
    switch (source)
    {
    case EXCHANGE_OKCOIN:
        return {"/shared/dcss/unit/TD/OKEX", "TD_OKEX"};
    default:
        return {"", ""};
    }
}

inline UnitPair GetStrategyTdUnitPair()
{
    return {"/shared/dcss/unit", "TD"};
}

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_UTIL_H
