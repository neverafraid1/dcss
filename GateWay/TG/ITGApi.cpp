//
// Created by wangzhen on 18-8-5.
//

#include "ITGApi.h"
#include "OKEX/OKTGApi.h"
#include "Binance/BinaTGApi.h"
#include "Timer.h"
#include "SysMessages.h"

ITGSpi::ITGSpi(const std::string& client, DCSSLogPtr logger, const std::string& proxy)
: mClient(client), mSignalReceived(-1), mIsRunning(false), mLogger(std::move(logger)), mProxy(proxy), mDefaultAccountIndex(-1), mCurTime(0)
{
    mWriter = UnitSafeWriter::Create(STRATEGY_BASE_FOLDER, mClient + "_TG", mClient + "_TG");
}

ITGSpi::~ITGSpi()
{
	mApiMap.clear();
	mReader.reset();
	mWriter.reset();
}

bool ITGSpi::Load(const std::string& config)
{
    mReader = UnitReader::Create(STRATEGY_BASE_FOLDER, mClient, GetNanoTime());
    nlohmann::json conf = nlohmann::json::parse(config);
    std::string folder = conf.at("folder");
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
    return true;
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
    if (mReader == nullptr)
    {
        throw std::runtime_error("reader is not inited! please call init() before start()");
    }

    mIsRunning = true;

    FramePtr frame;
    while (mIsRunning && mSignalReceived < 0)
    {
        frame = mReader->GetNextFrame();
        if (frame != nullptr)
        {
            mCurTime = frame->GetNano();
            uint8_t source = frame->GetSource();
            FH_MSG_TP_TYPE msgType = frame->GetMsgType();


            if (mApiMap.count(source) == 0 || !mApiMap.at(source)->IsLogged())
            {
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

void ITGSpi::OnRspQryTicker(const DCSSTickerField* ticker, uint8_t source, bool isLast, int requestId, int errorId, const char* errorMsg)
{
	mWriter->WriteErrorFrame(ticker, sizeof(DCSSTickerField), source, MSG_TYPE_RSP_QRY_TICKER, isLast, requestId, errorId, errorMsg);
}

void ITGSpi::OnRspOrderAction(const DCSSRspCancelOrderField* rsp, uint8_t source, bool isLast, int requestId, int errorId, const char* errorMsg)
{
	mWriter->WriteErrorFrame(rsp, sizeof(DCSSRspCancelOrderField), source, MSG_TYPE_RSP_ORDER_ACTION, isLast, requestId, errorId, errorMsg);
}

void ITGSpi::OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, uint8_t source, bool isLast, int requestId, int errorId, const char* errorMsg)
{
	mWriter->WriteErrorFrame(rsp, 0, source, MSG_TYPE_RSP_ORDER_INSERT, isLast, requestId, errorId, errorMsg);
}

void ITGSpi::OnRspQryUserInfo(const DCSSTradingAccountField* userInfo, uint8_t source, bool isLast, int requestId, int errorId, const char* errorMsg)
{
	mWriter->WriteErrorFrame(userInfo, sizeof(DCSSTradingAccountField), source, MSG_TYPE_RSP_QRY_ACCOUNT, isLast, requestId, errorId, errorMsg);
}

void ITGSpi::OnRtnOrder(const DCSSOrderField* order, uint8_t source)
{
    mWriter->WriteFrame(order, sizeof(DCSSOrderField), source, MSG_TYPE_RTN_ORDER, true, 0);
}

void ITGSpi::OnRtnBalance(const DCSSBalanceField* balance, uint8_t source)
{
    mWriter->WriteFrame(balance, sizeof(DCSSBalanceField), source, MSG_TYPE_RTN_BALANCE, true, 0);
}

void ITGSpi::OnRtnTdStatus(const GWStatus& status, uint8_t source)
{
    mWriter->WriteFrame(&status, sizeof(GWStatus), source, MSG_TYPE_RTN_TD_STATUS, true, 0);
}

void ITGSpi::OnRspQryOrder(const DCSSOrderField* order, uint8_t source, bool isLast,
        int requestId, int errorId, const char* errorMsg)
{
	mWriter->WriteErrorFrame(order, sizeof(DCSSOrderField), source, MSG_TYPE_RSP_QRY_ORDER, isLast, requestId, errorId, errorMsg);
}

void ITGSpi::OnRspQryKline(const DCSSKlineField* kline, uint8_t source, bool isLast,
        int requestId, int errorId, const char* errorMsg)
{
	mWriter->WriteErrorFrame(kline, sizeof(DCSSKlineField), source, MSG_TYPE_RSP_QRY_KLINE, isLast, requestId, errorId, errorMsg);
}


ITGApi::ITGApi(uint8_t source)
: mSourceId(source), mSpi(nullptr), mLogger(nullptr)
{}

ITGApiPtr ITGApi::CreateTGApi(uint8_t source)
{
    switch (source)
    {
    case EXCHANGE_OKCOIN:
    {
        return ITGApiPtr(new OKTGApi(EXCHANGE_OKCOIN));
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
    if (spi == nullptr)
        return;

    mSpi = spi.get();
    mLogger = mSpi->GetLogger().get();
    mProxy = mSpi->GetProxy();
}
