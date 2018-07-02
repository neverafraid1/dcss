//
// Created by wangzhen on 18-6-25.
//

#include <csignal>
#include <thread>
#include <iostream>
#include "IDCSSStrategy.h"

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
    mUtil = std::make_shared(new DCSSStrategyUtil(name));
    mData = std::make_shared(new DCSSDataWrapper(this, mUtil.get()));

    RegisterSignalCallback();
}

void IDCSSStrategy::Start()
{
    mDataThread = std::make_shared(new std::thread(&DCSSDataWrapper::Run, mData.get()));
    DCSS_LOG_INFO(mLogger, "[Start] data thread start");
}

void IDCSSStrategy::~IDCSSStrategy()
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

void IDCSSStrategy::OnRtnTicker(const DCSSTickerField& ticker, long recvTime)
{
    DCSS_LOG_DEBUG(mLogger, "[rtn ticker] (symbol)"
            << ticker.Symbol
            << " (last price)" << ticker.LastPrice << " (time)" << ticker.Time
            << " (recv time)" << recvTime);
}

void IDCSSStrategy::OnRtnKline(const DCSSKlineHeaderField& header, const std::vector<DCSSKlineField>& kline, long recvTime)
{ }

void IDCSSStrategy::OnRtnDepth(const DCSSDepthHeaderField& header, const std::vector<DCSSDepthField>& ask,
        const std::vector<DCSSDepthField>& bid, long recvTime)
{ }

void IDCSSStrategy::OnRspOrderInsert(const DCSSRspInsertOrderField& rsp, int requestId, long recvTime, short errorId,
        const char* errorMsg)
{ }

void IDCSSStrategy::OnTime(long curTime)
{
    mUtil->P
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
