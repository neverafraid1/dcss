//
// Created by wangzhen on 18-6-28.
//

#include "DCSSStrategyUtil.h"
#include "Timer.h"

DCSSStrategyUtil::DCSSStrategyUtil(const std::string& strategyName)
: StrategyUtil(strategyName), mStrategyName(strategyName), mCurNano(0), mMdNano(0)
{}

long DCSSStrategyUtil::GetNano()
{
    return GetNanoTime();
}

static const char TimeFormat[] = "%Y-%m-%d %H:%M:%S";

std::string DCSSStrategyUtil::GetTime()
{
    return UnitEngine::ParseNano(GetNanoTime(), TimeFormat);
}

long DCSSStrategyUtil::ParseTime(const std::string& timeStr)
{
    return UnitEngine::ParseTime(timeStr.c_str(), TimeFormat);
}

std::string DCSSStrategyUtil::ParseNano(long nano)
{
    return UnitEngine::ParseNano(nano, TimeFormat);
}

std::string DCSSStrategyUtil::GetName() const
{
    return mStrategyName;
}

int DCSSStrategyUtil::InsertLimitOrder(short source, const DCSSSymbolField& symbol, double price, double volume,
        TradeTypeType tradeType)
{
    if (tradeType != BUY && tradeType != SELL)
    {
        return -1;
    }
    int rid = GetRid();

    DCSSReqInsertOrderField req = {};
    req.Symbol = symbol;
    req.Price = price;
    req.Amount = volume;
    req.TradeType = tradeType;

    WriteFrameExtra(&req, sizeof(DCSSReqInsertOrderField), source, MSG_TYPE_REQ_ORDER_INSERT, rid, mMdNano);
    return rid;
}

int DCSSStrategyUtil::InsertMarketOrder(short source, const DCSSSymbolField& symbol, double volume,
        TradeTypeType tradeType)
{
    if(tradeType != BUY_MARKET && tradeType != SELL_MARKET)
    {
        return -1;
    }
    int rid = GetRid();
    DCSSReqInsertOrderField req = {};
    req.Symbol = symbol;
    req.Amount = volume;
    req.TradeType = tradeType;

    WriteFrameExtra(&req, sizeof(req), source, MSG_TYPE_REQ_ORDER_INSERT, rid, mMdNano);
}

int DCSSStrategyUtil::CancelOrder(short source, const DCSSSymbolField& symbol, long orderId)
{
    int rid = GetRid();

    DCSSReqCancelOrderField req = {};
    req.Symbol = symbol;
    req.OrderID[0] = orderId;
    WriteFrame(&req, sizeof(DCSSReqCancelOrderField), source, MSG_TYPE_REQ_ORDER_ACTION, rid);

    return rid;
}