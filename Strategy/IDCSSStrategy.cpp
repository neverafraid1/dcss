//
// Created by wangzhen on 18-6-25.
//

#include <csignal>
#include <thread>
#include <iostream>
#include <fstream>
#include "IDCSSStrategy.h"

void RegisterSignalCallback()
{
    std::signal(SIGTERM, IDCSSDataProcessor::SignalHandler);
    std::signal(SIGINT, IDCSSDataProcessor::SignalHandler);
    std::signal(SIGHUP, IDCSSDataProcessor::SignalHandler);
    std::signal(SIGQUIT, IDCSSDataProcessor::SignalHandler);
    std::signal(SIGKILL, IDCSSDataProcessor::SignalHandler);
}

IDCSSStrategy::IDCSSStrategy(const std::string& name) : mName(name)
{
    mLogger = DCSSLog::GetStrategyLogger(name, name);
    mUtil.reset(new DCSSStrategyUtil(name));
    mData.reset(new DCSSDataWrapper(this, mUtil.get()));

    RegisterSignalCallback();
}

void IDCSSStrategy::SetConfigPath(const std::string& path)
{
    mConfigPath = path;
}

void IDCSSStrategy::Init(const std::vector<uint8_t>& tgSources, const std::vector<uint8_t>& mgSources)
{
    for (auto& item : tgSources)
    {
        mData->AddRegisterTd(item);
        mSourceSet.insert(item);
    }

    for (auto& item : mgSources)
        mData->AddMarketData(item);
}

void IDCSSStrategy::Start()
{
    mData->PreRun();

    mDataThread.reset(new std::thread(&DCSSDataWrapper::Run, mData.get()));
    DCSS_LOG_INFO(mLogger, "data thread start");

    std::ifstream in(mConfigPath);
    std::ostringstream oss;
    oss << in.rdbuf();
    in.close();

    mData->Connect(oss.str());
    if (mData->IsAllLogined())
        DCSS_LOG_INFO(mLogger, "td logined");
    else
    {
        DCSS_LOG_ERROR(mLogger, "td login timeout");
        mData->Stop();
        return;
    }

    for (const auto& item : mSourceSet)
    {
        int rid = QryTradingAccount(item);
        while (!mReqReady.at(rid));
        mReqReady.erase(rid);
    }
}

IDCSSStrategy::~IDCSSStrategy()
{
    DCSS_LOG_INFO(mLogger, mName << " end!");

    mDataThread.reset();
    mData.reset();
    mLogger.reset();
    mUtil.reset();
}

void IDCSSStrategy::Terminate()
{
    Stop();

    if (mDataThread.get() != nullptr)
    {
        mDataThread.reset();
    }

    DCSS_LOG_INFO(mLogger, "[Terminate] terminated");
}

void IDCSSStrategy::Stop()
{
    if (mData.get() != nullptr)
        mData->Stop();
}

void IDCSSStrategy::Run()
{
    if (mData.get() != nullptr)
        mData->Run();
}

void IDCSSStrategy::Block()
{
    if (mDataThread.get() != nullptr)
        mDataThread->join();
}

void IDCSSStrategy::OnRtnTicker(const DCSSTickerField* ticker, uint8_t source, long recvTime)
{
    DCSS_LOG_DEBUG(mLogger, "(recv time)" << recvTime << " (source)" << source << " (tick time)" << ticker->UpdateTime << " (symbol)" << ticker->Symbol);
}

void IDCSSStrategy::OnRtnKline(const DCSSKlineField* kline, uint8_t source, long recvTime)
{
    DCSS_LOG_DEBUG(mLogger, "(recv time)" << recvTime << " (source)" << source << " (symbol)" << kline->Symbol);
}

void IDCSSStrategy::OnRtnDepth(const DCSSDepthField* depth, uint8_t source, long recvTime)
{
    DCSS_LOG_DEBUG(mLogger, "(recv time)" << recvTime << " (source)" << source << " (symbol)" << depth->Symbol);
}

void IDCSSStrategy::OnRtnBalance(const DCSSBalanceField* balance, uint8_t source, long recvTime)
{
    mBalance[source][balance->Currency].Free = balance->Free;
    mBalance[source][balance->Currency].Freezed = balance->Freezed;
}

