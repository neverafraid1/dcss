//
// Created by wangzhen on 18-6-28.
//

#include <util.h>
#include <iostream>
#include "DCSSDataWrapper.h"
#include "DCSSStrategyImpl.h"
#include "SysMessages.h"
#include "Timer.h"

volatile int DCSSStrategyImpl::mSingalReceived = -1;

DCSSDataWrapper::DCSSDataWrapper(DCSSStrategyImpl* impl)
: mImpl(impl), mForceStop(false)
{
    auto rids = mImpl->GetRidRange();
    mRidStart = rids.first;
    mRidEnd = rids.second;
    mCurTime = GetNanoTime();

    mImpl->OnTime(mCurTime);
}

void DCSSDataWrapper::PreRun()
{
    const std::string& name = mImpl->GetName();
    mFolders.emplace_back(STRATEGY_BASE_FOLDER);
    mNames.emplace_back(name + "_TG");
    mReader = UnitReader::CreateReaderWithSys(mFolders, mNames, GetNanoTime(), mImpl->GetName());
}

void DCSSDataWrapper::Connect(const std::string& config, long time)
{
    for (auto& it : mTdStatus)
    {
        *(it.second) = GWStatus::Requested;
    }
    mImpl->TdConnect(config);
    long startTime = GetNanoTime();
    while (!IsAllLogined() && GetNanoTime() - startTime < time && mImpl->mSingalReceived <= 0);
}

void DCSSDataWrapper::Stop()
{
    mForceStop = true;
}

GWStatus DCSSDataWrapper::GetTdStatus(uint8_t source)
{
    return mTdStatus.count(source) == 0 ? GWStatus::Unknown : *(mTdStatus.at(source));
}

void DCSSDataWrapper::AddMarketData(uint8_t source)
{
    UnitPair p = GetMdUnitPair(source);
    mFolders.emplace_back(p.first);
    mNames.emplace_back(p.second);
}

void DCSSDataWrapper::AddRegisterTd(uint8_t source)
{
    mTdStatus[source].reset(new GWStatus(GWStatus::Added));
}

void DCSSDataWrapper::AddTicker(const std::string& symbol, uint8_t source)
{
    mSubedTicker[source].insert(symbol);
}

void DCSSDataWrapper::AddKline(const std::string& symbol, KlineType klineType, uint8_t source)
{
    mSubedKline[source][symbol].insert(klineType);
}

void DCSSDataWrapper::AddDepth(const std::string& symbol, uint8_t source)
{
    mSubedDepth[source].insert(symbol);
}

