//
// Created by wangzhen on 18-6-25.
//

#include <csignal>
#include <thread>
#include <iostream>
#include "IDCSSStrategy.h"

static std::string SymbolStr(const DCSSSymbolField& symbol)
{
    std::stringstream ss;
    ss << symbol.Base << "_" << symbol.Quote;
    return ss.str();
}

void RegisterSignalCallback()
{
    std::signal(SIGTERM, IDCSSDataProcessor::SignalHandler);
    std::signal(SIGINT, IDCSSDataProcessor::SignalHandler);
    std::signal(SIGHUP, IDCSSDataProcessor::SignalHandler);
    std::signal(SIGQUIT, IDCSSDataProcessor::SignalHandler);
    std::signal(SIGKILL, IDCSSDataProcessor::SignalHandler);
}

IDCSSStrategy::IDCSSStrategy(const std::string& name) : mName(name)
{
    mLogger = DCSSLog::GetStrategyLogger(name, name);
    mUtil.reset(new DCSSStrategyUtil(name));
    mData.reset(new DCSSDataWrapper(this, mUtil.get()));

    RegisterSignalCallback();
}

void IDCSSStrategy::Start()
{
    mDataThread.reset(new std::thread(&DCSSDataWrapper::Run, mData.get()));
    DCSS_LOG_INFO(mLogger, "[Start] data thread start");
}

IDCSSStrategy::~IDCSSStrategy()
{
    DCSS_LOG_INFO(mLogger, "[IDCSSStrategytegy]");

    mDataThread->join();
    mDataThread.reset();
    mData.reset();
    mLogger.reset();
    mUtil.reset();
}

void IDCSSStrategy::Terminate()
{
    Stop();

    if (mDataThread.get() != nullptr)
    {
        mDataThread->join();
        mDataThread.reset();
    }

    DCSS_LOG_INFO(mLogger, "[Terminate] terminated");
}

void IDCSSStrategy::Stop()
{
    if (mData.get() != nullptr)
        mData->Stop();
}

void IDCSSStrategy::Run()
{
    if (mData.get() != nullptr)
        mData->Run();
}

void IDCSSStrategy::Block()
{
    if (mDataThread.get() != nullptr)
        mDataThread->join();
}

void IDCSSStrategy::OnRtnTicker(const DCSSTickerField& ticker, short source, long recvTime)
{
    std::cout << GetNanoTime() - recvTime << std::endl;
//    DCSS_LOG_DEBUG(mLogger, "[rtn ticker]"
//            << " (source)" << source
//            << " (symbol)" << SymbolStr(ticker.Symbol)
//            << " (last price)" << ticker.LastPrice
//            << " (time)" << ticker.Time
//            << " (recv time)" << recvTime);
}

void IDCSSStrategy::OnRtnKline(const DCSSKlineHeaderField& header, const std::vector<DCSSKlineField>& kline,
        short source, long recvTime)
{
//    DCSS_LOG_DEBUG(mLogger, "[kline]"
//                            << " (source)" << source
//    << " (symbol)" << SymbolStr(header.Symbol) << )
}

void IDCSSStrategy::OnRtnOrder(const DCSSOrderField& order, int requestID, short source, long recvTime)
{
    DCSS_LOG_DEBUG(mLogger, "[rtn_order] (source)" << source << " (rid)" << requestID);
}

void IDCSSStrategy::OnRspOrderInsert(const DCSSRspInsertOrderField& rsp, int requestID, short source, long recvTime)
{

}

void IDCSSStrategy::Debug(const char* msg)
{
    DCSS_LOG_DEBUG(mLogger, msg);
}

void IDCSSStrategy::OnTime(long curTime)
{
//    mUtil->
}

int IDCSSStrategy::InsertLimitOrder(short source, const DCSSSymbolField& symbol, double price, double volume, TradeTypeType tradeType)
{
    return mUtil->InsertLimitOrder(source, symbol, price, volume, tradeType);
}

int IDCSSStrategy::InsertMarketOrder(short source, const DCSSSymbolField& symbol, double volume,
        TradeTypeType tradeType)
{
    return mUtil->InsertMarketOrder(source, symbol, volume, tradeType);
}

int IDCSSStrategy::CancelOrder(short source, const DCSSSymbolField& symbol, long orderId)
{
    return mUtil->CancelOrder(source, symbol, orderId);
}

bool IDCSSStrategy::IsTdReady(short source) const
{
    if (mData.get() != nullptr)
    {
        auto status = mData->GetTdStatus(source);
        if (status == CONNECT_TD_STATUS_ACK_POS
                || status == CONNECT_TD_STATUS_SET_BACK)
            return true;
    }
    return false;
}

bool IDCSSStrategy::IsTdConnected(short source) const
{
    if (mData.get() != nullptr)
    {
        auto status = mData->GetTdStatus(source);
        if (status == CONNECT_TD_STATUS_ACK_POS
                || status == CONNECT_TD_STATUS_SET_BACK
                || status == CONNECT_TD_STATUS_ACK_NONE)
            return true;
    }
    return false;
}
