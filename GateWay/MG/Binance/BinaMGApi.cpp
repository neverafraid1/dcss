//
// Created by wangzhen on 18-8-14.
//

#include "BinaMGApi.h"
#include "Helper.h"
#include "SymbolDao.hpp"
#include "BinanceConstant.h"

std::unordered_map<KlineType, std::string, EnumClassHash> BinaMGApi::klineStringMap = {
        {KlineType::Min1,   "1m"},
        {KlineType::Min3,   "3m"},
        {KlineType::Min5,   "5m"},
        {KlineType::Min15,  "15m"},
        {KlineType::Min30,  "30m"},
        {KlineType::Hour1,  "1h"},
        {KlineType::Hour2,  "2h"},
        {KlineType::Hour4,  "4h"},
        {KlineType::Hour6,  "6h"},
        {KlineType::Hour8,  "8h"},
        {KlineType::Hour12, "12h"},
        {KlineType::Day1,   "1d"},
        {KlineType::Day3,   "3d"},
        {KlineType::Week1,  "1w"},
        {KlineType::Month1, "1M"}
};

std::unordered_map<std::string, KlineType> BinaMGApi::stringKlineMap = {
		{"1m",	KlineType::Min1},
		{"3m",	KlineType::Min3},
		{"5m",	KlineType::Min5},
		{"15m",	KlineType::Min15},
		{"30m",	KlineType::Min30},
		{"1h",	KlineType::Hour1},
		{"2h",	KlineType::Hour2},
		{"4h",	KlineType::Hour4},
		{"6h",	KlineType::Hour6},
		{"8h",	KlineType::Hour8},
		{"12h",	KlineType::Hour12},
		{"1d",	KlineType::Day1},
		{"3d",	KlineType::Day3},
		{"1w",	KlineType::Week1},
		{"1M",	KlineType::Month1}
};

BinaMGApi::BinaMGApi(uint8_t source)
: IMGApi(source), mWsConnected(false)
{
    auto res = SymbolDao::GetAllSymbol(source);
    for (const auto& item : res)
    {
        std::string common = std::string(std::get<1>(item)).append("_").append(std::get<2>(item));
        std::string binaSymbol = std::get<0>(item);
        mCommonToBinaSymbolMap[common] = binaSymbol;
        std::transform(binaSymbol.begin(), binaSymbol.end(), binaSymbol.begin(), ::toupper);
        mBinaToCommonSymbolMap[binaSymbol] = common;
    }
}

BinaMGApi::~BinaMGApi()
{
	for (auto& item : mSubTickerClient)
	{
		item.second->close().get();
	}
	mSubTickerClient.clear();

	for (auto& item : mSubDepthClient)
	{
		item.second->close().get();
	}
	mSubDepthClient.clear();

	for (auto& item : mSubKlineClient)
	{
		item.second->close().get();
	}
	mSubKlineClient.clear();
//    mWsClient->close(websocket_close_status::normal, "destruct");
}

void BinaMGApi::Connect()
{
	mWsConnected = true;
	DCSS_LOG_INFO(mLogger, "[bina]connect to binance success");
}

void BinaMGApi::OnWsConnect()
{
    DCSS_LOG_INFO(mLogger, "connect to binance success!");
    mWsConnected = true;
}

std::shared_ptr<websocket_callback_client> BinaMGApi::CreateNewWsClient()
{
	std::shared_ptr<websocket_callback_client> client;
	if (mProxy.empty())
	{
		client.reset(new websocket_callback_client());
	}
	else
	{
		web_proxy proxy(mProxy);
		websocket_client_config config;
		config.set_proxy(proxy);
		client.reset(new websocket_callback_client(config));
	}

	return client;
}

std::string BinaMGApi::GetTickerChannel(const std::string& symbol)
{
    std::string binaSymbol = mCommonToBinaSymbolMap.at(symbol);
    std::transform(binaSymbol.begin(), binaSymbol.end(), binaSymbol.begin(), ::tolower);

    return BinanceConstant::WS_API_BASE_URL+ binaSymbol + "@ticker";
}

std::string BinaMGApi::GetKlineChannel(const std::string& symbol, KlineType type)
{
	std::string binaSymbol = mCommonToBinaSymbolMap.at(symbol);
	std::transform(binaSymbol.begin(), binaSymbol.end(), binaSymbol.begin(), ::tolower);

	return BinanceConstant::WS_API_BASE_URL + binaSymbol + "@kline_" + klineStringMap.at(type);
}

std::string BinaMGApi::GetDepthChannel(const std::string& symbol, int depth)
{
    std::string binaSymbol = mCommonToBinaSymbolMap.at(symbol);
    std::transform(binaSymbol.begin(), binaSymbol.end(), binaSymbol.begin(), ::tolower);

    return BinanceConstant::WS_API_BASE_URL + binaSymbol + "@depth" + std::to_string(depth);
}