void DCSSDataWrapper::Run()
{
    FramePtr frame;
    mForceStop = false;
    mImpl->mSingalReceived = -1;

    while (!mForceStop && mImpl->mSingalReceived <= 0)
    {
        frame = mReader->GetNextFrame();
        if (frame != nullptr)
        {
            mCurTime = frame->GetNano();

            /*process message*/
            short msgType = frame->GetMsgType();
            uint8_t msgSource = frame->GetSource();
            int requestId = frame->GetRequestID();

            if (msgType == MSG_TYPE_TRADE_ENGINE_ACK)
            {
                if (*(mTdStatus[msgSource]) == GWStatus::Requested)
                {
                    std::string content((char*) frame->GetData());
                    ProcessTdAck(content, msgSource, mCurTime);
                }
            }
            else
            {
                void* data = frame->GetData();
                if (-1 == requestId)
                {
                    mImpl->SetMdNano(mCurTime);
                    switch (msgType)
                    {
                    case MSG_TYPE_RTN_TICKER:
                    {
                        auto ticker = static_cast<const DCSSTickerField*>(data);
                        const std::string& symbol(ticker->Symbol);
                        if (mSubedTicker.count(msgSource) > 0 && mSubedTicker.at(msgSource).count(symbol) > 0)
                        {
                            mImpl->OnRtnTicker(ticker, msgSource, mCurTime);
                        }
                        break;
                    }
                    case MSG_TYPE_RTN_KLINE:
                    {
                        auto kline = static_cast<const DCSSKlineField*>(data);
                        const std::string& symbol(kline->Symbol);
                        if (mSubedKline.count(msgSource) > 0 && mSubedKline.at(msgSource).count(symbol) > 0
                                && mSubedKline.at(msgSource).at(symbol).count(kline->Type) > 0)
                        {
                            mImpl->OnRtnKline(kline, msgSource, mCurTime);
                        }
                        break;
                    }
                    case MSG_TYPE_RTN_DEPTH:
                    {
                        auto depth = static_cast<const DCSSDepthField*>(data);
                        const std::string& symbol(depth->Symbol);
                        if (mSubedDepth.count(msgSource) > 0 && mSubedDepth.at(msgSource).count(symbol) > 0)
                        {
                            mImpl->OnRtnDepth(depth, msgSource, mCurTime);
                        }
                        break;
                    }
                    }
                }
                else if (0 == requestId)
                {
                    switch (msgType)
                    {
                    case MSG_TYPE_RTN_ORDER:
                    {
                        mImpl->OnRtnOrder(static_cast<DCSSOrderField*>(data), msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RTN_BALANCE:
                    {
                        mImpl->OnRtnBalance(static_cast<DCSSBalanceField*>(data), msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RTN_TD_STATUS:
                    {
                        *(mTdStatus[msgSource]) = *(static_cast<GWStatus*>(data));
                        break;
                    }
                    }
                }
                else if (requestId >= mRidStart && requestId < mRidEnd)
                {
                    switch (msgType)
                    {
                    case MSG_TYPE_RSP_QRY_ACCOUNT:
                    {
                        mImpl->OnRspQryTradingAccount(static_cast<DCSSTradingAccountField*>(data), requestId,
                                frame->GetErrorID(), frame->GetErrorMsg(), msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RSP_ORDER_INSERT:
                    {
                        mImpl->OnRspOrderInsert(static_cast<DCSSRspInsertOrderField*>(data), requestId,
                                frame->GetErrorID(), frame->GetErrorMsg(), msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RSP_QRY_TICKER:
                    {
                        mImpl->OnRspQryTicker(static_cast<DCSSTickerField*>(data), requestId, frame->GetErrorID(),
                                frame->GetErrorMsg(), msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RSP_QRY_KLINE:
                    {
                        mImpl->OnRspQryKline(static_cast<const DCSSKlineField*>(data), requestId, frame->GetErrorID(), frame->GetErrorMsg(),
                                        msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RSP_QRY_SIGNAL_ORDER:
                    {
                        mImpl->OnRspQryOrder(static_cast<DCSSOrderField*>(data), requestId, frame->GetErrorID(),
                                frame->GetErrorMsg(), msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RSP_QRY_SYMBOL:
                    {
                        mImpl->OnRspQrySymbol(static_cast<DCSSSymbolField*>(data), requestId, frame->GetErrorID(),
                                frame->GetErrorMsg(), msgSource, mCurTime);
                        break;
                    }
                    }
                }
            }
        }
        else
        {
            mCurTime = GetNanoTime();
        }
        mImpl->OnTime(mCurTime);
    }

    if (mImpl->mSingalReceived > 0)
    {
        DCSS_LOG_DEBUG(mImpl->GetLogger(), "[DataWrapper] signal received: " << mImpl->mSingalReceived);
    }

    if (mForceStop)
    {
        DCSS_LOG_DEBUG(mImpl->GetLogger(), "[DataWrapper] forced to stop!");
    }
}

bool DCSSDataWrapper::IsAllLogined()
{
    bool rtn(true);
    for (auto& item : mTdStatus)
    {
        rtn &= (*(item.second) == GWStatus::Logined);
    }
    return rtn;
}

void DCSSDataWrapper::ProcessTdAck(const std::string& content, uint8_t source, long recvTime)
{

}
