//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGYUTIL_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGYUTIL_H

#include <vector>
#include <string>
#include "DataStruct.h"

/**
 * interface class
 */
class IStrategyUtil
{
public:
    virtual bool RegisterStrategy(int& ridStart, int& ridEnd) = 0;

    virtual bool TdConnect(short source) = 0;

    virtual bool MdSubscribe(const std::vector<DCSSSymbolField>& tickers, short source) = 0;

    virtual bool MdSubscribeKline(const DCSSSymbolField& symbol, KlineTypeType klineType, short source) = 0;

    virtual bool MdSubscribeDepth(const DCSSSymbolField& symbol, int depth, short source) = 0;

    virtual ~IStrategyUtil() = default;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGYUTIL_H
