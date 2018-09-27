/*
 * IDCSSStrategyImpl.cpp
 *
 *  Created on: 2018年8月30日
 *      Author: wangzhen
 */

#include <csignal>
#include <fstream>
#include "json.hpp"
#include "DCSSStrategyImpl.h"
#include "IDCSSStrategy.h"
#include "SysMessages.h"
#include "Timer.h"

void RegisterSignalCallback()
{
    std::signal(SIGTERM, DCSSStrategyImpl::SignalHandler);
    std::signal(SIGINT, DCSSStrategyImpl::SignalHandler);
    std::signal(SIGHUP, DCSSStrategyImpl::SignalHandler);
    std::signal(SIGQUIT, DCSSStrategyImpl::SignalHandler);
}

DCSSStrategyImpl::DCSSStrategyImpl(const std::string& name, IDCSSStrategy* strategy)
: mName(name), StrategyUtil(name), mCurNano(0), mMdNano(0), mStrategy(strategy), mQryResult(nullptr), mWaitingRid(0), mErrorMsg(nullptr)
{
    mLogger = DCSSLog::GetStrategyLogger(name, name);
    mData.reset(new DCSSDataWrapper(this));

    RegisterSignalCallback();
}

DCSSStrategyImpl::~DCSSStrategyImpl()
{
    DCSS_LOG_INFO(mLogger, mName << " end!");

    mDataThread.reset();
    mData.reset();
    mLogger.reset();
}

bool DCSSStrategyImpl::Init(const std::string& path)
{
	DCSS_LOG_INFO(mLogger, "load config information from " << path);

	mConfigPath = path;

    std::ifstream in(mConfigPath);
    std::ostringstream oss;
    oss << in.rdbuf();
    in.close();

    mConfig = oss.str();

    nlohmann::json object = nlohmann::json::parse(mConfig);
    if (object.count("accounts") == 0)
    {
    	DCSS_LOG_ERROR(mLogger, "config must contains account info");
    	return false;
    }

    const nlohmann::json::array_t& accounts = object.at("accounts");
    for (const auto& item : accounts)
    {
    	if (item.count("source") == 0 ||
    			item.count("api_key") == 0 ||
				item.count("secret_key") == 0)
    	{
    		DCSS_LOG_ERROR(mLogger, "config has invalid info");
    		return false;
    	}
    	short source = item.at("source");
    	mData->AddRegisterTd(source);
    	mData->AddMarketData(source);
    	mSourceSet.insert(source);
    }

    return true;
}

void DCSSStrategyImpl::Start()
{
    mData->PreRun();

    mDataThread.reset(new std::thread(&DCSSDataWrapper::Run, mData.get()));
    DCSS_LOG_INFO(mLogger, "data thread start");

    mData->Connect(mConfig);
    if (mData->IsAllLogined())
        DCSS_LOG_INFO(mLogger, "all td logined");
    else
    {
        DCSS_LOG_ERROR(mLogger, "td login timeout");
        for (auto& item : mSourceSet)
        {
        	DCSS_LOG_INFO(mLogger, "(source)" << (int)item << "(status)" << (int)mData->GetTdStatus(item));
        }
        mData->Stop();
        return;
    }

    for (const auto& item : mSourceSet)
    {
        int rid = QryTradingAccount(item);
        std::unique_lock<std::mutex> lock(mMutex);
        mCondVar.wait(lock);

        DCSS_LOG_INFO(mLogger, "finish qry trading account for (source)" << (int)item);
    }
}

void DCSSStrategyImpl::Terminate()
{
    Stop();

    if (mDataThread != nullptr)
    {
        mDataThread.reset();
    }

    DCSS_LOG_INFO(mLogger, "[Terminate] terminated");
}

void DCSSStrategyImpl::Stop()
{
    if (mData != nullptr)
        mData->Stop();
}

void DCSSStrategyImpl::Run()
{
    if (mData != nullptr)
        mData->Run();
}

void DCSSStrategyImpl::Block()
{
    if (mDataThread != nullptr)
        mDataThread->join();
}

int DCSSStrategyImpl::InsertOrder(uint8_t source, const std::string& symbol, double price, double volume, OrderDirection direction, OrderType type)
{
    CheckOrder(source, symbol, price, volume, direction, type);

    int rid = GetRid();

    DCSSReqInsertOrderField req = {};
    strcpy(req.Symbol, symbol.c_str());
    req.Price = price;
    req.Amount = volume;
    req.Direction = direction;
    req.Type = type;

    WriteFrameExtra(&req, sizeof(DCSSReqInsertOrderField), source,
            MSG_TYPE_REQ_ORDER_INSERT, true, rid, GetNanoTime());
    return rid;
}

int DCSSStrategyImpl::CancelOrder(uint8_t source, const std::string& symbol, long orderId)
{
    int rid = GetRid();

    DCSSReqCancelOrderField req = {};
    strcpy(req.Symbol, symbol.c_str());
    req.OrderID = orderId;
    WriteFrame(&req, sizeof(DCSSReqCancelOrderField), source, MSG_TYPE_REQ_ORDER_ACTION, true, rid);

    return rid;
}

