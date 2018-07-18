//
// Created by wangzhen on 18-6-28.
//

#include <util.h>
#include <iostream>
#include "DCSSDataWrapper.h"
#include "SysMessages.h"

volatile int IDCSSDataProcessor::mSingalReceived = -1;

static std::string SymbolStr(const DCSSSymbolField& symbol)
{
    std::string s;
    s.append(symbol.Base).append("_").append(symbol.Quote);
    return s;
}

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
    mFolders.push_back(STRATEGY_BASE_FOLDER);
    mNames.push_back(name + "_TG");
    mReader = UnitReader::CreateReaderWithSys(mFolders, mNames, GetNanoTime(), mProcessor->GetName());

    for (auto& it : mTdStatus)
    {
        mUtil->TdConnect(it.first);
        it.second = CONNECT_TD_STATUS_REQUESTED;
    }
}

void DCSSDataWrapper::Stop()
{
    mForceStop = true;
}

uint8_t DCSSDataWrapper::GetTdStatus(short source)
{
    return mTdStatus.count(source) == 0 ? CONNECT_TD_STATUS_UNKNOWN : mTdStatus.at(source);
}

void DCSSDataWrapper::AddMarketData(short source)
{
    UnitPair p = GetMdUnitPair(source);
    mFolders.emplace_back(p.first);
    mNames.emplace_back(p.second);
}

void DCSSDataWrapper::AddRegisterTd(short source)
{
    mTdStatus[source] = CONNECT_TD_STATUS_ADDED;
}

void DCSSDataWrapper::Run()
{
    PreRun();

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
            short msgSource = frame->GetSource();
            int requestId = frame->GetRequestID();
            if (msgType == MSG_TYPE_TRADE_ENGINE_ACK)
            {
                if (mTdStatus[msgSource] == CONNECT_TD_STATUS_REQUESTED)
                {
                    std::string content((char*)frame->GetData());
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
                        auto ticker = static_cast<DCSSTickerField*>(data);
                        mLastPriceMap[SymbolStr(ticker->Symbol)] = ticker->LastPrice;
                        mProcessor->OnRtnTicker(*ticker, msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RTN_KLINE:
                    {
                        auto header = static_cast<DCSSKlineHeaderField*>(data);
                        std::vector<DCSSKlineField> klineVec(header->Size);
                        for (auto i = 0; i < klineVec.size(); ++i)
                        {
                            memcpy(&klineVec[i], header + sizeof(DCSSKlineHeaderField) + i * sizeof(DCSSKlineField), sizeof(DCSSKlineField));
                        }
                        mProcessor->OnRtnKline(*header, klineVec, msgSource, mCurTime);
                        break;
                    }
                    case MSG_TYPE_RTN_DEPTH:
                    {
                        auto header = static_cast<DCSSDepthHeaderField*>(data);
                        std::vector<DCSSDepthField> ask(header->AskNum);
                        std::vector<DCSSDepthField> bid(header->BidNum);
                        for (auto i = 0; i < ask.size(); ++i)
                        {
                            memcpy(&ask[i], header + sizeof(DCSSDepthHeaderField) + i * sizeof(DCSSDepthField), sizeof(DCSSDepthField));
                        }
                        for (auto i = 0; i < bid.size(); ++i)
                        {
                            memcpy(&bid[i], header + sizeof(DCSSDepthHeaderField)
                            + (ask.size() + i) * sizeof(DCSSDepthField), sizeof(DCSSDepthField));
                        }
                        mProcessor->OnRtnDepth(*header, ask, bid, msgSource, mCurTime);
                        break;
                    }
                    }
                }
                else if (requestId >= mRidStart && requestId < mRidEnd)
                {
//                    switch (msgType)
//                    {
//                    case MSG_TYPE_RTN_ORDER:
//                    {
//
//                    }
//                    }
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

void DCSSDataWrapper::ProcessTdAck(const std::string& content, short source, long recvTime)
{

}