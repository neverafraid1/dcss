//
// Created by wangzhen on 18-6-28.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_DCSSSTRATEGYUTIL_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_DCSSSTRATEGYUTIL_H

#include "UnitDeclare.h"
#include "StrategyUtil.h"

USING_UNIT_NAMESPACE

/**
 * strategy utilities include:
 * 1. all unit writing (insert order, cancel order, qry, etc)
 * 2. request id
 * 3. call back insert process
 */
class DCSSStrategyUtil : public StrategyUtil
{
public:
    explicit DCSSStrategyUtil(const std::string& strategyName);
    ~DCSSStrategyUtil() override = default;

    int InsertLimitOrder(short source, const DCSSSymbolField& symbol, double price, double volume, TradeTypeType tradeType);

    int InsertMarketOrder(short source, const DCSSSymbolField& symbol, double volume, TradeTypeType tradeType);

    int CancelOrder(short source, const DCSSSymbolField& symbol, long orderId);


    /*get nano time*/
    long GetNano();
    /*get string time YYYY-MM-DD HH:MM:SS*/
    std::string GetTime();
    /*parse time*/
    long ParseTime(const std::string& timeStr);
    /*parse nano*/
    std::string ParseNano(long nano);

    std::string GetName() const;

    void SetMdNano(long curTime) { mMdNano = curTime;}

private:
    long mMdNano;
    long mCurNano;
    std::string mStrategyName;
};

DECLARE_PTR(DCSSStrategyUtil);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_DCSSSTRATEGYUTIL_H