int DCSSStrategyImpl::QryOrder(uint8_t source, const std::string& symbol, long orderId)
{
    int rid = GetRid();

    DCSSReqQryOrderField req = {};
    strcpy(req.Symbol, symbol.c_str());
    req.OrderID = orderId;
    WriteFrame(&req, sizeof(DCSSReqQryOrderField), source, MSG_TYPE_REQ_QRY_SIGNAL_ORDER, true, rid);

    return rid;
}

int DCSSStrategyImpl::QryTradingAccount(uint8_t source)
{
    char tmp = {};

    int rid = GetRid();

    WriteFrame(&tmp, sizeof(char), source, MSG_TYPE_REQ_QRY_ACCOUNT, true, rid);

    return rid;
}

int DCSSStrategyImpl::QryOpenOrder(uint8_t source)
{
	DCSSReqQryOrderField req;
    int rid = GetRid();

    WriteFrame(&req, sizeof(DCSSReqQryOrderField), source, MSG_TYPE_REQ_QRY_OPEN_ORDER, true, rid);

    return rid;
}

int DCSSStrategyImpl::QryTicker(uint8_t source, const std::string& symbol)
{
    DCSSReqQryTickerField req = {};
    strcpy(req.Symbol, symbol.c_str());

    int rid = GetRid();

    WriteFrame(&req, sizeof(DCSSReqQryTickerField), source, MSG_TYPE_REQ_QRY_TICKER, true, rid);

    return rid;
}

int DCSSStrategyImpl::QryKline(uint8_t source, const std::string& symbol, KlineType klineType, int size, long since)
{
    DCSSReqQryKlineField req = {};
    strcpy(req.Symbol, symbol.c_str());
    req.Type = klineType;
    req.Size = size;
    req.Since = since;

    int rid = GetRid();

    WriteFrame(&req, sizeof(DCSSReqQryKlineField), source, MSG_TYPE_REQ_QRY_KLINE, true, rid);

    return rid;
}

int DCSSStrategyImpl::QrySymbol(uint8_t source, const std::string& symbol)
{
    DCSSReqQrySymbolField req = {};
    strcpy(req.Symbol, symbol.c_str());

    int rid = GetRid();

    WriteFrame(&req, sizeof(DCSSReqQrySymbolField), source, MSG_TYPE_REQ_QRY_SYMBOL, true, rid);

    return rid;
}

bool DCSSStrategyImpl::QrySymbolSync(uint8_t source, const std::string& symbol, DCSSSymbolField& symbolField, std::string& errorMsg, int timeOut)
{
    DCSSReqQrySymbolField req = {};
    strcpy(req.Symbol, symbol.c_str());

    int rid = GetRid();

    mWaitingRid = rid;

    WriteFrame(&req, sizeof(DCSSReqQrySymbolField), source, MSG_TYPE_REQ_QRY_SYMBOL, true, rid);

    std::unique_lock<std::mutex> lock(mMutex);

    if (mCondVar.wait_for(lock, std::chrono::seconds(timeOut)) == std::cv_status::timeout)
    {
        errorMsg = std::string("timeout");
        return false;
    }
    else
    {
        if (mQryResult)
        {
            symbolField = *(reinterpret_cast<DCSSSymbolField*>(mQryResult));
            mQryResult = nullptr;
            return true;
        }
        else
        {
            if (mErrorMsg)
            {
                errorMsg = std::string(mErrorMsg);
                mErrorMsg = nullptr;
            }
            return false;
        }
    }
}

bool DCSSStrategyImpl::QryCurrencyBalance(uint8_t source, const std::string& currency, DCSSBalanceField& balance, std::string& errorMsg)
{
    errorMsg.clear();
    if (mBalance.count(source) == 0)
    {
        errorMsg.append("no balance information for source: ").append(1, source);
        return false;
    }
    if (mBalance.at(source).count(currency) == 0)
    {
        errorMsg.append("no balance information for currency: ").append(currency);
        return false;
    }

    strcpy(balance.Currency, currency.c_str());
    balance.Free = mBalance.at(source).at(currency).Free;
    balance.Freezed = mBalance.at(source).at(currency).Freezed;

    return true;
}

void DCSSStrategyImpl::SubscribeTicker(uint8_t source, const std::string& symbol)
{
    if (!StrategyUtil::MdSubscribeTicker(symbol, source))
        throw std::runtime_error("sub ticker failed!");

    mData->AddTicker(symbol, source);
}

void DCSSStrategyImpl::SubscribeKline(uint8_t source, const std::string& symbol, KlineType klineType)
{
    if (!StrategyUtil::MdSubscribeKline(symbol, (int)klineType, source))
        throw std::runtime_error("sub kline failed!");

    mData->AddKline(symbol, klineType, source);
}

void DCSSStrategyImpl::SubscribeDepth(uint8_t source, const std::string& symbol)
{
    if (!StrategyUtil::MdSubscribeDepth(symbol, source))
        throw std::runtime_error("sub depth failed!");

    mData->AddDepth(symbol, source);
}

