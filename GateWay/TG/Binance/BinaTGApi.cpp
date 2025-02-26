//
// Created by wangzhen on 18-8-6.
//

#include <openssl/hmac.h>
#include <algorithm>

#include "BinaTGApi.h"
#include "Timer.h"
#include "Helper.h"
#include "SymbolDao.hpp"
#include "BinanceConstant.h"
using namespace BinanceConstant;

std::unordered_map<OrderDirection, std::string, EnumClassHash> BinaTGApi::enumDirectionMap = {
        {OrderDirection::Buy,   "BUY"},
        {OrderDirection::Sell,  "SELL"}
};

std::unordered_map<std::string, OrderDirection> BinaTGApi::directionEnumMap = {
        {"BUY",     OrderDirection::Buy},
        {"SELL",    OrderDirection::Sell}
};

std::unordered_map<OrderType, std::string, EnumClassHash> BinaTGApi::enumOrderTypeMap = {
        {OrderType::Limit,  "LIMIT"},
        {OrderType::Market, "MARKET"}
};

std::unordered_map<std::string, OrderType> BinaTGApi::orderTypeEnumMap = {
        {"LIMIT",   OrderType::Limit},
        {"MARKET",  OrderType::Market}
};

std::unordered_map<KlineType, std::string, EnumClassHash> BinaTGApi::klineMap = {
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

std::unordered_map<std::string, OrderStatus> BinaTGApi::statusEnumMap =
{
		{"NEW",					OrderStatus::Submitted},
		{"PARTIALLY_FILLED",	OrderStatus::PartTraded},
		{"FILLED",				OrderStatus::AllTraded},
		{"CANCELED",			OrderStatus::AllCanceled},
		{"PENDING_CANCEL ",		OrderStatus::Canceling},
		{"REJECTED",			OrderStatus::Rejected},
		{"EXPIRED",				OrderStatus::Expired}
};

BinaTGApi::BinaTGApi()
: ITGApi(ExchangeEnum::Binance), mWsConnected(false), mAccountLastUpdateTime(0), mLogined(false), mRestClient(nullptr), mWsClient(nullptr)
{
    auto res = SymbolDao::GetAllSymbol(ExchangeEnum::Binance);
    for (const auto& item : res)
    {
        std::string common = std::string(item.Currency.BaseCurrency).append("_").append(item.Currency.QuoteCurrecy);
        std::transform(common.begin(), common.end(), common.begin(), ::tolower);
        mBinaToCommonSymbolMap[item.Symbol] = common;
        mCommonToBinaSymbolMap[common] = item;
    }
}

BinaTGApi::~BinaTGApi()
{
    mWsClient->close(websocket_close_status::normal, "destruct").get();
    mPingThread->join();

    if (mRestClient)
    	delete mRestClient;
    if (mWsClient)
    	delete mWsClient;
}

void BinaTGApi::LoadAccount(const nlohmann::json& config)
{
    mApiKey = config.at("api_key");
    mSecretKey = config.at("secret_key");
    if (config.count("proxy") > 0)
    	mProxy = config.at("proxy");
}

void BinaTGApi::Connect()
{
	ResetRestClient();

    if (mProxy.empty())
    {
        mWsClient = new websocket_callback_client();
    }
    else
    {
        web_proxy proxy(U(mProxy));
        websocket_client_config ws_config;
        ws_config.set_proxy(proxy);

        mWsClient = new websocket_callback_client(ws_config);
    }

    mWsClient->set_message_handler(std::bind(&BinaTGApi::OnWsMessage, this, std::placeholders::_1));
    mWsClient->set_close_handler(std::bind(&BinaTGApi::OnWsClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    http_request request(methods::POST);
    request.headers().add(X_MBX_APIKEY, mApiKey);
    uri_builder builder(USER_DATA_STREAM);
    request.set_request_uri(builder.to_uri());
    auto response = mRestClient->request(request).get();
    mListenKey = response.extract_json().get().as_object().at("listenKey").as_string();

    mWsClient->connect(WS_API_BASE_URL + mListenKey)
            .then([=]()
            {
                OnWsConnected();
            })
            .then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[trade gateway][binance] connect to ws failed! (exception)" << e.what());
                }

            });
}

