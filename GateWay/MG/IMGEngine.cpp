//
// Created by wangzhen on 18-6-15.
//

#include <thread>
#include "util.h"
#include "IMGEngine.h"
#include "OKMGApi.h"
#include "Timer.h"

void IMGEngine::Init()
{
}

void IMGEngine::SetReaderThread()
{
    mReaderThread.reset(new std::thread(std::bind(&IMGEngine::Listening, this)));
}

void IMGEngine::Load(const nlohmann::json& config)
{
    short source = config.at("source");
    auto pair = GetMdUnitPair(source);
    mMGApi = IMGApi::Create(source);
    mMGApi->Register(shared_from_this());
    mMGApi->Connect();
    while (!mMGApi->IsConnected())
    {
        DCSS_LOG_INFO(mLogger, "[load] mg is not connected now (source)" << source);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    mWriter = UnitWriter::Create(pair.first, pair.second, mMGApi->Name());
    mReader = UnitReader::CreateRevisableReader(mMGApi->Name());
}

void IMGEngine::Listening()
{
    FramePtr frame;
    while (mIsRunning && SignalReceived < 0)
    {
        frame = mReader->GetNextFrame();
        if (frame.get() != nullptr)
        {
            short source = frame->GetSource();
            if (source != mMGApi->GetSource())
            {
                continue;
            }

            FH_MSG_TP_TYPE msgType = frame->GetMsgType();

            void* data = frame->GetData();

            switch (msgType)
            {
            case MSG_TYPE_SUB_TICKER:
            {
                auto req = static_cast<DCSSSubTickerField*>(data);
                mMGApi->ReqSubTicker(req->Symbol);
                break;
            }
            case MSG_TYPE_SUB_DEPTH:
            {
                auto req = static_cast<DCSSSubDepthField*>(data);
                mMGApi->ReqSubDepth(req->Symbol, req->Depth);
                break;
            }
            case MSG_TYPE_SUB_KLINE:
            {
                auto req = static_cast<DCSSSubKlineField*>(data);
                mMGApi->ReqSubKline(req->Symbol, req->KlineType);
                break;
            }
            case MSG_TYPE_UNSUB_TICKER:
            {
                auto req = static_cast<DCSSSubTickerField*>(data);
                mMGApi->ReqUnSubTicker(req->Symbol);
                break;
            }
            case MSG_TYPE_UNSUB_DEPTH:
            {
                auto req = static_cast<DCSSSubDepthField*>(data);
                mMGApi->ReqUnSubDepth(req->Symbol, req->Depth);
                break;
            }
            case MSG_TYPE_UNSUB_KLINE:
            {
                auto req = static_cast<DCSSSubKlineField*>(data);
                mMGApi->ReqUnSubKline(req->Symbol, req->KlineType);
                break;
            }
            default:break;
            }
        }
    }


    if (SignalReceived >= 0)
    {
        DCSS_LOG_INFO(mLogger, "[IEngine] signal received: " << SignalReceived);
    }

    if (!mIsRunning)
    {
        DCSS_LOG_INFO(mLogger, "[IEngine] forced to stop.");
    }
}

void IMGEngine::OnRtnTicker(const DCSSTickerField* ticker, short source)
{
    if (mIsRunning)
    {
        mWriter->WriteFrame(ticker, sizeof(DCSSTickerField), source, MSG_TYPE_RTN_TICKER, -1);
    }
}

void IMGEngine::OnRtnDepth(const DCSSDepthHeaderField* header, const std::vector<DCSSDepthField>& depthVec, short source)
{
    if (mIsRunning)
    {
        size_t length = sizeof(DCSSDepthHeaderField) + depthVec.size() * sizeof(DCSSDepthField);
        char tmp[length];
        bzero(tmp, length);
        memcpy(tmp, header, sizeof(DCSSDepthHeaderField));
        for (int i = 0; i < depthVec.size(); ++i)
        {
            memcpy(tmp + sizeof(DCSSDepthHeaderField) + i * sizeof(DCSSDepthField), &(depthVec[i]), sizeof(DCSSDepthField));
        }

        mWriter->WriteFrame(tmp, (int)length, source, MSG_TYPE_RTN_DEPTH, -1);
    }
}

void IMGEngine::OnRtnKline(const DCSSKlineHeaderField* header, const std::vector<DCSSKlineField>& klineVec, short source)
{
    if (mIsRunning)
    {
        size_t length = sizeof(DCSSKlineHeaderField) + klineVec.size() * sizeof(DCSSKlineField);
        char tmp[length];
        bzero(tmp, length);
        memcpy(tmp, header, sizeof(DCSSKlineHeaderField));
        for (int i = 0; i < klineVec.size(); ++i)
        {
            memcpy(tmp + sizeof(DCSSKlineHeaderField) + i * sizeof(DCSSKlineField), &(klineVec[i]), sizeof(DCSSKlineField));
        }
        mWriter->WriteFrame(tmp, (int)length, source, MSG_TYPE_RTN_KLINE, -1);
    }
}

IMGApiPtr IMGApi::Create(short source)
{
    switch (source)
    {
    case EXCHANGE_OKCOIN:
        return IMGApiPtr(new OKMGApi(source));
    default:
        return IMGApiPtr();
    }
}

short IMGApi::GetSource() const
{
    return mSourceId;
}