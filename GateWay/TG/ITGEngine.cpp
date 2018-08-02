//
// Created by wangzhen on 18-6-13.
//

#include <thread>
#include "ITGEngine.h"
#include "OKTGApi.h"
#include "util.h"
#include "SysMessages.h"

ITGEngine::ITGEngine()
: IEngine(), mDefaultAccountIndex(-1)
{ }

void ITGEngine::Init()
{
//    mReader = UnitReader::CreateRevisableReader(Name());
//    auto tdPair = GetTdUnitPair(source);
//    mWriter = UnitSafeWriter::Create(tdPair.first, tdPair.second, Name());

}

void ITGEngine::Load(const nlohmann::json& config)
{
    mName = config["name"].get<std::string>() + "_TG";
    mFolder = config.at("folder");

    mReader = UnitReader::CreateRevisableReader(mName);

    auto iter = config.find("accounts");
    for (auto& account : iter.value())
    {
        uint8_t source = account.at("source");
        if (mApiMap.count(source) > 0)
        {
            DCSS_LOG_ERROR(mLogger, "try to add duplicae source trade engine (source)" << source);
            throw std::runtime_error("duplicate trade engine source" + std::to_string(source));
        }

        auto pApi = ITGApi::CreateTGApi(source);
        if (pApi.get() != nullptr)
        {
            pApi->Register(shared_from_this());
            pApi->LoadAccount(account);
            mApiMap[source] = pApi;
        }
        else
        {
            DCSS_LOG_ERROR(mLogger, "invalid source " << source);
        }
    }

    mWriter = UnitSafeWriter::Create(STRATEGY_BASE_FOLDER, mName, mName);
}

void ITGEngine::SetReaderThread()
{
    mReaderThread.reset(new std::thread(std::bind(&ITGEngine::Listening, this)));
}

void ITGEngine::Connect()
{
    DCSS_LOG_INFO(mLogger, "connecting...");
    for (auto& it : mApiMap)
    {
        it.second->Connect();
    }
    DCSS_LOG_INFO(mLogger, "finish connecting");
}

void ITGEngine::Disconnect()
{
    DCSS_LOG_INFO(mLogger, "disconnecting...");
    for (auto& it : mApiMap)
    {
        it.second->Disconnect();
    }
    DCSS_LOG_INFO(mLogger, "finish disconnecting");
}

bool ITGEngine::IsConnected() const
{
    bool connected = true;
    for (auto& it : mApiMap)
    {
        connected &= it.second->IsConnected();
    }
    return connected;
}

TradeAccount ITGEngine::LoadAccount(int idx, const nlohmann::json& account)
{
    DCSS_LOG_ERROR(mLogger, "[account] FUNC NOT IMPLEMENTED! (content)" << account);
    throw std::runtime_error("load account not implemented yet");
}

bool ITGEngine::RegisterClient(const std::string& name, const nlohmann::json& request)
{
    std::string folder = request.at("folder");
    int rid_s = request.at("rid_s");
    int rid_e = request.at("rid_e");
    if (mClient.IsAlive)
    {
        DCSS_LOG_ERROR(mLogger, "login already exists... (client)" << name);
    }
    else
    {
        size_t idx = mReader->AddUnit(folder, name);
        mReader->SeekTimeUnit(idx, mCurTime);
        mClient.IsAlive = true;
        mClient.UnitIndex = idx;
        mClient.RidStart = rid_s;
        mClient.RidEnd = rid_e;
    }

    Connect();
}

bool ITGEngine::RemoveClient(const std::string& name, const nlohmann::json& request)
{
    if (mClient.IsAlive)
    {
        mClient.IsAlive = false;
        Disconnect();
    }
    //TODO remove writer
}

