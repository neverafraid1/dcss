//
// Created by wangzhen on 18-8-5.
//

#include "ITGApi.h"
#include "OKEX/OKTGApi.h"
#include "Binance/BinaTGApi.h"
#include "Timer.h"
#include "SysMessages.h"

ITGSpi::ITGSpi(const std::string& client, DCSSLogPtr logger, const std::string& proxy)
: mClient(client), mSignalReceived(-1), mIsRunning(false), mLogger(std::move(logger)), mProxy(proxy)
{
    mWriter = UnitSafeWriter::Create(STRATEGY_BASE_FOLDER, mClient + "_TG", mClient + "_TG");
}

bool ITGSpi::Load(const std::string& config)
{
    mReader = UnitReader::Create(STRATEGY_BASE_FOLDER, mClient, GetNanoTime());
    nlohmann::json conf = nlohmann::json::parse(config);
    std::string folder = conf.at("folder");
    std::cout << folder << std::endl;
    auto accounts = conf.find("accounts");
    for (auto& item : accounts.value())
    {
        uint8_t source = item.at("source");
        if (mApiMap.count(source) == 0)
        {
            ITGApiPtr apiPtr = ITGApi::CreateTGApi(source);
            if (apiPtr == nullptr)
            {
                DCSS_LOG_ERROR(mLogger, "invalid (source)" << source);
                return false;
            }
            apiPtr->RegisterSpi(shared_from_this());
            apiPtr->LoadAccount(item);
            apiPtr->Connect();
            long startTime = GetNanoTime();
            while (!apiPtr->IsLogged() && GetNanoTime() > startTime < 10 * NANOSECONDS_PER_SECOND);
            if (!apiPtr->IsLogged())
            {
                DCSS_LOG_ERROR(mLogger, "cannot login tg (source)" << source);
                return false;
            }
            mApiMap[source] = std::move(apiPtr);
        }
    }
}

void ITGSpi::SetReaderThread()
{
    mIsRunning = false;
    mReaderThread.reset(new std::thread(&ITGSpi::Listening, this));
}

void ITGSpi::SetSignal(int sig)
{
    if (mSignalReceived > 0)
    {
        mSignalReceived = sig;
        mReaderThread->join();
    }
}

void ITGSpi::ForceStop()
{
    mIsRunning = false;
    mReaderThread->join();
}