void IDCSSStrategy::OnRtnOrder(const DCSSOrderField* order, uint8_t source, long recvTime)
{
    DCSS_LOG_DEBUG(mLogger, "(recv time)" << recvTime << " (source)" << source << " (symbol)" << order->Symbol
                                          << " (status)" << (int)order->Status);
}

void IDCSSStrategy::OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, int requestID, int errorId, const char* errorMsg, uint8_t source, long recvTime)
{
    std::stringstream ss;
    ss << " (recv time)" << recvTime << "(source)" << source;
    if (errorId != 0)
        ss << " (error id)" << errorId;
    if (errorMsg != nullptr)
        ss << " (error msg)" << errorMsg;
    DCSS_LOG_DEBUG(mLogger, ss.str().c_str());
}

void IDCSSStrategy::OnRspQryTicker(const DCSSTickerField* rsp, int requestId, int errorId, const char* errorMsg,
        uint8_t source, long recvTime)
{
    std::stringstream ss;
    ss << " (recv time)" << recvTime << "(source)" << source;
    if (errorId != 0)
        ss << " (error id)" << errorId;
    if (errorMsg != nullptr)
        ss << " (error msg)" << errorMsg;
    DCSS_LOG_DEBUG(mLogger, ss.str().c_str());
}

void IDCSSStrategy::OnRspQryKline(const DCSSKlineField* kline, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime)
{
    std::stringstream ss;
    ss << " (recv time)" << recvTime << "(source)" << source;
    if (errorId != 0)
        ss << " (error id)" << errorId;
    if (errorMsg != nullptr)
        ss << " (error msg)" << errorMsg;
    DCSS_LOG_DEBUG(mLogger, ss.str().c_str());
}

void IDCSSStrategy::OnRspQryOrder(const DCSSOrderField* rsp, int requestId, int errorId, const char* errorMsg,
        uint8_t source, long recvTime)
{
    std::stringstream ss;
    ss << " (recv time)" << recvTime << "(source)" << source;
    if (errorId != 0)
        ss << " (error id)" << errorId;
    if (errorMsg != nullptr)
        ss << " (error msg)" << errorMsg;
    DCSS_LOG_DEBUG(mLogger, ss.str().c_str());
}

void IDCSSStrategy::Debug(const char* msg)
{
    DCSS_LOG_DEBUG(mLogger, msg);
}

void IDCSSStrategy::OnTime(long curTime)
{
//    mUtil->
}

void IDCSSStrategy::OnRspQryTradingAccount(const DCSSTradingAccountField* account, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime)
{
    if (errorId == 0)
    {
        for (int i = 0; i < MAX_CURRENCY_NUM && strlen(account->Balance[i].Currency) > 0; ++i)
        {
            mBalance[source][account->Balance[i].Currency].Free = account->Balance[i].Free;
            mBalance[source][account->Balance[i].Currency].Freezed = account->Balance[i].Freezed;
        }
        std::cout << "1111111" << std::endl;
    }
    else
    {
        if (errorMsg != nullptr)
            DCSS_LOG_ERROR(mLogger, "(error id)" << errorId << " (error msg)" << errorMsg);
        else
            DCSS_LOG_ERROR(mLogger, "(error id)" << errorId);
    }
    mReqReady[requestId] = true;
}