void ITGEngine::Listening()
{
    FramePtr frame;
    while (mIsRunning && SignalReceived < 0)
    {
        frame = mReader->GetNextFrame();
        if (frame.get() != nullptr)
        {
            mCurTime = frame->GetNano();
            uint8_t source = frame->GetSource();
            FH_MSG_TP_TYPE msgType = frame->GetMsgType();

            if (msgType < 200)
            {
                if (msgType == MSG_TYPE_TRADE_ENGINE_LOGIN)
                {
                    try
                    {
                        std::string content((char*) frame->GetData());
                        nlohmann::json j_request = nlohmann::json::parse(content);
                        std::string clientName = j_request.at("name");
                        if (RegisterClient(clientName, j_request))
                            DCSS_LOG_INFO(mLogger, "[user] Accecpted: " << clientName);
                        else
                            DCSS_LOG_INFO(mLogger, "[user] Rejected: " << clientName);
                    }
                    catch (...)
                    {
                        DCSS_LOG_ERROR(mLogger, "error in parsing TRADE_ENGINE_LOGIN: " << (char*) frame->GetData());
                    }
                }
                else if (msgType == MSG_TYPE_STRATEGY_END)
                {
                    try
                    {
                        std::string content((char*) frame->GetData());
                        nlohmann::json j_request = nlohmann::json::parse(content);
                        std::string clientName = j_request.at("name");
                        if (RemoveClient(clientName, j_request))
                            DCSS_LOG_INFO(mLogger, "[user] Removed: " << clientName);
                    }
                    catch (...)
                    {
                        DCSS_LOG_ERROR(mLogger, "error in parsing STRATEGY_END: " << (char*) frame->GetData());
                    }
                }
            }
            else
            {

                if (mApiMap.count(source) == 0)
                {
                    // TODO
                    continue;
                }

                ITGApiPtr& tgApi = mApiMap.at(source);

                void* data = frame->GetData();
                int requestId = frame->GetRequestID();
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

void ITGEngine::OnRspQryTicker(const DCSSTickerField* ticker, uint8_t source, int requestId, int errorId, const char* errorMsg)
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

void ITGEngine::OnRspOrderAction(const DCSSRspCancelOrderField* rsp, uint8_t source, int requestId, int errorId, const char* errorMsg)
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

void ITGEngine::OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, uint8_t source, int requestId, int errorId, const char* errorMsg)
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

void ITGEngine::OnRspQryUserInfo(const DCSSTradingAccountField* userInfo, uint8_t source, int requestId, int errorId, const char* errorMsg)
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

void ITGEngine::OnRtnOrder(const DCSSOrderField* order, uint8_t source)
{
    mWriter->WriteFrame(order, sizeof(DCSSOrderField), source, MSG_TYPE_RTN_ORDER, 0);
}

void ITGEngine::OnRtnBalance(const DCSSBalanceField* balance, uint8_t source)
{
    mWriter->WriteFrame(balance, sizeof(DCSSBalanceField), source, MSG_TYPE_RTN_BALANCE, 0);
}

void ITGEngine::OnRtnTdStatus(const GateWayStatusType& status, uint8_t source)
{
    mWriter->WriteFrame(&status, sizeof(GateWayStatusType), source, MSG_TYPE_RTN_TD_STATUS, 0);
}

void ITGEngine::OnRspQryOrder(const DCSSRspQryOrderHeaderField* header, uint8_t source, const std::vector<DCSSRspQryOrderField>& order,
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
        mWriter->WriteFrame(tmp, length, source, MSG_TYPE_RSP_QRY_ORDER, requestId);
    else
        mWriter->WriteErrorFrame(tmp, length, source, MSG_TYPE_RSP_QRY_ORDER, requestId, errorId, errorMsg);
}

void ITGEngine::OnRspQryKline(const DCSSKlineHeaderField* header, uint8_t source, const std::vector<DCSSKlineField>& kline,
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
        ITGApiPtr ptr(new OKTGApi(EXCHANGE_OKCOIN));;
        return ptr;
    }
    default:
    {
        return ITGApiPtr();
    }
    }
}