void ITGSpi::Listening()
{
    if (mReader.get() == nullptr)
    {
        throw std::runtime_error("reader is not inited! please call init() before start()");
    }

    mIsRunning = true;

    FramePtr frame;
    while (mIsRunning && mSignalReceived < 0)
    {
        frame = mReader->GetNextFrame();
        if (frame.get() != nullptr)
        {
            mCurTime = frame->GetNano();
            uint8_t source = frame->GetSource();
            FH_MSG_TP_TYPE msgType = frame->GetMsgType();


            if (mApiMap.count(source) == 0 || !mApiMap.at(source)->IsLogged())
            {
                // TODO
                continue;
            }

            ITGApiPtr& tgApi = mApiMap.at(source);

            void* data = frame->GetData();
            int requestId(frame->GetRequestID());

            switch (msgType)
            {
            case MSG_TYPE_REQ_ORDER_INSERT:
            {
                auto req = reinterpret_cast<DCSSReqInsertOrderField*>(data);
                tgApi->ReqInsertOrder(req, requestId);
                DCSS_LOG_DEBUG(mLogger, "[insert_order] (rid)" << requestId << " (ticker)");
                break;
            }
            case MSG_TYPE_REQ_ORDER_ACTION:
            {
                auto req = reinterpret_cast<DCSSReqCancelOrderField*>(data);
                tgApi->ReqCancelOrder(req, requestId);
                break;
            }
            case MSG_TYPE_REQ_QRY_TICKER:
            {
                auto req = reinterpret_cast<DCSSReqQryTickerField*>(data);
                tgApi->ReqQryTicker(req, requestId);
                break;
            }
            case MSG_TYPE_REQ_QRY_KLINE:
            {
                auto req = reinterpret_cast<DCSSReqQryKlineField*>(data);
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

    if (mSignalReceived >= 0)
    {
        DCSS_LOG_INFO(mLogger, "signal received: " << mSignalReceived);
    }

    if (!mIsRunning)
    {
        DCSS_LOG_INFO(mLogger, "forced to stop.");
    }
}

void ITGSpi::OnRspQryTicker(const DCSSTickerField* ticker, uint8_t source, int requestId, int errorId, const char* errorMsg)
{
    if (0 == errorId)
    {
        mWriter->WriteFrame(ticker, sizeof(DCSSTickerField), source, MSG_TYPE_RSP_QRY_TICKER, requestId);
    }
    else
    {
        mWriter->WriteErrorFrame(ticker, sizeof(DCSSTickerField), source, MSG_TYPE_RSP_QRY_TICKER, requestId, errorId, errorMsg);
    }
}

void ITGSpi::OnRspOrderAction(const DCSSRspCancelOrderField* rsp, uint8_t source, int requestId, int errorId, const char* errorMsg)
{
    if (0 == errorId)
    {
        mWriter->WriteFrame(rsp, sizeof(DCSSRspCancelOrderField), source, MSG_TYPE_RSP_ORDER_ACTION, requestId);
    }
    else
    {
        mWriter->WriteErrorFrame(rsp, sizeof(DCSSRspCancelOrderField), source, MSG_TYPE_RSP_ORDER_ACTION, requestId, errorId, errorMsg);
    }
}

void ITGSpi::OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, uint8_t source, int requestId, int errorId, const char* errorMsg)
{
    if (0 == errorId)
    {
        mWriter->WriteFrame(rsp, sizeof(DCSSRspInsertOrderField), source, MSG_TYPE_RSP_ORDER_INSERT, requestId);
    }
    else
    {
        mWriter->WriteErrorFrame(rsp, 0, source, MSG_TYPE_RSP_ORDER_INSERT, requestId, errorId, errorMsg);
    }
}

void ITGSpi::OnRspQryUserInfo(const DCSSTradingAccountField* userInfo, uint8_t source, int requestId, int errorId, const char* errorMsg)
{
    if (0 == errorId)
    {
        mWriter->WriteFrame(userInfo, sizeof(DCSSTradingAccountField), source, MSG_TYPE_RSP_QRY_ACCOUNT, requestId);
    }
    else
    {
        mWriter->WriteErrorFrame(userInfo, sizeof(DCSSTradingAccountField), source, MSG_TYPE_RSP_QRY_ACCOUNT, requestId, errorId, errorMsg);
    }
}

void ITGSpi::OnRtnOrder(const DCSSOrderField* order, uint8_t source)
{
    mWriter->WriteFrame(order, sizeof(DCSSOrderField), source, MSG_TYPE_RTN_ORDER, 0);
}

void ITGSpi::OnRtnBalance(const DCSSBalanceField* balance, uint8_t source)
{
    mWriter->WriteFrame(balance, sizeof(DCSSBalanceField), source, MSG_TYPE_RTN_BALANCE, 0);
}

void ITGSpi::OnRtnTdStatus(const GWStatus& status, uint8_t source)
{
    mWriter->WriteFrame(&status, sizeof(GWStatus), source, MSG_TYPE_RTN_TD_STATUS, 0);
}

void ITGSpi::OnRspQryOrder(const DCSSRspQryOrderHeaderField* header, uint8_t source, const std::vector<DCSSOrderField>& order,
        int requestId, int errorId, const char* errorMsg)
{
    size_t length = sizeof(DCSSRspQryOrderHeaderField) + order.size() * sizeof(DCSSOrderField);
    uint8_t tmp[length];
    bzero(tmp, length);
    memcpy(tmp, header, sizeof(DCSSRspQryOrderHeaderField));

    for (int i = 0; i < order.size(); ++i)
    {
        memcpy(tmp + sizeof(DCSSRspQryOrderHeaderField) + i *sizeof(DCSSOrderField), &order[i], sizeof(DCSSOrderField));
    }
    if (errorId == 0)
        mWriter->WriteFrame(tmp, length, source, MSG_TYPE_RSP_QRY_ORDER, requestId);
    else
        mWriter->WriteErrorFrame(tmp, length, source, MSG_TYPE_RSP_QRY_ORDER, requestId, errorId, errorMsg);
}

void ITGSpi::OnRspQryKline(const DCSSKlineHeaderField* header, uint8_t source, const std::vector<DCSSKlineField>& kline,
        int requestId, int errorId, const char* errorMsg)
{
    size_t length = sizeof(DCSSKlineHeaderField) + kline.size() * sizeof(DCSSKlineField);
    uint8_t tmp[length];
    bzero(tmp, length);
    memcpy(tmp, header, sizeof(DCSSKlineHeaderField));

    for (int i = 0; i < kline.size(); ++i)
    {
        memcpy(tmp + sizeof(DCSSKlineHeaderField) + i *sizeof(DCSSKlineField), &kline[i], sizeof(DCSSKlineField));
    }
    if (errorId == 0)
        mWriter->WriteFrame(tmp, length, source, MSG_TYPE_RSP_QRY_KLINE, requestId);
    else
        mWriter->WriteErrorFrame(tmp, length, source, MSG_TYPE_RSP_QRY_KLINE, requestId, errorId, errorMsg);
}


ITGApiPtr ITGApi::CreateTGApi(uint8_t source)
{
    switch (source)
    {
    case EXCHANGE_OKCOIN:
    {
        return ITGApiPtr(new OKTGApi(EXCHANGE_OKCOIN));;
    }
    case EXCHANGE_BINANCE:
    {
        return ITGApiPtr(new BinaTGApi(EXCHANGE_BINANCE));
    }
    default:
    {
        return ITGApiPtr();
    }
    }
}

void ITGApi::RegisterSpi(ITGSpiPtr spi)
{
    if (spi.get() == nullptr)
        return;

    mSpi = spi;
    mLogger = mSpi->GetLogger();
    mProxy = mSpi->GetProxy();
}