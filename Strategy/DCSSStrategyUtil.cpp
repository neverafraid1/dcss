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
    return MemoryBuffer::GetNanoTime();
}

static const char TimeFormat[] = "%Y-%m-%d %H:%M:%S";

std::string DCSSStrategyUtil::GetTime()
{
    return MemoryBuffer::ParseNano(MemoryBuffer::GetNanoTime(), TimeFormat);
}

long DCSSStrategyUtil::ParseTime(const std::string& timeStr)
{
    return MemoryBuffer::ParseTime(timeStr.c_str(), TimeFormat);
}

std::string DCSSStrategyUtil::ParseNano(long nano)
{
    return MemoryBuffer::ParseNano(nano, TimeFormat);
}

int DCSSStrategyUtil::InsertLimitOrder(short source, const DCSSSymbolField& symbol, double price, double volume,
        TradeTypeType tradeType)
{
    if (tradeType != BUY && tradeType != SELL)
    {
        // TODO
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
        //TODO
        return -1;
    }
    int rid = GetRid();
    DCSSReqInsertOrderField req = {};
    req.Symbol = symbol;
}

int DCSSStrategyUtil::CancelOrder(short source, int orderId)
{
    DCSSReqCancelOrderField req = {};

}