DCSSLog* DCSSStrategyImpl::GetLogger() const
{
	return mLogger.get();
}

std::string DCSSStrategyImpl::GetName() const
{
    return mName;
}

void DCSSStrategyImpl::OnRtnTicker(const DCSSTickerField* ticker, uint8_t source, long recvTime)
{
    mStrategy->OnRtnTicker(ticker, source, recvTime);
}

void DCSSStrategyImpl::OnRtnKline(const DCSSKlineField* kline, uint8_t source,
            long recvTime)
{
    mStrategy->OnRtnKline(kline, source, recvTime);
}

void DCSSStrategyImpl::OnRtnDepth(const DCSSDepthField* depth, uint8_t source,
        long recvTime)
{
    mStrategy->OnRtnDepth(depth, source, recvTime);
}

void DCSSStrategyImpl::OnRtnOrder(const DCSSOrderField* order, uint8_t source,
        long recvTime)
{
    mStrategy->OnRtnOrder(order, source, recvTime);
}

void DCSSStrategyImpl::OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, int requestId,
        int errorId, const char* errorMsg, uint8_t source,
        long recvTime)
{
    mStrategy->OnRspOrderInsert(rsp, requestId, errorId, errorMsg, source, recvTime);
}

void DCSSStrategyImpl::OnRspQryTicker(const DCSSTickerField* rsp, int requestId, int errorId,
        const char* errorMsg, uint8_t source, long recvTime)
{
    mStrategy->OnRspQryTicker(rsp, requestId, errorId, errorMsg, source, recvTime);
}

void DCSSStrategyImpl::OnRspQryKline(const DCSSKlineField* kline, int requestId, int errorId,
        const char* errorMsg, uint8_t source, long recvTime)
{
    mStrategy->OnRspQryKline(kline, requestId, errorId, errorMsg, source, recvTime);
}

void DCSSStrategyImpl::OnRspQryOrder(const DCSSOrderField* rsp, int requestId, int errorId,
        const char* errorMsg, uint8_t source, long recvTime)
{
    mStrategy->OnRspQryOrder(rsp, requestId, errorId, errorMsg, source, recvTime);
}

void DCSSStrategyImpl::OnRspQrySymbol(const DCSSSymbolField* rsp, int requestId, int errorId,
        const char* errorMsg, uint8_t source, long recvTime)
{
    if (mWaitingRid == requestId)
    {
        if (errorId != 0)
        {
            mErrorMsg = const_cast<char*>(errorMsg);
        }
        else
        {
            mQryResult = (void*)rsp;
        }
        mWaitingRid = 0;
        std::unique_lock<std::mutex> lck(mMutex);
        mCondVar.notify_one();
    }
    else
        mStrategy->OnRspQrySymbol(rsp, requestId, errorId, errorMsg, source, recvTime);
}

void DCSSStrategyImpl::OnRspQryTradingAccount(const DCSSTradingAccountField* account, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime)
{
    if (errorId == 0)
    {
        for (int i = 0; i < MAX_CURRENCY_NUM && strlen(account->Balance[i].Currency) > 0; ++i)
        {
            mBalance[source][account->Balance[i].Currency].Free = account->Balance[i].Free;
            mBalance[source][account->Balance[i].Currency].Freezed = account->Balance[i].Freezed;
        }
    }
    else
    {
        if (errorMsg != nullptr)
            DCSS_LOG_ERROR(mLogger, "(error id)" << errorId << " (error msg)" << errorMsg);
        else
            DCSS_LOG_ERROR(mLogger, "(error id)" << errorId);
    }
    std::unique_lock<std::mutex> lck(mMutex);
    mCondVar.notify_one();
}

void DCSSStrategyImpl::OnTime(long curTime)
{
    mCurNano = curTime;
}

void DCSSStrategyImpl::OnRtnBalance(const DCSSBalanceField* balance, uint8_t source, long recvTime)
{
    mBalance[source][balance->Currency].Free = balance->Free;
    mBalance[source][balance->Currency].Freezed = balance->Freezed;
}

void DCSSStrategyImpl::CheckOrder(uint8_t source, const std::string& symbol, double price, double volume, OrderDirection direction, OrderType type)
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

std::pair<std::string, std::string> DCSSStrategyImpl::SplitStrSymbol(const std::string& symbol)
{
	auto idx = symbol.find('_');
	if (idx == std::string::npos) return std::move(std::make_pair("", ""));

	return std::move(std::make_pair(symbol.substr(0, idx), symbol.substr(idx + 1, symbol.length() - idx - 1)));
}

bool DCSSStrategyImpl::IsTdReady(uint8_t source) const
{
    if (mData != nullptr)
    {
        auto status = mData->GetTdStatus(source);
        if (status == GWStatus::Logined)
            return true;
    }
    return false;
}

bool DCSSStrategyImpl::IsTdConnected(uint8_t source) const
{
    if (mData != nullptr)
    {
        auto status = mData->GetTdStatus(source);
        if (status == GWStatus::Connected
                || status == GWStatus::Logined)
            return true;
    }
    return false;
}
