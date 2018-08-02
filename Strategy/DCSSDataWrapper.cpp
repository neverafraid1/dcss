//
// Created by wangzhen on 18-6-28.
//

#include <util.h>
#include <iostream>
#include "DCSSDataWrapper.h"
#include "SysMessages.h"
#include "Helper.h"

volatile int IDCSSDataProcessor::mSingalReceived = -1;

DCSSDataWrapper::DCSSDataWrapper(IDCSSDataProcessor* processor, DCSSStrategyUtil* util)
: mProcessor(processor), mUtil(util), mForceStop(false)
{
    auto rids = mUtil->GetRidRange();
    mRidStart = rids.first;
    mRidEnd = rids.second;
    mCurTime = GetNanoTime();

    mProcessor->OnTime(mCurTime);
}

void DCSSDataWrapper::PreRun()
{
    const std::string& name = mUtil->GetName();
    mFolders.emplace_back(STRATEGY_BASE_FOLDER);
    mNames.push_back(name + "_TG");
    mReader = UnitReader::CreateReaderWithSys(mFolders, mNames, GetNanoTime(), mProcessor->GetName());
}

void DCSSDataWrapper::Connect(long time)
{
    for (auto& it : mTdStatus)
    {
        mUtil->TdConnect(it.first);
        it.second = TD_STATUS_REQUESTED;
    }

    long startTime = GetNanoTime();
    while (!IsAllLogined() && GetNanoTime() - startTime < time);
}

void DCSSDataWrapper::Stop()
{
    mForceStop = true;
}

uint8_t DCSSDataWrapper::GetTdStatus(uint8_t source)
{
    return mTdStatus.count(source) == 0 ? TD_STATUS_UNKNOWN : mTdStatus.at(source);
}

void DCSSDataWrapper::AddMarketData(uint8_t source)
{
    UnitPair p = GetMdUnitPair(source);
    mFolders.emplace_back(p.first);
    mNames.emplace_back(p.second);
}

void DCSSDataWrapper::AddRegisterTd(uint8_t source)
{
    mTdStatus[source] = TD_STATUS_ADDED;
}

void DCSSDataWrapper::AddTicker(const std::string& symbol, uint8_t source)
{
    mSubedTicker[source].insert(symbol);
}

void DCSSDataWrapper::AddKline(const std::string& symbol, KlineTypeType klineType, uint8_t source)
{
    mSubedKline[source][symbol].insert(klineType);
}

void DCSSDataWrapper::AddDepth(const std::string& symbol, int depth, uint8_t source)
{
    mSubedDepth[source][symbol].insert(depth);
}