void BinaTGApi::Disconnect()
{
    if (mWsConnected)
    {
        mWsClient->close(websocket_close_status::normal, "disconnect by client");
        mWsConnected = false;
        mSpi->OnRtnTdStatus(GWStatus::Disconnected, mSourceId);
    }
}

void BinaTGApi::Login()
{
    DCSS_LOG_INFO(mLogger, "[trade gateway][binance] login success!");
    mLogined = true;
    mSpi->OnRtnTdStatus(GWStatus::Logined, mSourceId);
}

bool BinaTGApi::IsConnected() const
{
    return mWsConnected;
}

bool BinaTGApi::IsLogged() const
{
    return mLogined;
}

void BinaTGApi::ReqQryTicker(const DCSSReqQryTickerField* req, int requestID)
{
	if (mCommonToBinaSymbolMap.count(req->Symbol) == 0)
	{
		DCSS_LOG_ERROR(mLogger, "invalid symbol " << req->Symbol);
		return;
	}

	ResetRestClient();

	http_request request(methods::GET);
	request.headers().add(X_MBX_APIKEY, mApiKey);

	uri_builder builder(PRICE);
	builder.append_query("symbol", mCommonToBinaSymbolMap.at(req->Symbol).Symbol);

	request.set_request_uri(builder.to_uri());
	mRestClient->request(request).then([=](http_response response)
	{
    	OnRspQryTicker(response, req, requestID);
	}).then(
			[=](pplx::task<void> task)
			{
				try
				{
					task.get();
				}
				catch (const std::exception& e)
				{
					DCSS_LOG_ERROR(mLogger, "send qry ticker failed (exception)" << e.what());
				}
            });
}

void BinaTGApi::ReqQryUserInfo(int requestID)
{
	ResetRestClient();

    http_request request(methods::GET);
    request.headers().add(X_MBX_APIKEY, mApiKey);

    uri_builder builder(ACCOUNT_INFO);
    AddTimeStamp(builder);
    HMAC_SHA256(builder);

    request.set_request_uri(builder.to_uri());
    mRestClient->request(request)
            .then(
                    [=](http_response response)
                    {
                        OnRspQryUserInfo(response, requestID);
                    })
            .then(
                    [=](pplx::task<void> task)
                    {
                        try
                        {
                            task.get();
                        }
                        catch (const std::exception& e)
                        {
                            DCSS_LOG_ERROR(mLogger, "[trade gateway][binance][qry user info] "
                                                    "send request failed!(expection)" << e.what());
                        }
                    }
            );

}

void BinaTGApi::ReqQryOrder(const DCSSReqQryOrderField* req, int requestID)
{
	ResetRestClient();

	http_request request(methods::GET);
	request.headers().add(X_MBX_APIKEY, mApiKey);

	uri_builder builder(ORDER);
	builder.append_query("symbol", mCommonToBinaSymbolMap.at(req->Symbol).Symbol);
	builder.append_query("orderId", req->OrderID);
	AddTimeStamp(builder);
	HMAC_SHA256(builder);

	request.set_request_uri(builder.to_uri());

	mRestClient->request(request).then([=](http_response response)
	{
		OnRspQryOrder(response, req, requestID);
	}).then(
			[=](pplx::task<void> task)
			{
				try
				{
					task.get();
				}
				catch (const std::exception& e)
				{
					DCSS_LOG_ERROR(mLogger, "[trade gateway][binance][qry order] send request failed!(exception)" << e.what());
				}
			});
}

