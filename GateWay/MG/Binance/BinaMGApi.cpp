//
// Created by wangzhen on 18-8-14.
//

#include <cpprest/http_client.h>
#include "BinaMGApi.h"
#include "Helper.h"
#include "SymbolDao.hpp"
#include "BinanceConstant.h"
#include "Timer.h"
#include <limits>

using namespace web::http;
using namespace web::http::client;

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

BinaMGApi::BinaMGApi()
: IMGApi(ExchangeEnum::Binance), mWsConnected(false)
{
    auto res = SymbolDao::GetAllSymbol(ExchangeEnum::Binance);
    for (const auto& item : res)
    {
        DCSSSymbolField symbolField = item;

        std::string common = std::string(symbolField.Currency.BaseCurrency).append("_").append(symbolField.Currency.QuoteCurrecy);
        std::transform(common.begin(), common.end(), common.begin(), ::tolower);

        std::string binaSymbol(symbolField.Symbol);
        std::transform(binaSymbol.begin(), binaSymbol.end(), binaSymbol.begin(), ::toupper);
        strcpy(symbolField.Symbol, binaSymbol.c_str());

        mBinaToCommonSymbolMap[symbolField.Symbol] = common;
        mCommonToBinaSymbolMap[common] = symbolField;
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
    std::string binaSymbol = mCommonToBinaSymbolMap.at(symbol).Symbol;
    std::transform(binaSymbol.begin(), binaSymbol.end(), binaSymbol.begin(), ::tolower);

    return BinanceConstant::WS_API_BASE_URL+ binaSymbol + "@ticker";
}

std::string BinaMGApi::GetKlineChannel(const std::string& symbol, KlineType type)
{
	std::string binaSymbol = mCommonToBinaSymbolMap.at(symbol).Symbol;
	std::transform(binaSymbol.begin(), binaSymbol.end(), binaSymbol.begin(), ::tolower);

	return BinanceConstant::WS_API_BASE_URL + binaSymbol + "@kline_" + klineStringMap.at(type);
}

std::string BinaMGApi::GetDepthChannel(const std::string& symbol)
{
    std::string binaSymbol = mCommonToBinaSymbolMap.at(symbol).Symbol;
    std::transform(binaSymbol.begin(), binaSymbol.end(), binaSymbol.begin(), ::tolower);

    return BinanceConstant::WS_API_BASE_URL + binaSymbol + "@depth";
}

void BinaMGApi::GetDepthSnapShot(const std::string& symbol)
{
    mBidDepth.erase(symbol);
    mAskDepth.erase(symbol);
    mLastUpdateIdMap.erase(symbol);

    http_client_config http_config;
    http_config.set_validate_certificates(false);
    if (!mProxy.empty())
    {
        web_proxy proxy(mProxy);
        http_config.set_proxy(proxy);
    }

    http_client restClient("https://www.binance.com", http_config);
    http_request request(methods::GET);

    uri_builder builder("api/v1/depth");
    builder.append_query("symbol",
            std::string(mCommonToBinaSymbolMap.at(symbol).Symbol));
    builder.append_query("limit", MAX_DEPTH_NUM);

    request.set_request_uri(builder.to_uri());

    try
    {
        auto& bidDepth = mBidDepth[symbol];
        auto& askDepth = mAskDepth[symbol];

        auto response = restClient.request(request).get();
        json::value jv = response.extract_json().get();

        const json::array& bids = jv.at("bids").as_array();
        for (const auto& item : bids)
        {
            const json::array& bid = item.as_array();

            double price = std::stod(bid.at(0).as_string());
            if (!IsEqual(price, 0.0))
                bidDepth.insert(std::make_pair(price, std::stod(bid.at(1).as_string())));
        }

        const json::array& asks = jv.at("asks").as_array();
        for (const auto& item : asks)
        {
            const json::array& ask = item.as_array();
            double price = std::stod(ask.at(0).as_string());
            if (!IsEqual(price, 0.0))
                askDepth.insert(std::make_pair(std::stod(ask.at(0).as_string()), std::stod(ask.at(1).as_string())));
        }

        mLastUpdateIdMap[symbol] = std::make_pair(jv.at("lastUpdateId").as_number().to_int64(), std::numeric_limits<long>::max());
    }
    catch (const std::exception& e)
    {
        DCSS_LOG_INFO(mLogger, "[binance mg] (channel) get depth error (exception) " << e.what());
    }
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
    	client->connect(channel).wait();
    }
    catch (const std::exception& e)
    {
    	DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << " sub failed!(exception)" << e.what());
    	--mSubTickNum.at(symbol);
    	return;
    }

    mSubTickerClient[channel] = client;

    DCSS_LOG_INFO(mLogger, "[binance mg](channel)" << channel << " sub success");
}