bool BinaMGApi::IsConnected() const
{
    return mWsConnected;
}

void BinaMGApi::ReqSubTicker(const std::string& symbol)
{
    if (!mWsConnected)
    {
        DCSS_LOG_ERROR(mLogger, "not connected to exchage now!");
        return;
    }

    if (mSubTickNum.count(symbol) > 0)
    {
        ++mSubTickNum.at(symbol);
        return;
    }

    mSubTickNum[symbol] = 1;

    std::string channel = GetTickerChannel(symbol);

    std::shared_ptr<websocket_callback_client> client = CreateNewWsClient();
    client->set_message_handler(std::bind(&BinaMGApi::OnWsTicker, this, std::placeholders::_1));
    client->set_close_handler(std::bind(&BinaMGApi::OnWsClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    try
    {
    	client->connect(channel).get();
    }
    catch (const std::exception& e)
    {
    	DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << " sub failed!(exception)" << e.what());
    	--mSubTickNum.at(symbol);
    	return;
    }

    mSubTickerClient[channel] = std::move(client);

    DCSS_LOG_INFO(mLogger, "[binance mg](channel)" << channel << " sub success");
}

void BinaMGApi::ReqSubDepth(const std::string& symbol, int depth)
{
    if (!mWsConnected)
    {
        DCSS_LOG_ERROR(mLogger, "[binance]not connected to exchange now!");
        return;
    }

    if (depth != 5 && depth != 10 && depth != 20)
    {
        DCSS_LOG_ERROR(mLogger, "[binance]invalid depth " << depth);
        return;
    }

    if (mSubDepthNum.count(symbol) > 0 && mSubDepthNum.at(symbol).count(depth) > 0)
    {
        ++mSubDepthNum.at(symbol).at(depth);
        return;
    }

    mSubDepthNum[symbol][depth] = 1;

    std::string channel = GetDepthChannel(symbol, depth);

    std::shared_ptr<websocket_callback_client> client = CreateNewWsClient();
    client->set_message_handler(std::bind(&BinaMGApi::OnWsMessage, this, std::placeholders::_1));
    client->set_close_handler(std::bind(&BinaMGApi::OnWsClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    try
    {
    	client->connect(channel).get();
    }
    catch (const std::exception& e)
    {
    	DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << " sub failed!(exception)" << e.what());
    	--mSubDepthNum.at(symbol).at(depth);
    	return;
    }

    mSubDepthClient[channel] = std::move(client);

    DCSS_LOG_INFO(mLogger, "[binance mg](channel)" << channel << " sub success");
}

void BinaMGApi::ReqSubKline(const std::string& symbol, KlineType klineType)
{
    if (!mWsConnected)
    {
        DCSS_LOG_ERROR(mLogger, "[binance]not connected to exchange now!");
        return;
    }

    if (klineStringMap.count(klineType) == 0)
    {
        DCSS_LOG_ERROR(mLogger, "[binance]unsupported kline type " << (int)klineType);
        return;
    }

    if (mSubKlineNum.count(symbol) > 0 && mSubKlineNum.at(symbol).count(klineType) > 0)
    {
        ++mSubKlineNum.at(symbol).at(klineType);
        return;
    }

    mSubKlineNum[symbol][klineType] = 1;

    std::string channel = GetKlineChannel(symbol, klineType);

    std::shared_ptr<websocket_callback_client> client = CreateNewWsClient();
    client->set_message_handler(std::bind(&BinaMGApi::OnWsKline, this, std::placeholders::_1));
    client->set_close_handler(std::bind(&BinaMGApi::OnWsClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    try
    {
    	client->connect(channel).get();
    }
    catch (const std::exception& e)
    {
    	DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << " sub failed!(exception)" << e.what());
    	--mSubKlineNum.at(symbol).at(klineType);
    	return;
    }

    mSubKlineClient[channel] = std::move(client);

    DCSS_LOG_INFO(mLogger, "[binance mg](channel)" << channel << " sub success");
}

void BinaMGApi::ReqUnSubTicker(const std::string& symbol)
{
	if (mSubTickNum.count(symbol) == 0 || --mSubTickNum.at(symbol) > 0)
		return;

	std::string channel = GetTickerChannel(symbol);
	if (mSubTickerClient.count(channel) == 0)
		return;

	auto client = mSubTickerClient.erase(mSubTickerClient.find(channel));
	try
	{
		client->second->close(websocket_close_status::normal, "unsub (channel)" + channel).get();
	}
	catch (const std::exception& e)
	{
		DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << "unsub failed!(exception)" << e.what());
	}

	DCSS_LOG_INFO(mLogger, "[binance mg](channel)" << channel << " unnub success");

	mSubTickNum.erase(symbol);
}

void BinaMGApi::ReqUnSubKline(const std::string& symbol, KlineType klineType)
{
	if (mSubKlineNum.count(symbol) == 0 ||
			mSubKlineNum.at(symbol).count(klineType) == 0 ||
			--mSubKlineNum.at(symbol).at(klineType) > 0)
		return;

	std::string channel = GetKlineChannel(symbol, klineType);
	if (mSubKlineClient.count(channel) == 0)
		return;

	auto client = mSubKlineClient.erase(mSubKlineClient.find(channel));
	try
	{
		client->second->close(websocket_close_status::normal, "unsub (channel)" + channel).get();
	}
	catch (const std::exception& e)
	{
		DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << "unsub failed!(exception)" << e.what());
	}

	DCSS_LOG_INFO(mLogger, "[binance mg](channel)" << channel << " unnub success");

	mSubKlineNum.at(symbol).erase(klineType);
	if (mSubKlineNum.at(symbol).empty())
		mSubKlineNum.erase(symbol);
}

void BinaMGApi::ReqUnSubDepth(const std::string& symbol, int depth)
{
	if (mSubDepthNum.count(symbol) == 0 ||
			mSubDepthNum.at(symbol).count(depth) == 0 ||
			--mSubDepthNum.at(symbol).at(depth) > 0)
		return;

	std::string channel = GetDepthChannel(symbol, depth);
	if (mSubDepthClient.count(channel) == 0)
		return;

	auto client = mSubDepthClient.erase(mSubDepthClient.find(channel));
	try
	{
		client->second->close(websocket_close_status::normal, "unsub (channel)" + channel).get();
	}
	catch (const std::exception& e)
	{
		DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << "unsub failed!(exception)" << e.what());
	}

	DCSS_LOG_INFO(mLogger, "[binance mg](channel)" << channel << " unnub success");
	mSubDepthNum.at(symbol).erase(depth);
	if (mSubDepthNum.at(symbol).empty())
		mSubDepthNum.erase(symbol);
}