void BinaTGApi::ReqQryOpenOrder(const DCSSReqQryOrderField* req, int requestID)
{
	ResetRestClient();

	http_request request(methods::GET);
	request.headers().add(X_MBX_APIKEY, mApiKey);

	uri_builder builder("/api/v3/openOrders");
	if (strlen(req->Symbol) > 0)
		builder.append_query("symbol", mCommonToBinaSymbolMap.at(req->Symbol).Symbol);
	AddTimeStamp(builder);
	HMAC_SHA256(builder);

	request.set_request_uri(builder.to_uri());
	mRestClient->request(request).then([=](http_response response)
	{
		OnRspQryOpenOrder(response, req, requestID);
	}).then(
			[=](pplx::task<void> task)
			{
				try
				{
					task.get();
				}
				catch (const std::exception& e)
				{
					DCSS_LOG_ERROR(mLogger, "[trade gateway][binance][qry open order] send request failed!(exception)" << e.what());
				}
			});
}

void BinaTGApi::ReqQryKline(const DCSSReqQryKlineField* req, int requestID)
{
    if (mCommonToBinaSymbolMap.count(req->Symbol) == 0)
    {
        DCSS_LOG_ERROR(mLogger, "error symbol " << req->Symbol);
        return;
    }

    ResetRestClient();

    http_request request(methods::GET);
	request.headers().add(X_MBX_APIKEY, mApiKey);

    uri_builder builder(KLINE);
    builder.append_query("symbol", mCommonToBinaSymbolMap.at(req->Symbol).Symbol);
    builder.append_query("interval", klineMap.at(req->Type));
    if (req->Size > 0)
        builder.append_query("limit", req->Size);
    if (req->Since > 0)
        builder.append_query("startTime", req->Since);

    request.set_request_uri(builder.to_uri());
    mRestClient->request(request)
            .then(
                    [=](http_response response)
                    {
                        OnRspQryKline(response, req, requestID);
                    }

            ).then(
                    [=](pplx::task<void> task)
                    {
                        try
                        {
                            task.get();
                        }
                        catch (const std::exception& e)
                        {

                        }
                    }

            );
}

void BinaTGApi::ReqInsertOrder(const DCSSReqInsertOrderField* req, int requestID)
{
    if (mCommonToBinaSymbolMap.count(req->Symbol) == 0)
    {
        DCSS_LOG_ERROR(mLogger, "error symbol " << req->Symbol);
        return;
    }

    ResetRestClient();

    http_request request(methods::POST);
    request.headers().add(X_MBX_APIKEY, mApiKey);

    uri_builder builder(ORDER);
    builder.append_query("symbol", mCommonToBinaSymbolMap.at(req->Symbol).Symbol);
    builder.append_query("side", enumDirectionMap.at(req->Direction));
    builder.append_query("type", enumOrderTypeMap.at(req->Type));
    builder.append_query("timeInForce", "GTC");
    builder.append_query("quantity", req->Amount);
    if (!IsEqual(req->Price, 0.0))
        builder.append_query("price", req->Price);

    AddTimeStamp(builder);
    HMAC_SHA256(builder);

    request.set_request_uri(builder.to_uri());
    mRestClient->request(request)
            .then(
                    [=](http_response response)
                    {
                        OnRspOrderInsert(response, req, requestID);
                    }

            ).then(
                    [=](pplx::task<void> task)
                    {
                        try
                        {
                            task.get();
                        }
                        catch (const std::exception& e)
                        {

                        }
                    }

            );
}

void BinaTGApi::ReqCancelOrder(const DCSSReqCancelOrderField* req, int requestID)
{
    if (mCommonToBinaSymbolMap.count(req->Symbol) == 0)
    {
        DCSS_LOG_ERROR(mLogger, "error symbol " << req->Symbol);
        return;
    }

    ResetRestClient();

    http_request request(methods::DEL);
    request.headers().add(X_MBX_APIKEY, mApiKey);

    uri_builder builder(ORDER);
    builder.append_query("symbol",
            mCommonToBinaSymbolMap.at(req->Symbol).Symbol);
    builder.append_query("orderId", req->OrderID);

    AddTimeStamp(builder);
    HMAC_SHA256(builder);

    request.set_request_uri(builder.to_uri());
    mRestClient->request(request).then([=](http_response response)
    {
        OnRspCancelOrder(response, req, requestID);
    }

    ).then([=](pplx::task<void> task)
    {
        try
        {
            task.get();
        }
        catch (const std::exception& e)
        {

        }
    }

    );
}

