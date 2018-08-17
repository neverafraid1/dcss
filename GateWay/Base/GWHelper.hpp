//
// Created by wangzhen on 18-8-14.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_GWHELPER_HPP
#define DIGITALCURRENCYSTRATEGYSYSTEM_GWHELPER_HPP

#include <unordered_map>
#include <string>
#include <algorithm>

#include "Constants.h"


std::unordered_map<KlineType, std::string> BinanceKlineStringMap = {
        {KlineType::Min1,   "1m"},
        {KlineType::Min3,   "3m"},
        {KlineType::Min5,   "5m"},
        {KlineType::Min15,  "15m"},
        {KlineType::Min30,  "30m"},
        {KlineType::Hour1,  "1h"},
        {KlineType::Hour2,  "2h"},
        {KlineType::Hour4,  "4h"},
        {KlineType::Hour6,  "6h"},
        {KlineType::Hour8,  "8h"},
        {KlineType::Hour12, "12h"},
        {KlineType::Day1,   "1d"},
        {KlineType::Day3,   "3d"},
        {KlineType::Week1,  "1w"},
        {KlineType::Month1, "1M"}
};

std::unordered_map<std::string, KlineType> BinanceStringKlineMap = {
        {"1m",  KlineType::Min1},
        {"3m",  KlineType::Min3},
        {"5m",  KlineType::Min5},
        {"15m", KlineType::Min15},
        {"30m", KlineType::Min30},
        {"1h",  KlineType::Hour1},
        {"2h",  KlineType::Hour2},
        {"4h",  KlineType::Hour4},
        {"6h",  KlineType::Hour6},
        {"8h",  KlineType::Hour8},
        {"12h", KlineType::Hour12},
        {"1d",  KlineType::Day1},
        {"3d",  KlineType::Day3},
        {"1w",  KlineType::Week1},
        {"1M",  KlineType::Month1}
};


// btc_ltc --> BTCLTC
inline std::string GetBinaSymbol(const std::string& symbol)
{
    std::string::size_type index = symbol.find('_');
    std::string tmp = symbol.substr(0, index) += ((symbol.substr(index + 1, symbol.length() - index - 1)));
    std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
    return std::move(tmp);
}

inline std::string GetSymbolFromBina(const std::string& symbol)
{
    return symbol;
}

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_GWHELPER_HPP