void BinaMGApi::OnWsMessage(const websocket_incoming_message& msg)
{
    json::value jv = json::value::parse(msg.extract_string().get());
    if (jv.is_object())
    {
        const json::object jo = jv.as_object();
        const std::string& event = jo.at("e").as_string();
        if (event.compare("kline") == 0)
            OnRtnKline(jo);
        else if(event.compare("24hrTicker") == 0)
            OnRtnTicker(jo);
    }
}

void BinaMGApi::OnWsTicker(const websocket_incoming_message& msg)
{
	json::value jv = json::value::parse(msg.extract_string().get());
	const json::object& jo = jv.as_object();
	OnRtnTicker(jo);
}

void BinaMGApi::OnWsKline(const websocket_incoming_message& msg)
{
	json::value jv = json::value::parse(msg.extract_string().get());
	const json::object& jo = jv.as_object();
	OnRtnKline(jo);
}

void BinaMGApi::OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error)
{

}

void BinaMGApi::OnRtnKline(const json::object& jo)
{
    const json::object& jkline = jo.at("k").as_object();

    DCSSKlineField kline;
    strcpy(kline.Symbol, mBinaToCommonSymbolMap.at(jo.at("s").as_string()).c_str());
    kline.UpdateTime = jo.at("E").as_number().to_int64();
    kline.Type = stringKlineMap.at(jkline.at("i").as_string());
    kline.OpenPrice = std::stod(jkline.at("o").as_string());
    kline.ClosePrice = std::stod(jkline.at("c").as_string());
    kline.Highest = std::stod(jkline.at("h").as_string());;
    kline.Lowest = std::stod(jkline.at("l").as_string());
    kline.Volume = std::stod(jkline.at("v").as_string());
    kline.StartTime = jkline.at("t").as_number().to_int64();
    kline.CloseTime = jkline.at("T").as_number().to_int64();

    mSpi->OnRtnKline(&kline, mSourceId);
}

void BinaMGApi::OnRtnTicker(const json::object& jo)
{
    DCSSTickerField ticker;
    ticker.UpdateTime = jo.at("E").as_number().to_int64();
    strcpy(ticker.Symbol, mBinaToCommonSymbolMap.at(jo.at("s").as_string()).c_str());
    ticker.LastPrice = std::stod(jo.at("w").as_string());
    ticker.BuyPrice = std::stod(jo.at("b").as_string());
    ticker.SellPrice = std::stod(jo.at("a").as_string());
    ticker.Highest = std::stod(jo.at("h").as_string());
    ticker.Lowest = std::stod(jo.at("l").as_string());
    ticker.Volume = std::stod(jo.at("v").as_string());

    mSpi->OnRtnTicker(&ticker, mSourceId);
}