void BinaMGApi::ReqSubDepth(const std::string& symbol)
{
    if (!mWsConnected)
    {
        DCSS_LOG_ERROR(mLogger, "[binance]not connected to exchange now!");
        return;
    }

    if (mSubDepthNum.count(symbol) > 0)
    {
        ++mSubDepthNum.at(symbol);
        return;
    }

    mSubDepthNum[symbol] = 1;

    std::string binaSymbol = mCommonToBinaSymbolMap.at(symbol).Symbol;
        std::transform(binaSymbol.begin(), binaSymbol.end(), binaSymbol.begin(), ::tolower);

    std::string channel = GetDepthChannel(symbol);

    std::shared_ptr<websocket_callback_client> client = CreateNewWsClient();
    client->set_message_handler(std::bind(&BinaMGApi::OnWsDepth, this, std::placeholders::_1));
    client->set_close_handler(std::bind(&BinaMGApi::OnWsClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    try
    {
        client->connect(channel).wait();
    }
    catch (const std::exception& e)
    {
        DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << " sub failed!(exception)" << e.what());
        --mSubDepthNum.at(symbol);
        return;
    }

    mSubDepthClient[channel] = client;

    GetDepthSnapShot(symbol);

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
    	client->connect(channel).wait();
    }
    catch (const std::exception& e)
    {
    	DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << " sub failed!(exception)" << e.what());
    	--mSubKlineNum.at(symbol).at(klineType);
    	return;
    }

    mSubKlineClient[channel] = client;

    DCSS_LOG_INFO(mLogger, "[binance mg](channel)" << channel << " sub success");
}

void BinaMGApi::ReqUnSubTicker(const std::string& symbol)
{
	if (mSubTickNum.count(symbol) == 0 || --mSubTickNum.at(symbol) > 0)
		return;

	std::string channel = GetTickerChannel(symbol);
	if (mSubTickerClient.count(channel) == 0)
		return;

	auto client = mSubTickerClient.at(channel);
	try
	{
		client->close(websocket_close_status::normal, "unsub (channel)" + channel).wait();
	}
	catch (const std::exception& e)
	{
		DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << "unsub failed!(exception)" << e.what());
	}

	DCSS_LOG_INFO(mLogger, "[binance mg](channel)" << channel << " unnub success");

	mSubTickerClient.erase(channel);
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

	auto client = mSubKlineClient.at(channel);
	try
	{
		client->close(websocket_close_status::normal, "unsub (channel)" + channel).wait();
	}
	catch (const std::exception& e)
	{
		DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << "unsub failed!(exception)" << e.what());
	}

	DCSS_LOG_INFO(mLogger, "[binance mg](channel)" << channel << " unnub success");

	mSubKlineClient.erase(channel);
	mSubKlineNum.at(symbol).erase(klineType);
	if (mSubKlineNum.at(symbol).empty())
		mSubKlineNum.erase(symbol);
}