void DCSSDataWrapper::Run()
{
    FramePtr frame;
    mForceStop = false;
    mProcessor->mSingalReceived = -1;

    while (!mForceStop && mProcessor->mSingalReceived <= 0)
    {
        frame = mReader->GetNextFrame();
        if (frame.get() != nullptr)
        {
            mCurTime = frame->GetNano();

            /*process message*/
            short msgType = frame->GetMsgType();
            uint8_t msgSource = frame->GetSource();
            int requestId = frame->GetRequestID();

            if (msgType == MSG_TYPE_TRADE_ENGINE_ACK)
            {
                if (mTdStatus[msgSource] == TD_STATUS_REQUESTED)
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
                    mUtil->SetMdNano(mCurTime);
                    switch (msgType)
                    {
                    case MSG_TYPE_RTN_TICKER:
                    {
                        auto ticker = static_cast<const DCSSTickerField*>(data);
                        const std::string& symbol(ticker->Symbol);
                        if (mSubedTicker.count(msgSource) > 0 && mSubedTicker.at(msgSource).count(symbol) > 0)
                        {
                            mLastPriceMap[symbol] = ticker->LastPrice;
                            mProcessor->OnRtnTicker(ticker, msgSource, mCurTime);
                        }
                        break;
                    }
                    case MSG_TYPE_RTN_KLINE:
                    {
                        auto header = static_cast<const DCSSKlineHeaderField*>(data);
                        const std::string& symbol(header->Symbol);
                        if (mSubedKline.count(msgSource) > 0 && mSubedKline.at(msgSource).count(symbol) > 0
                                && mSubedKline.at(msgSource).at(symbol).count(header->KlineType) > 0)
                        {
                            std::vector<const DCSSKlineField*> klineVec;
                            for (auto i = 0; i < header->Size; ++i)
                            {
                                klineVec.emplace_back(
                                        static_cast<const DCSSKlineField*>(data + sizeof(DCSSKlineHeaderField)
                                                + i * sizeof(DCSSKlineField)));
                            }
                            mProcessor->OnRtnKline(header, klineVec, msgSource, mCurTime);
                        }
                        break;
                    }
                    case MSG_TYPE_RTN_DEPTH:
                    {
                        auto header = static_cast<const DCSSDepthHeaderField*>(data);
                        const std::string& symbol(header->Symbol);
                        if (mSubedDepth.count(msgSource) > 0 && mSubedDepth.at(msgSource).count(symbol) > 0
                                && mSubedDepth.at(msgSource).at(symbol).count(header->Depth) > 0)
                        {
                            std::vector<const DCSSDepthField*> ask;
                            std::vector<const DCSSDepthField*> bid;
                            for (auto i = 0; i < header->AskNum; ++i)
                            {
                                ask.emplace_back(static_cast<const DCSSDepthField*>(data + sizeof(DCSSDepthHeaderField)
                                        + i * sizeof(DCSSDepthField)));
                            }
                            for (auto i = 0; i < header->BidNum; ++i)
                            {
                                bid.emplace_back(static_cast<const DCSSDepthField*>(data + sizeof(DCSSDepthHeaderField)
                                        + (ask.size() + i) * sizeof(DCSSDepthField)));
                            }
                            mProcessor->OnRtnDepth(header, ask, bid, msgSource, mCurTime);
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
                        mProcessor->OnRtnOrder(static_cast<DCSSOrderField*>(data), msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RTN_BALANCE:
                    {
                        mProcessor->OnRtnBalance(static_cast<DCSSBalanceField*>(data), msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RTN_TD_STATUS:
                    {
                        mTdStatus[msgSource] = *(static_cast<GateWayStatusType*>(data));
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
                        mProcessor->OnRspQryTradingAccount(static_cast<DCSSTradingAccountField*>(data), requestId,
                                frame->GetErrorID(), frame->GetErrorMsg(), msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RSP_ORDER_INSERT:
                    {
                        mProcessor->OnRspOrderInsert(static_cast<DCSSRspInsertOrderField*>(data), requestId,
                                frame->GetErrorID(), frame->GetErrorMsg(), msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RSP_QRY_TICKER:
                    {
                        mProcessor->OnRspQryTicker(static_cast<DCSSTickerField*>(data), requestId, frame->GetErrorID(),
                                frame->GetErrorMsg(), msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RSP_QRY_KLINE:
                    {
                        auto header = static_cast<const DCSSKlineHeaderField*>(data);
                        const std::string& symbol(header->Symbol);
                        std::vector<const DCSSKlineField*> klineVec;
                        for (auto i = 0; i < header->Size; ++i)
                        {
                            klineVec.emplace_back(
                                    static_cast<const DCSSKlineField*>(data + sizeof(DCSSKlineHeaderField)
                                            + i * sizeof(DCSSKlineField)));
                        }
                        mProcessor->OnRspQryKline(header, klineVec, requestId, frame->GetErrorID(), frame->GetErrorMsg(),
                                        msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RSP_QRY_ORDER:
                    {
                        mProcessor->OnRspQryOrder(static_cast<DCSSOrderField*>(data), requestId, frame->GetErrorID(),
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
        mProcessor->OnTime(mCurTime);
    }

    if (mProcessor->mSingalReceived > 0)
    {
        char msg[100];
        sprintf(msg, "%s%d", "[DataWrapper] signal received: ", mProcessor->mSingalReceived);
        mProcessor->Debug(msg);

    }

    if (mForceStop)
    {
        mProcessor->Debug("[DataWrapper] forced to stop!");
    }
}

bool DCSSDataWrapper::IsAllLogined()
{
    bool rtn(true);
    for (auto& item : mTdStatus)
    {
        rtn &= item.second == TD_STATUS_LOGINED;
    }
    return rtn;
}

void DCSSDataWrapper::ProcessTdAck(const std::string& content, uint8_t source, long recvTime)
{

}