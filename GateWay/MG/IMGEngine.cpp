//
// Created by wangzhen on 18-6-15.
//

#include <thread>
#include "util.h"
#include "IMGEngine.h"
#include "OKEX/OKMGApi.h"
#include "Binance/BinaMGApi.h"
#include "Timer.h"
#include "SysMessages.h"

IMGEngine::~IMGEngine()
{
	mMGApi.reset();
	mReader.reset();
	mWriter.reset();
}

void IMGEngine::SetReaderThread()
{
    mReaderThread.reset(new std::thread(std::bind(&IMGEngine::Listening, this)));
}

void IMGEngine::Load(const nlohmann::json& config)
{
    mLogger = DCSSLog::GetLogger(Name());
    uint8_t source = config.at("source");
    auto pair = GetMdUnitPair(source);
    mMGApi = IMGApi::Create(source);
    mMGApi->Register(shared_from_this());
    mMGApi->SetProxy(config.at("proxy").get<std::string>());
    Connect();
    while (!IsConnected())
    {
        DCSS_LOG_INFO(mLogger, "mg is not connected now (source)" << source);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    mWriter = UnitWriter::Create(pair.first, pair.second, mMGApi->Name());
    mReader = UnitReader::CreateRevisableReader(mMGApi->Name());
}

void IMGEngine::Connect()
{
    if (mMGApi == nullptr)
    {
        DCSS_LOG_ERROR(mLogger, "mg is not created!");
        return;
    }

    mMGApi->Connect();
}

bool IMGEngine::IsConnected() const
{
    return mMGApi == nullptr ? false : mMGApi->IsConnected();
}

void IMGEngine::Listening()
{
    FramePtr frame;
    while (mIsRunning && SignalReceived < 0)
    {
        frame = mReader->GetNextFrame();
        if (frame != nullptr)
        {
            uint8_t source = frame->GetSource();
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
                mMGApi->ReqSubKline(req->Symbol, req->Type);
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
                mMGApi->ReqUnSubKline(req->Symbol, req->Type);
                break;
            }
            default:break;
            }
        }
    }

    if (SignalReceived >= 0)
    {
        DCSS_LOG_INFO(mLogger, "signal received: " << SignalReceived);
    }

    if (!mIsRunning)
    {
        DCSS_LOG_INFO(mLogger, "forced to stop.");
    }
}

void IMGEngine::OnRtnTicker(const DCSSTickerField* ticker, uint8_t source)
{
    if (mIsRunning)
    {
        mWriter->WriteFrame(ticker, sizeof(DCSSTickerField), source, MSG_TYPE_RTN_TICKER, true, -1);
    }
}

void IMGEngine::OnRtnDepth(const DCSSDepthField* depth, uint8_t source)
{
    if (mIsRunning)
    {
        mWriter->WriteFrame(depth, sizeof(DCSSDepthField), source, MSG_TYPE_RTN_DEPTH, true, -1);
    }
}

void IMGEngine::OnRtnKline(const DCSSKlineField* kline, uint8_t source)
{
    if (mIsRunning)
    {

//        mWriter->WriteFrame(kline, sizeof(DCSSKlineField), source, MSG_TYPE_RTN_KLINE, true, -1);
    }
}

IMGApiPtr IMGApi::Create(uint8_t source)
{
    switch (source)
    {
    case EXCHANGE_OKCOIN:
        return IMGApiPtr(new OKMGApi(source));
    case EXCHANGE_BINANCE:
    	return IMGApiPtr(new BinaMGApi(source));
    default:
        return IMGApiPtr();
    }
}

IMGApi::IMGApi(uint8_t source)
: mSourceId(source), mSpi(nullptr), mLogger(nullptr)
{}

void IMGApi::Register(IMGEnginePtr spi)
{
	if (!spi)
		return;
    mSpi = spi.get();
    mLogger = mSpi->GetLogger().get();
}

void IMGApi::SetProxy(const std::string& proxy)
{
	mProxy = proxy;
}

short IMGApi::GetSource() const
{
    return mSourceId;
}