void IDCSSStrategy::CheckOrder(uint8_t source, const std::string& symbol, double price, double volume, OrderDirection direction, OrderType type)
{
    if (!IsTdReady(source))
        throw std::runtime_error("(source)" + std::to_string(source) + " is not logined");

    if (mBalance.count(source) == 0)
        throw std::runtime_error("no balance information for (source)" + std::to_string(source));

    std::unordered_map<std::string, Balance>& balance = mBalance.at(source);

    auto pair = SplitStrSymbol(symbol);

    const std::string& base = pair.first;
    const std::string& quote = pair.second;

    switch (direction)
    {
    case OrderDirection::Buy:
    {
        double frozen = price * volume;
        if (balance.at(quote).Free < frozen)
        {
            throw std::runtime_error(quote + "'s position is not enough, we need " + std::to_string(frozen)
                    + " while there's only " + std::to_string(balance.at(quote).Free));
        }
        else
        {
            balance.at(quote).Free -= frozen;
            balance.at(quote).Freezed += frozen;
        }
        break;
    }
    case OrderDirection::Sell:
    {
        double frozen =  volume;
        if (balance.at(base).Free < frozen)
        {
            throw std::runtime_error(base + "'s position is not enough, we need " + std::to_string(frozen)
                    + " while there's only " + std::to_string(balance.at(base).Free));
        }
        else
        {
            balance.at(base).Free -= frozen;
            balance.at(base).Freezed += frozen;
        }
        break;
    }
//    case BUY_MARKET:
//    {
//        if (source == EXCHANGE_OKCOIN)
//        {
//            if (!IsEqual(volume, 0.0))
//                throw std::runtime_error("okex's buy_market order should contain no volume");
//
//            double frozen = price;
//            if (balance.at(quote).Free < frozen)
//            {
//                throw std::runtime_error(quote + "'s position is not enough, we need " + std::to_string(frozen)
//                        + " while there's only " + std::to_string(balance.at(quote).Free));
//            }
//            else
//            {
//                balance.at(quote).Free -= frozen;
//                balance.at(quote).Freezed += frozen;
//            }
//        }
//        break;
//    }
//    case SELL_MARKET:
//    {
//        if (source == EXCHANGE_OKCOIN)
//        {
//            if (!IsEqual(price, 0.0))
//                throw std::runtime_error("okex's sell_market order should contain no volume");
//
//            double frozen = volume;
//            if (balance.at(base).Free < frozen)
//            {
//                throw std::runtime_error(base + "'s position is not enough, we need " = std::to_string(frozen)
//                        + " while there's only " + std::to_string(balance.at(base).Free));
//            }
//            else
//            {
//                balance.at(base).Free -= frozen;
//                balance.at(base).Freezed += frozen;
//            }
//        }
//        break;
//    }
    default:
        throw std::runtime_error("invalid trade type");
    }
}

int IDCSSStrategy::InsertOrder(uint8_t source, const std::string& symbol, double price, double volume, OrderDirection direction, OrderType type)
{
    CheckOrder(source, symbol, price, volume, direction, type);
    return mUtil->InsertOrder(source, symbol, price, volume, direction, type);
}

int IDCSSStrategy::CancelOrder(uint8_t source, const std::string& symbol, long orderId)
{
    return mUtil->CancelOrder(source, symbol, orderId);
}

int IDCSSStrategy::QryTradingAccount(uint8_t source)
{
    int rid = mUtil->ReqQryAccount(source);
    mReqReady[rid] = false;
    return rid;
}

int IDCSSStrategy::QryTicker(uint8_t source, const std::string& symbol)
{
    DCSSReqQryTickerField req = {};
    strcpy(req.Symbol, symbol.c_str());

    return mUtil->ReqQryTicker(source, req);
}

int IDCSSStrategy::QryKline(uint8_t source, const std::string& symbol, KlineType klineType, int size, long since)
{
    DCSSReqQryKlineField req = {};
    strcpy(req.Symbol, symbol.c_str());
    req.Type = klineType;
    req.Size = size;
    req.Since = since;

    return mUtil->ReqQryKline(source, req);
}

void IDCSSStrategy::SubscribeTicker(uint8_t source, const std::string& symbol)
{
    if (!mUtil->MdSubscribeTicker(symbol, source))
        throw std::runtime_error("sub ticker failed!");

    mData->AddTicker(symbol, source);
}

void IDCSSStrategy::SubscribeKline(uint8_t source, const std::string& symbol, KlineType klineType)
{
    if (!mUtil->MdSubscribeKline(symbol, (int)klineType, source))
        throw std::runtime_error("sub kline failed!");

    mData->AddKline(symbol, klineType, source);
}

void IDCSSStrategy::SubscribeDepth(uint8_t source, const std::string& symbol, int depth)
{
    if (!mUtil->MdSubscribeDepth(symbol, depth, source))
        throw std::runtime_error("sub depth failed!");

    mData->AddDepth(symbol, depth, source);
}

bool IDCSSStrategy::IsTdReady(uint8_t source) const
{
    if (mData.get() != nullptr)
    {
        auto status = mData->GetTdStatus(source);
        if (status == GWStatus::Logined)
            return true;
    }
    return false;
}

bool IDCSSStrategy::IsTdConnected(uint8_t source) const
{
    if (mData.get() != nullptr)
    {
        auto status = mData->GetTdStatus(source);
        if (status == GWStatus::Connected
                || status == GWStatus::Logined)
            return true;
    }
    return false;
}