void BinaMGApi::ReqUnSubDepth(const std::string& symbol)
{
	if (mSubDepthNum.count(symbol) == 0 || --mSubDepthNum.at(symbol) > 0)
		return;

	std::string channel = GetDepthChannel(symbol);

	if (mSubDepthClient.count(channel) == 0)
		return;

	auto client = mSubDepthClient.at(channel);
	try
	{
		client->close(websocket_close_status::normal, "unsub (channel)" + channel).wait();
	}
	catch (const std::exception& e)
	{
		DCSS_LOG_ERROR(mLogger, "[binance mg](channel)" << channel << "unsub failed!(exception)" << e.what());
	}

	DCSS_LOG_INFO(mLogger, "[binance mg](channel)" << channel << " unnub success");
	mSubDepthClient.erase(channel);
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

void BinaMGApi::OnWsDepth(const websocket_incoming_message& msg)
{
    json::value jv = json::value::parse(msg.extract_string().get());

    const std::string& binaSymbol = jv.at("s").as_string();
    const std::string& commSymbol = mBinaToCommonSymbolMap.at(binaSymbol);

    if (mLastUpdateIdMap.count(commSymbol) == 0)
        return;

    long firstUpdateId = jv.at("U").as_number().to_int64();
    long finalUpdateId = jv.at("u").as_number().to_int64();
    long updateTime = jv.at("E").as_number().to_int64();

    auto& item = mLastUpdateIdMap.at(commSymbol);
    auto& bidDepth = mBidDepth.at(commSymbol);
    auto& askDepth = mAskDepth.at(commSymbol);

    if (finalUpdateId <= item.first)
        return;

    if (firstUpdateId > item.first + 1)
    {
        GetDepthSnapShot(commSymbol);
        return;
    }

    item.first = finalUpdateId;
    item.second = updateTime;

    const json::array& bids = jv.at("b").as_array();
    for (const auto& item : bids)
    {
        const json::array& bid = item.as_array();
        double price = std::stod(bid.at(0).as_string());
        if (IsEqual(price, 0.0))
            bidDepth.erase(price);
        else
            bidDepth[price] = std::stod(bid.at(1).as_string());
    }

    const json::array& asks = jv.at("a").as_array();
    for (const auto& item : asks)
    {
        const json::array& ask = item.as_array();
        double price = std::stod(ask.at(0).as_string());
        if (IsEqual(price, 0.0))
            askDepth.erase(price);
        else
            askDepth[price] = std::stod(ask.at(1).as_string());
    }

    DCSSDepthField depth;
    strcpy(depth.Symbol, commSymbol.c_str());
    depth.UpdateTime = mLastUpdateIdMap.at(commSymbol).second;

    int index = 0;
    for (auto iter = bidDepth.begin(); iter != bidDepth.end() && index < MAX_DEPTH_NUM; ++iter, ++index)
    {
        depth.BidDepth[index].Price = iter->first;
        depth.BidDepth[index].Volume = iter->second;
    }

    index = 0;
    for (auto iter = askDepth.begin(); iter != askDepth.end() && index < MAX_DEPTH_NUM; ++iter, ++index)
    {
        depth.AskDepth[index].Price = iter->first;
        depth.AskDepth[index].Volume = iter->second;
    }

    depth.SendTime = GetNanoTime();
    mSpi->OnRtnDepth(&depth, mSourceId);

}

void BinaMGApi::OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error)
{

}

void BinaMGApi::OnRtnKline(const json::object& jo)
{
    DCSSKlineField kline;
    strcpy(kline.Symbol, mBinaToCommonSymbolMap.at(jo.at("s").as_string()).c_str());
    kline.UpdateTime = jo.at("E").as_number().to_int64();
    kline.Type = stringKlineMap.at(jo.at("i").as_string());
    kline.OpenPrice = std::stod(jo.at("o").as_string());
    kline.ClosePrice = std::stod(jo.at("c").as_string());
    kline.Highest = std::stod(jo.at("h").as_string());;
    kline.Lowest = std::stod(jo.at("l").as_string());
    kline.Volume = std::stod(jo.at("v").as_string());
    kline.StartTime = jo.at("t").as_number().to_int64();
    kline.CloseTime = jo.at("T").as_number().to_int64();

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
