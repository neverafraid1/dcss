//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGYUTIL_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGYUTIL_H

#include <string>
#include "UnitDeclare.h"

UNIT_NAMESPACE_START

/**
 * interface class
 * may consider move out engine in future
 */
class IStrategyUtil
{
public:
    virtual bool RegisterStrategy(int& ridStart, int& ridEnd) = 0;

    virtual bool TdConnect(const std::string& config) = 0;

    virtual bool MdSubscribeTicker(const std::string& tickers, uint8_t source) = 0;

    virtual bool MdSubscribeKline(const std::string& symbol, int klineType, uint8_t source) = 0;

    virtual bool MdSubscribeDepth(const std::string& symbol, int depth, uint8_t source) = 0;

    virtual ~IStrategyUtil() = default;
};

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGYUTIL_H