void BinaTGApi::ReqQrySymbol(const DCSSReqQrySymbolField* req, int requestID)
{
    DCSSSymbolField symbol = {};
    if (mCommonToBinaSymbolMap.count(req->Symbol) > 0)
    {
        symbol = mCommonToBinaSymbolMap.at(req->Symbol);
        mSpi->OnRspQrySymbol(&symbol, mSourceId, true, requestID);
    }
    else
        mSpi->OnRspQrySymbol(&symbol, mSourceId, true, requestID, 1, "not found");
}

/// rsp ///
void BinaTGApi::OnWsMessage(const websocket_incoming_message& msg)
{
    json::value jv = json::value::parse(msg.extract_string().get());
    const json::object& jo = jv.as_object();
    const std::string& event = jo.at("e").as_string();
    if (event == "outboundAccountInfo")
    {
        OnRtnAccount(jo);
    }
    else if (event == "executionReport")
    {
        OnRtnOrder(jo);
    }
}

void BinaTGApi::OnWsClose(websocket_close_status close_status, const utility::string_t& reason,
        const std::error_code& error)
{
	DCSS_LOG_INFO(mLogger, "[trade gateway][binance] ws close due to " << reason);
	mSpi->OnRtnTdStatus(GWStatus::Disconnected, mSourceId);
    mWsConnected = false;

    if (close_status != websocket_close_status::normal)
    {
    	std::thread t(std::bind(&BinaTGApi::Connect, this));
    }
}

void BinaTGApi::OnRtnAccount(const json::object& jo)
{
    long updateTime = jo.at("E").as_number().to_int64();
    if (updateTime <= mAccountLastUpdateTime)
        return;

    mAccountLastUpdateTime = updateTime;
    json::array balanceArray = jo.at("B").as_array();

    for (auto& item : balanceArray)
    {
        DCSSBalanceField balanceField;
        const json::object& balance = item.as_object();
        double free = std::stod(balance.at("f").as_string());
        double freezed = std::stod(balance.at("l").as_string());
        if (IsEqual(free, 0.0) && IsEqual(freezed, 0.0))
            continue;

        std::string asset = balance.at("a").as_string();
        std::transform(asset.begin(), asset.end(), asset.begin(), ::tolower);
        strcpy(balanceField.Currency, asset.c_str());
        balanceField.Free = free;
        balanceField.Freezed = freezed;

        mSpi->OnRtnBalance(&balanceField, mSourceId);
    }
}

void BinaTGApi::OnRtnOrder(const json::object& jo)
{
    std::string symbol = jo.at("s").as_string();
    if (mBinaToCommonSymbolMap.count(symbol) == 0)
    {
        DCSS_LOG_ERROR(mLogger, "error symbol " << symbol);
        return;
    }

    long updateTime = jo.at("E").as_number().to_int64();
    int orderId = jo.at("i").as_integer();
    if (updateTime < mOrderLastUpdateTime[orderId])
        return;

    std::string type = jo.at("o").as_string();
    if (orderTypeEnumMap.count(type) == 0)
    {
        DCSS_LOG_ERROR(mLogger, "unsupported order type " << type);
        return;
    }

    std::string direction = jo.at("S").as_string();
    if (directionEnumMap.count(direction) == 0)
    {
        DCSS_LOG_ERROR(mLogger, "unsupported direction " << direction);
        return;
    }

    mOrderLastUpdateTime[orderId] = updateTime;

    DCSSOrderField orderField;
    orderField.Exchange = ExchangeEnum::Binance;
    strcpy(orderField.Symbol, mBinaToCommonSymbolMap.at(symbol).c_str());
    orderField.InsertTime = jo.at("T").as_number().to_int64();
    orderField.OrderID = orderId;
    orderField.Type = orderTypeEnumMap.at(type);
    orderField.Direction = directionEnumMap.at(direction);
    orderField.OriginQuantity = std::stod(jo.at("q").as_string());
    orderField.ExecuteQuantity = std::stod(jo.at("z").as_string());
    orderField.UpdateTime = jo.at("T").as_number().to_int64();

    mSpi->OnRtnOrder(&orderField, mSourceId);
}

