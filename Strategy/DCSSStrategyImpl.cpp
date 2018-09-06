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

void RegisterSignalCallback()
{
    std::signal(SIGTERM, IDCSSDataProcessor::SignalHandler);
    std::signal(SIGINT, IDCSSDataProcessor::SignalHandler);
    std::signal(SIGHUP, IDCSSDataProcessor::SignalHandler);
    std::signal(SIGQUIT, IDCSSDataProcessor::SignalHandler);
}

DCSSStrategyImpl::DCSSStrategyImpl(const std::string& name, IDCSSStrategy* strategy)
: mName(name), mStrategy(strategy)
{
    mLogger = DCSSLog::GetStrategyLogger(name, name);
    mUtil.reset(new DCSSStrategyUtil(name));
    mData.reset(new DCSSDataWrapper(this, mUtil.get()));

    RegisterSignalCallback();
}

DCSSStrategyImpl::~DCSSStrategyImpl()
{
    DCSS_LOG_INFO(mLogger, mName << " end!");

    mDataThread.reset();
    mData.reset();
    mLogger.reset();
    mUtil.reset();
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
        std::unique_lock<std::mutex> lock(mMutex);
        while (!mReqReady.at(rid))
        	mCondVar.wait(lock);
        mReqReady.erase(rid);
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
    return mUtil->InsertOrder(source, symbol, price, volume, direction, type);
}

int DCSSStrategyImpl::CancelOrder(uint8_t source, const std::string& symbol, long orderId)
{
    return mUtil->CancelOrder(source, symbol, orderId);
}

int DCSSStrategyImpl::QryOrder(uint8_t source, const std::string& symbol, long orderId)
{
	return mUtil->ReqQryOrder(source, symbol, orderId);
}

int DCSSStrategyImpl::QryTradingAccount(uint8_t source)
{
    int rid = mUtil->ReqQryAccount(source);
    mReqReady[rid] = false;
    return rid;
}

int DCSSStrategyImpl::QryOpenOrder(uint8_t source)
{
	DCSSReqQryOrderField req;
	int rid = mUtil->ReqQryOpenOrder(source, req);
	mReqReady[rid] = false;
	return rid;
}

int DCSSStrategyImpl::QryTicker(uint8_t source, const std::string& symbol)
{
    DCSSReqQryTickerField req = {};
    strcpy(req.Symbol, symbol.c_str());

    return mUtil->ReqQryTicker(source, req);
}

int DCSSStrategyImpl::QryKline(uint8_t source, const std::string& symbol, KlineType klineType, int size, long since)
{
    DCSSReqQryKlineField req = {};
    strcpy(req.Symbol, symbol.c_str());
    req.Type = klineType;
    req.Size = size;
    req.Since = since;

    return mUtil->ReqQryKline(source, req);
}

void DCSSStrategyImpl::SubscribeTicker(uint8_t source, const std::string& symbol)
{
    if (!mUtil->MdSubscribeTicker(symbol, source))
        throw std::runtime_error("sub ticker failed!");

    mData->AddTicker(symbol, source);
}

void DCSSStrategyImpl::SubscribeKline(uint8_t source, const std::string& symbol, KlineType klineType)
{
    if (!mUtil->MdSubscribeKline(symbol, (int)klineType, source))
        throw std::runtime_error("sub kline failed!");

    mData->AddKline(symbol, klineType, source);
}

void DCSSStrategyImpl::SubscribeDepth(uint8_t source, const std::string& symbol, int depth)
{
    if (!mUtil->MdSubscribeDepth(symbol, depth, source))
        throw std::runtime_error("sub depth failed!");

    mData->AddDepth(symbol, depth, source);
}

DCSSLog* DCSSStrategyImpl::GetLogger() const
{
	return mLogger.get();
}

void DCSSStrategyImpl::OnRtnTicker(const DCSSTickerField* ticker, uint8_t source, long recvTime)
{
	mStrategy->OnRtnTicker(ticker, source, recvTime);
}

void DCSSStrategyImpl::OnRtnKline(const DCSSKlineField* kline, uint8_t source, long recvTime)
{
	mStrategy->OnRtnKline(kline, source, recvTime);
}

void DCSSStrategyImpl::OnRtnDepth(const DCSSDepthField* depth, uint8_t source, long recvTime)
{
	mStrategy->OnRtnDepth(depth, source, recvTime);
}

void DCSSStrategyImpl::OnRtnOrder(const DCSSOrderField* order, uint8_t source, long recvTime)
{
	mStrategy->OnRtnOrder(order, source, recvTime);
}

void DCSSStrategyImpl::OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, int requestID, int errorId, const char* errorMsg, uint8_t source, long recvTime)
{
	mStrategy->OnRspOrderInsert(rsp, requestID, errorId, errorMsg, source, recvTime);
}

void DCSSStrategyImpl::OnRspQryTicker(const DCSSTickerField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime)
{
	mStrategy->OnRspQryTicker(rsp, requestId, errorId, errorMsg, source, recvTime);
}

void DCSSStrategyImpl::OnRspQryKline(const DCSSKlineField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime)
{
	mStrategy->OnRspQryKline(rsp, requestId, errorId, errorMsg, source, recvTime);
}

void DCSSStrategyImpl::OnRspQryOrder(const DCSSOrderField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime)
{
	mStrategy->OnRspQryOrder(rsp, requestId, errorId, errorMsg, source, recvTime);
}

void DCSSStrategyImpl::Debug(const char* msg)
{
	DCSS_LOG_DEBUG(mLogger, msg);
}

void DCSSStrategyImpl::OnTime(long curTime)
{

}

std::string DCSSStrategyImpl::GetName() const
{
	return mName;
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
    mReqReady[requestId] = true;
    mCondVar.notify_one();
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
