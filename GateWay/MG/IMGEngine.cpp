//
// Created by wangzhen on 18-6-15.
//

#include <thread>
#include "IMGEngine.h"
#include "OKMGApi.h"

void IMGEngine::Init(short index)
{
    mMGApi = IMGApi::Create(index);
    mMGApi->Register(shared_from_this());
}

void IMGEngine::SetReaderThread()
{
    mReaderThread = ThreadPtr(new std::thread(std::bind(&IMGEngine::Listening, this)));
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
//            if (source != ) TODO

            FH_MSG_TP_TYPE msgType = frame->GetMsgType();
            int requestId = frame->GetRequestID();
            void* data = frame->GetData();

            switch (msgType)
            {
            case MSG_TYPE_SUB_TICKER:
            {
                DCSSSubTickerField* req = reinterpret_cast<DCSSSubTickerField*>(data);
                mMGApi->ReqSubTicker(req->Symbol);
                break;
            }
            case MSG_TYPE_SUB_DEPTH:
            {
                DCSSSubDepthField* req = reinterpret_cast<DCSSSubDepthField*>(data);
                mMGApi->ReqSubDepth(req->Symbol, req->Depth);
                break;
            }
            case MSG_TYPE_SUB_KLINE:
            {
                DCSSSubKlineField* req = reinterpret_cast<DCSSSubKlineField*>(data);
                mMGApi->ReqSubKline(req->Symbol, req->KlineType);
                break;
            }
            case MSG_TYPE_UNSUB_TICKER:
            {
                DCSSSubTickerField* req = reinterpret_cast<DCSSSubTickerField*>(data);
                mMGApi->ReqUnSubTicker(req->Symbol);
                break;
            }
            case MSG_TYPE_UNSUM_DEPTH:
            {
                DCSSSubDepthField* req = reinterpret_cast<DCSSSubDepthField*>(data);
                mMGApi->ReqUnSubDepth(req->Symbol, req->Depth);
                break;
            }
            case MSG_TYPE_UNSUB_KLINE:
            {
                DCSSSubKlineField* req = reinterpret_cast<DCSSSubKlineField*>(data);
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

void IMGEngine::OnRtnTicker(const DCSSTickerField* ticker)
{
    if (mIsRunning)
    {
        mWriter->WriteFrame(ticker, sizeof(DCSSTickerField), 1, MSG_TYPE_RTN_TICKER, -1);
    }
}

void IMGEngine::OnRtnDepth(const DCSSDepthHeaderField* header, const std::vector<DCSSDepthField>& depthVec)
{
    if (mIsRunning)
    {
        int length = sizeof(DCSSDepthHeaderField) + depthVec.size() * sizeof(DCSSDepthField);
        char tmp[length];
        bzero(tmp, length);
        memcpy(tmp, header, sizeof(DCSSDepthHeaderField));
        for (int i = 0; i < depthVec.size(); ++i)
        {
            memcpy(tmp + sizeof(DCSSDepthHeaderField) + i * sizeof(DCSSDepthField), &(depthVec[i]), sizeof(DCSSDepthField));
        }

        mWriter->WriteFrame(tmp, length, 1, MSG_TYPE_RTN_DEPTH, -1);
    }
}

void IMGEngine::OnRtnKline(const DCSSKlineHeaderField* header, const std::vector<DCSSKlineField>& klineVec)
{
    if (mIsRunning)
    {
        int length = sizeof(DCSSKlineHeaderField) + klineVec.size() * sizeof(DCSSKlineField);
        char tmp[length];
        bzero(tmp, length);
        memcpy(tmp, header, sizeof(DCSSKlineHeaderField));
        for (int i = 0; i < klineVec.size(); ++i)
        {
            memcpy(tmp + sizeof(DCSSKlineHeaderField) + i * sizeof(DCSSKlineField), &(klineVec[i]), sizeof(DCSSKlineField));
        }
        mWriter->WriteFrame(tmp,length, 1, MSG_TYPE_RTN_KLINE, -1);
    }
}