void BinaTGApi::OnWsConnected()
{
    DCSS_LOG_INFO(mLogger, "[trade gateway][binance] ws connect success!");
    mWsConnected = true;

    mSpi->OnRtnTdStatus(GWStatus::Connected, mSourceId);

    mPingThread.reset(new std::thread(&BinaTGApi::Ping, this));
    Login();
}

void BinaTGApi::Ping()
{
//    http_request request(methods::POST);
//    request.headers().add("X-MBX-APIKEY", mApiKey);
//    uri_builder builder("/api/v1/userDataStream");
//    request.set_request_uri(builder.to_uri());
//    auto response = mRestClient->request(request).get();
//    mListenKey = response.extract_json().get().as_object().at("listenKey").as_string();
    int loop(0);
    while (mWsConnected)
    {
        if (++loop >= 30 * 60)
        {
            uri_builder builder(USER_DATA_STREAM);
            builder.append_query("listenKey", mListenKey);
            mRestClient->request(methods::PUT, builder.to_string());
            loop = 0;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void BinaTGApi::ResetRestClient()
{
	http_client_config http_config;
	http_config.set_validate_certificates(false);
	if (mProxy.empty())
	{
		if (!mRestClient)
			mRestClient = new http_client(API_BASE_URL, http_config);
	}
	else
	{
		web_proxy proxy(mProxy);
		http_config.set_proxy(proxy);
		if (mRestClient)
			delete mRestClient;
		mRestClient = new http_client(API_BASE_URL, http_config);
	}
}

void BinaTGApi::OnRspQryUserInfo(http_response& response, int requestID)
{
    try
    {
        const json::value jv = response.extract_json().get();
        const json::object& jobj = jv.as_object();

        DCSSTradingAccountField rsp;
        if (jobj.find("code") == jobj.end())
        {
            int index = 0;

            const json::array& jarray = jobj.at("balances").as_array();
            for(const auto& item : jarray)
            {
                const json::object& balance = item.as_object();
                double free = std::stod(balance.at("free").as_string());
                double locked = std::stod(balance.at("locked").as_string());
                if (IsEqual(free, 0.0) && IsEqual(locked, 0.0))
                    continue;

                const std::string& asset = balance.at("asset").as_string();
                memcpy(rsp.Balance[index].Currency, asset.c_str(), asset.length());
                rsp.Balance[index].Free = free;
                rsp.Balance[index].Freezed = locked;
             }

             mSpi->OnRspQryUserInfo(&rsp, mSourceId, true, requestID);
        }
        else
        {
            mSpi->OnRspQryUserInfo(&rsp, mSourceId, true, requestID, jobj.at("code").as_integer(), jobj.at("msg").as_string().c_str());
        }
    }
    catch (const std::exception& e)
    {
        DCSS_LOG_ERROR(mLogger, "parse rsp failed(exception)" << e.what());
    }
}

void BinaTGApi::OnRspQryKline(http_response& response, const DCSSReqQryKlineField* req, int requestID)
{
    try
    {
        const json::value jv = response.extract_json().get();
        DCSSKlineField kline;
        strcpy(kline.Symbol, req->Symbol);
        kline.Type = req->Type;
        if (jv.is_array())
        {
            const json::array& arr = jv.as_array();

            size_t idx(0);
            for (const auto& item : arr)
            {
            	++idx;
                if (item.is_array())
                {
                    const json::array& klineArr = item.as_array();
                    kline.UpdateTime = klineArr.at(0).as_number().to_int64();
                    kline.OpenPrice = std::stod(klineArr.at(1).as_string());
                    kline.Highest = std::stod(klineArr.at(2).as_string());
                    kline.Lowest = std::stod(klineArr.at(3).as_string());
                    kline.ClosePrice = std::stod(klineArr.at(4).as_string());
                    kline.Volume = std::stod(klineArr.at(5).as_string());

                    mSpi->OnRspQryKline(&kline, mSourceId, idx == arr.size(), requestID);
                }
            }
        }
        else if (jv.is_object())
        {
            const json::object& obj = jv.as_object();
            if (obj.find("code") != obj.end())
            {
                mSpi->OnRspQryKline(&kline, mSourceId, true,
                        requestID, obj.at("code").as_integer(), obj.at("msg").as_string().c_str());
            }
        }
    }
    catch (const std::exception& e)
    {
        DCSS_LOG_ERROR(mLogger, "error during parse (exception)" << e.what());
    }
}

void BinaTGApi::OnRspQryTicker(http_response& response, const DCSSReqQryTickerField* req, int requestID)
{
	try
	{
		const json::value jv = response.extract_json().get();
		const json::object& jo = jv.as_object();
		DCSSTickerField ticker;
		strcpy(ticker.Symbol, req->Symbol);
		if (jo.find("code") == jo.end())
		{
			ticker.LastPrice = std::stod(jo.at("price").as_string());

			mSpi->OnRspQryTicker(&ticker, mSourceId, true, requestID);
		}
		else
		{
			mSpi->OnRspQryTicker(&ticker, mSourceId, true, requestID, jo.at("code").as_integer(), jo.at("msg").as_string().c_str());
		}
	}
	catch (const std::exception& e)
	{
		DCSS_LOG_ERROR(mLogger, "error during parse (exception)" << e.what());
	}
}

void BinaTGApi::OnRspQryOrder(http_response& response, const DCSSReqQryOrderField* req, int requestID)
{
	try
	{
		const json::value jv = response.extract_json().get();
		const json::object& jo = jv.as_object();
		DCSSOrderField order;
		strcpy(order.Symbol, req->Symbol);
		order.OrderID = req->OrderID;
		if (jo.find("code") != jo.end())
		{
			const std::string& side = jo.at("side").as_string();
			const std::string& type = jo.at("type").as_string();
			const std::string& status = jo.at("status").as_string();

			if (directionEnumMap.count(side) == 0 ||
					orderTypeEnumMap.count(type) == 0 ||
					statusEnumMap.count(status) == 0)
				return;

			order.Exchange = ExchangeEnum::Binance;
			order.Direction = directionEnumMap.at(side);
			order.Type = orderTypeEnumMap.at(type);
			order.OriginQuantity = std::stod(jo.at("origQty").as_string());
			order.ExecuteQuantity = std::stod(jo.at("executedQty").as_string());
			order.Price = std::stod(jo.at("price").as_string());
			order.UpdateTime = jo.at("updateTime").as_number().to_int64();
			order.Status = statusEnumMap.at(status);

			mSpi->OnRspQryOrder(&order, mSourceId, true, requestID);
		}
		else
			mSpi->OnRspQryOrder(&order, mSourceId, true, requestID, jo.at("code").as_integer(), jo.at("msg").as_string().c_str());
	}
	catch (const std::exception& e)
	{
		DCSS_LOG_ERROR(mLogger, "error during parse (exception)" << e.what());
	}
}

void BinaTGApi::OnRspQryOpenOrder(http_response& response, const DCSSReqQryOrderField* req, int requestID)
{
	try
	{
		DCSSOrderField order;
		order.Exchange = ExchangeEnum::Binance;
		strcpy(order.Symbol, req->Symbol);
		const json::value jv = response.extract_json().get();
		if (jv.is_object())
		{
			const json::object& jo = jv.as_object();
			if (jo.find("code") != jo.end())
				mSpi->OnRspQryOpenOrder(&order, mSourceId, true, requestID, jo.at("code").as_integer(), jo.at("msg").as_string().c_str());
		}
		else
		{
			const json::array& orders = jv.as_array();
			size_t total = orders.size();
			size_t index = 0;
			for (auto& item : orders)
			{
				++index;
				const json::object& orderItem = item.as_object();

				const std::string& side = orderItem.at("side").as_string();
				const std::string& type = orderItem.at("type").as_string();
				const std::string& status = orderItem.at("status").as_string();

				if (directionEnumMap.count(side) == 0 ||
						orderTypeEnumMap.count(type) == 0 ||
						statusEnumMap.count(status) == 0)
					continue;

				order.InsertTime = orderItem.at("time").as_number().to_int64();
				order.OrderID = orderItem.at("orderId").as_number().to_int64();
				order.Direction = directionEnumMap.at(side);
				order.Type = orderTypeEnumMap.at(type);
				order.OriginQuantity = std::stod(orderItem.at("origQty").as_string());
				order.ExecuteQuantity = std::stod(orderItem.at("executeQty").as_string());
				order.Price = std::stod(orderItem.at("price").as_string());
				order.UpdateTime = orderItem.at("updateTime").as_number().to_int64();
				order.Status = statusEnumMap.at(status);

				mSpi->OnRspQryOpenOrder(&order, mSourceId, index == total, requestID);
			}
		}
	}
	catch (const std::exception& e)
	{
		DCSS_LOG_ERROR(mLogger, "error during parse (exception)" << e.what());
	}
}

void BinaTGApi::OnRspOrderInsert(http_response& response, const DCSSReqInsertOrderField* req, int requestID)
{
    try
    {
        const json::value& jo = response.extract_json().get();

        if (!jo.is_object())
            return;

        DCSSRspInsertOrderField rsp;

        if (jo.has_integer_field("code"))
        {
            mSpi->OnRspOrderInsert(&rsp, mSourceId, true, requestID, jo.at("code").as_integer(), jo.at("msg").as_string().c_str());
        }
        else
        {
            rsp.OrderID = jo.at("orderId").as_number().to_int64();
            rsp.Result = jo.at("status").as_string() != "REJECTED";

            mSpi->OnRspOrderInsert(&rsp, mSourceId, true, requestID);
        }
    }
    catch (const std::exception& e)
    {
        DCSS_LOG_ERROR(mLogger, "error during parse (exception)" << e.what());
    }
}

void BinaTGApi::OnRspCancelOrder(http_response& response, const DCSSReqCancelOrderField* req, int requestID)
{
	try
	{
		const json::value& jo = response.extract_json().get();
		if (!jo.is_object())
			return;

		DCSSRspCancelOrderField rsp;
		rsp.OrderID = req->OrderID;
		if (jo.has_integer_field("code"))
		{
			mSpi->OnRspOrderAction(&rsp, mSourceId, true, requestID, jo.at("code").as_integer(), jo.at("msg").as_string().c_str());
		}
		else
		{
			mSpi->OnRspOrderAction(&rsp, mSourceId, true, requestID);
		}
	}
	catch (const std::exception& e)
	{
		DCSS_LOG_ERROR(mLogger, "error during parse (exception)" << e.what());
	}
}

void BinaTGApi::AddTimeStamp(uri_builder& builder)
{
	builder.append_query("timestamp", GetNanoTime() / NANOSECONDS_PER_MILLISECOND);
}

void BinaTGApi::HMAC_SHA256(uri_builder& builder)
{
    const std::string& str = builder.query();
    unsigned char mac[EVP_MAX_MD_SIZE] = {};
    unsigned int macLength = 0;
    const EVP_MD* engine = EVP_sha256();
    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);
    HMAC_Init_ex(&ctx, mSecretKey.c_str(), (int)mSecretKey.length(), engine, nullptr);
    HMAC_Update(&ctx, (unsigned char *)str.c_str(), str.length());

    HMAC_Final(&ctx, mac, &macLength);
    HMAC_CTX_cleanup(&ctx);

    char result[EVP_MAX_MD_SIZE] = {};

    for (int i = 0; i < macLength; ++i)
        sprintf(result + i * 2, "%02x", mac[i]);

    builder.append_query("signature", std::string(result));
}
