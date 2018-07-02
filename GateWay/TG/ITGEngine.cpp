//
// Created by wangzhen on 18-6-13.
//

#include <thread>
#include "ITGEngine.h"
#include "OKTGApi.h"

using namespace DCSS;

void ITGEngine::Init(const std::vector<short>& index)
{
    for (auto& i : index)
    {
        ITGApiPtr p = ITGApi::CreateTGApi(i);
        if (p.get() != nullptr);
        p->Register(shared_from_this());
        mApiMap[i] = p;
    }
}

void ITGEngine::SetReaderThread()
{
    mReaderThread = ThreadPtr(new std::thread(std::bind(&ITGEngine::Listening, this)));
}

void ITGEngine::Listening()
{
    FramePtr frame;
    while (mIsRunning && SignalReceived < 0)
    {
        frame = mReader->GetNextFrame();
        if (frame.get() != nullptr)
        {
            short source = frame->GetSource();

            if (mApiMap.count(source) == 0)
            {
                // TODO
                continue;
            }

            ITGApiPtr& tgApi = mApiMap.at(source);

            FH_MSG_TP_TYPE msgType = frame->GetMsgType();

//            std::string Name = mReader->GetFrameName();


            void* data = frame->GetData();
            int requestId = frame->GetRequestID();
            switch (msgType)
            {
//            case MSG_TYPE_REQ_QRY_POS:
//            {
//                DCSSREQ
//                break;
//            }
            case MSG_TYPE_REQ_ORDER_INSERT:
            {
                DCSSReqInsertOrderField* req = reinterpret_cast<DCSSReqInsertOrderField*>(data);
                tgApi->ReqInsertOrder(req, requestId);
                DCSS_LOG_DEBUG(mLogger, "[insert_order](rid)" << requestId << "(ticker)" << req->Symbol);
                break;
            }
            case MSG_TYPE_REQ_ORDER_ACTION:
            {
                DCSSReqCancelOrderField* req = reinterpret_cast<DCSSReqCancelOrderField*>(data);
                tgApi->ReqCancelOrder(req, requestId);
                break;
            }
            case MSG_TYPE_REQ_QRY_TICKER:
            {
                DCSSReqQryTickerField* req = reinterpret_cast<DCSSReqQryTickerField*>(data);
                tgApi->ReqQryTicker(req, requestId);
                break;
            }
            case MSG_TYPE_REQ_QRY_KLINE:
            {
                DCSSReqQryKlineField* req = reinterpret_cast<DCSSReqQryKlineField*>(data);
                tgApi->ReqQryKline(req, requestId);
                break;
            }
            case MSG_TYPE_REQ_QRY_ACCOUNT:
            {
                tgApi->ReqQryUserInfo(requestId);
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

void ITGEngine::OnRspQryTicker(const DCSSTickerField* ticker, int requestId, int errorId, const char* errorMsg)
{
    if (0 == errorId)
    {
        mWriter->WriteFrame(ticker, sizeof(DCSSTickerField), EXCHANGE_OKCOIN, MSG_TYPE_RSP_QRY_TICKER, requestId);
    }
    else
    {
        mWriter->WriteErrorFrame(ticker, sizeof(DCSSTickerField), EXCHANGE_OKCOIN, MSG_TYPE_RSP_QRY_TICKER, requestId, errorId, errorMsg);
    }
}

void ITGEngine::OnRspOrderAction(const DCSSRspCancelOrderField* rsp, int requestId, int errorId, const char* errorMsg)
{
    if (0 == errorId)
    {
        mWriter->WriteFrame(rsp, sizeof(DCSSRspCancelOrderField), EXCHANGE_OKCOIN, MSG_TYPE_RSP_ORDER_ACTION, requestId);
    }
    else
    {
        mWriter->WriteErrorFrame(rsp, sizeof(DCSSRspCancelOrderField), EXCHANGE_OKCOIN, MSG_TYPE_RSP_ORDER_ACTION, requestId, errorId, errorMsg);
    }
}

void ITGEngine::OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, int requestId, int errorId, const char* errorMsg)
{
    if (0 == errorId)
    {
        mWriter->WriteFrame(rsp, sizeof(DCSSRspInsertOrderField), EXCHANGE_OKCOIN, MSG_TYPE_RSP_ORDER_INSERT, requestId);
    }
    else
    {
        mWriter->WriteErrorFrame(rsp, sizeof(DCSSRspInsertOrderField), EXCHANGE_OKCOIN, MSG_TYPE_RSP_ORDER_INSERT, requestId, errorId, errorMsg);
    }
}

void ITGEngine::OnRspQryUserInfo(const DCSSUserInfoField* userInfo, int requestId, int errorId, const char* errorMsg)
{
    if (0 == errorId)
    {
        mWriter->WriteFrame(userInfo, sizeof(DCSSUserInfoField), EXCHANGE_OKCOIN, MSG_TYPE_RSP_QRY_ACCOUNT, requestId);
    }
    else
    {
        mWriter->WriteErrorFrame(userInfo, sizeof(DCSSUserInfoField), EXCHANGE_OKCOIN, MSG_TYPE_RSP_QRY_ACCOUNT, requestId, errorId, errorMsg);
    }
}

void ITGEngine::OnRtnOrder(const DCSSOrderField* order)
{
    mWriter->WriteFrame(order, sizeof(DCSSOrderField), EXCHANGE_OKCOIN, MSG_TYPE_RTN_ORDER, 0);
}

void ITGEngine::OnRspQryOrder(const DCSSRspQryOrderHeaderField* header, const std::vector<DCSSRspQryOrderField>& order,
        int requestId, int errorId, const char* errorMsg)
{
    int length = sizeof(DCSSRspQryOrderHeaderField) + order.size() * sizeof(DCSSRspQryOrderField);
    uint8_t tmp[length];
    bzero(tmp, length);
    memcpy(tmp, header, sizeof(DCSSRspQryOrderHeaderField));

    for (int i = 0; i < order.size(); ++i)
    {
        memcpy(tmp + sizeof(DCSSRspQryOrderHeaderField) + i *sizeof(DCSSRspQryOrderField), &order[i], sizeof(DCSSRspQryOrderField));
    }
    if (errorId == 0)
        mWriter->WriteFrame(tmp, length, EXCHANGE_OKCOIN, MSG_TYPE_RSP_QRY_ORDER, requestId);
    else
        mWriter->WriteErrorFrame(tmp, length, EXCHANGE_OKCOIN, MSG_TYPE_RSP_QRY_ORDER, requestId, errorId, errorMsg);
}

void ITGEngine::OnRspQryKline(const DCSSKlineHeaderField* header, const std::vector<DCSSKlineField>& kline,
        int requestId, int errorId, const char* errorMsg)
{
    int length = sizeof(DCSSKlineHeaderField) + kline.size() * sizeof(DCSSKlineField);
    uint8_t tmp[length];
    bzero(tmp, length);
    memcpy(tmp, header, sizeof(DCSSKlineHeaderField));

    for (int i = 0; i < kline.size(); ++i)
    {
        memcpy(tmp + sizeof(DCSSKlineHeaderField) + i *sizeof(DCSSKlineField), &kline[i], sizeof(DCSSKlineField));
    }
    if (errorId == 0)
        mWriter->WriteFrame(tmp, length, EXCHANGE_OKCOIN, MSG_TYPE_RSP_QRY_KLINE, requestId);
    else
        mWriter->WriteErrorFrame(tmp, length, EXCHANGE_OKCOIN, MSG_TYPE_RSP_QRY_KLINE, requestId, errorId, errorMsg);
}

ITGApiPtr ITGApi::CreateTGApi(short index)
{
    return ITGApiPtr(new OKTGApi("640dfcea-3355-4bd7-8fd7-1150a9901276", "5847EDA9A02BA35B67D92ADF26C23E58"));
}