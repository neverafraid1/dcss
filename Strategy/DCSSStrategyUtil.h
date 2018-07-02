//
// Created by wangzhen on 18-6-28.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_DCSSSTRATEGYUTIL_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_DCSSSTRATEGYUTIL_H

#include "Declare.h"
#include "StrategyUtil.h"

/**
 * strategy utilities include:
 * 1. all unit writing (insert order, cancel order, qry, etc)
 * 2. request id
 * 3. call back insert process
 */
class DCSSStrategyUtil : public StrategyUtil
{
public:
    DCSSStrategyUtil(const std::string& strategyName);
    ~DCSSStrategyUtil() = default;

    int InsertLimitOrder(short source, const DCSSSymbolField& symbol, double price, double volume, TradeTypeType tradeType);

    int InsertMarketOrder(short source, const DCSSSymbolField& symbol, double volume, TradeTypeType tradeType);

    int CancelOrder(short source, int orderId);


    /*get nano time*/
    long GetNano();
    /*get string time YYYY-MM-DD HH:MM:SS*/
    std::string GetTime();
    /*parse time*/
    long ParseTime(const std::string& timeStr);
    /*parse nano*/
    std::string ParseNano(long nano);

private:
    long mMdNano;
    long mCurNano;
    std::string mStrategyName;
};

DECLARE_PTR(DCSSStrategyUtil);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_DCSSSTRATEGYUTIL_H
