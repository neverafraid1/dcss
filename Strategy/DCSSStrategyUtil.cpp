//
// Created by wangzhen on 18-6-28.
//

#include "DCSSStrategyUtil.h"
#include "Timer.h"
#include "SysMessages.h"

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

int DCSSStrategyUtil::InsertOrder(uint8_t source, const std::string& symbol, double price, double volume,
		OrderDirection direction, OrderType type)
{
    int rid = GetRid();

    DCSSReqInsertOrderField req = {};
    strcpy(req.Symbol, symbol.c_str());
    req.Price = price;
    req.Amount = volume;
    req.Direction = direction;
    req.Type = type;

    WriteFrameExtra(&req, sizeof(DCSSReqInsertOrderField), source, MSG_TYPE_REQ_ORDER_INSERT, true, rid, mMdNano);
    return rid;
}

int DCSSStrategyUtil::CancelOrder(uint8_t source, const std::string& symbol, long orderId)
{
    int rid = GetRid();

    DCSSReqCancelOrderField req = {};
    strcpy(req.Symbol, symbol.c_str());
    req.OrderID[0] = orderId;
    WriteFrame(&req, sizeof(DCSSReqCancelOrderField), source, MSG_TYPE_REQ_ORDER_ACTION, true, rid);

    return rid;
}

int DCSSStrategyUtil::ReqQryAccount(uint8_t source)
{
    int rid = GetRid();

    char tmp;
    WriteFrame(&tmp, sizeof(char), source, MSG_TYPE_REQ_QRY_ACCOUNT, true, rid);

    return rid;
}

int DCSSStrategyUtil::ReqQryTicker(uint8_t source, const DCSSReqQryTickerField& req)
{
    int rid = GetRid();

    WriteFrame(&req, sizeof(DCSSReqQryTickerField), source, MSG_TYPE_REQ_QRY_TICKER, true, rid);

    return rid;
}

int DCSSStrategyUtil::ReqQryKline(uint8_t source, const DCSSReqQryKlineField& req)
{
    int rid = GetRid();

    WriteFrame(&req, sizeof(DCSSReqQryKlineField), source, MSG_TYPE_REQ_QRY_KLINE, true, rid);

    return rid;
}
