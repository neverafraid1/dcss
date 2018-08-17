//
// Created by wangzhen on 18-8-6.
//

#include <openssl/hmac.h>
#include <algorithm>

#include "BinaTGApi.h"
#include "Timer.h"
#include "Helper.h"
#include "SymbolDao.hpp"

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

BinaTGApi::BinaTGApi(uint8_t source)
: ITGApi(source), mWsConnected(false), mAccountLastUpdateTime(0), mLogined(false)
{
    auto res = SymbolDao::GetAllSymbol(source);
    for (const auto& item : res)
    {
        std::string common = std::string(std::get<1>(item)).append("_").append(std::get<2>(item));
        std::transform(common.begin(), common.end(), common.begin(), ::toupper);
        mBinaToCommonSymbolMap[std::get<0>(item)] = common;
        mCommonToBinaSymbolMap[common] = std::get<0>(item);
    }
}

BinaTGApi::~BinaTGApi()
{
    mWsClient->close(websocket_close_status::normal, "destruct");
    if (mPingThread)
    {
        mPingThread->join();
        mPingThread.reset();
    }
    mWsClient.reset();
    mRestClient.reset();
}

void BinaTGApi::LoadAccount(const nlohmann::json& config)
{
    mApiKey = config.at("api_key");
    mSecretKey = config.at("secret_key");
}

void BinaTGApi::Connect()
{
    if (mProxy.empty())
    {
        mWsClient.reset(new websocket_callback_client());
        mRestClient.reset(new http_client(U("https://api.binance.com")));
    }
    else
    {
        web_proxy proxy(U(mProxy));

        websocket_client_config ws_config;
        ws_config.set_proxy(proxy);
        mWsClient.reset(new websocket_callback_client(ws_config));

        http_client_config http_config;
        http_config.set_proxy(proxy);
        mRestClient.reset(new http_client(U("https://api.binance.com"), http_config));
    }

    mWsClient->set_message_handler(std::bind(&BinaTGApi::OnWsMessage, this, std::placeholders::_1));
    mWsClient->set_close_handler(std::bind(&BinaTGApi::OnWsClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    mWsClient->connect("wss://stream.binance.com:9443")
            .then([this]()
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
                    DCSS_LOG_ERROR(mLogger, "[binance][tg] connect to ws failed! (exception)" << e.what());
                }

            });
}

void BinaTGApi::Disconnect()
{
    if (mWsConnected)
    {
        mWsClient->close(websocket_close_status::normal, "disconnect by client");
        mWsConnected = false;
    }
}

void BinaTGApi::Login()
{
    mLogined = true;
}

bool BinaTGApi::IsConnected() const
{
    return mWsConnected;
}

bool BinaTGApi::IsLogged() const
{
    return mLogined;
}

void BinaTGApi::ReqQryUserInfo(int requestID)
{
    http_request request(methods::GET);
    request.headers().add("X-MBX-APIKEY", mApiKey);

    uri_builder builder("/api/v3/account");
    builder.append_query("timestamp", GetNanoTime() / NANOSECONDS_PER_MILLISECOND);
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
                            DCSS_LOG_ERROR(mLogger, "[bina tg][qry user info] "
                                                    "send request failed!(expection)" << e.what());
                        }
                    }
            );

}

void BinaTGApi::ReqQryKline(const DCSSReqQryKlineField* req, int requestID)
{
    if (mCommonToBinaSymbolMap.count(req->Symbol) == 0)
    {
        DCSS_LOG_ERROR(mLogger, "error symbol " << req->Symbol);
        return;
    }

    http_request request(methods::GET);
    uri_builder builder("/api/v1/klines");
    builder.append_query("symbol", mCommonToBinaSymbolMap.at(req->Symbol));
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
    http_request request(methods::POST);
    request.headers().add("X-MBX-APIKEY", mApiKey);
    uri_builder builder("/api/v3/order");
    builder.append_query("symbol", mCommonToBinaSymbolMap.at(req->Symbol));
    builder.append_query("side", enumDirectionMap.at(req->Direction));
    builder.append_query("type", enumOrderTypeMap.at(req->Type));
    builder.append_query("timeInForce", "GTC");
    builder.append_query("quantity", req->Amount);
    if (!IsEqual(req->Price, 0.0))
        builder.append_query("price", req->Price);

    builder.append_query("timestamp", GetNanoTime() / NANOSECONDS_PER_MILLISECOND);
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


/// rsp ///
void BinaTGApi::OnWsMessage(const websocket_incoming_message& msg)
{
    json::value jv = json::value::parse(msg.extract_string().get());
    const json::object& jo = jv.as_object();
    const std::string& event = jo.at("e").as_string();
    if (event == "outboundAccountInfo" == 0)
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
    mWsConnected = false;

    mPingThread->join();
    mPingThread.reset();

    mSpi->OnRtnTdStatus(TD_STATUS_DISCONNECTED, mSourceId);
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
    strcpy(orderField.Symbol, mBinaToCommonSymbolMap.at(symbol).c_str());
    SplitLongTime(jo.at("T").as_number().to_int64(), orderField.CreateDate, orderField.CreateTime, orderField.Millisec);
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
    DCSS_LOG_INFO(mLogger, "[binance][tg] ws connect success!");
    mWsConnected = true;

    mPingThread.reset(new std::thread(&BinaTGApi::Ping, this));
}

void BinaTGApi::Ping()
{
    http_request request(methods::POST);
    request.headers().add("X-MBX-APIKEY", mApiKey);
    uri_builder builder("/api/v1/userDataStream");
    request.set_request_uri(builder.to_uri());
    auto response = mRestClient->request(request).get();
    mListenKey = response.extract_json().get().as_object().at("listenKey").as_string();
    int loop(0);
    while (mWsConnected)
    {
        if (++loop >= 30 * 60)
        {
            uri_builder builder("/api/v1/userDataStream");
            builder.append_query("listenKey", mListenKey);
            mRestClient->request(methods::PUT, builder.to_string());
            loop = 0;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
void BinaTGApi::OnRspQryUserInfo(http_response& response, int requestID)
{
    try
    {
        const json::value jv = response.extract_json().get();
        const json::object& jobj = jv.as_object();

        if (jobj.find("code") == jobj.end())
        {
            DCSSTradingAccountField rsp;
            int index = 0;

            const json::array& jarray = jobj.at("balances").as_array();
            for(const auto& item : jarray)
            {
                const json::object& balance = item.as_object();
                double free = std::stod(balance.at("free").as_string());
                double locked = std::stod(balance.at("locked").as_string());
                if (IsEqual(free, 0.0) && IsEqual(locked, 0,0))
                    continue;

                const std::string& asset = balance.at("asset").as_string();
                memcpy(rsp.Balance[index].Currency, asset.c_str(), asset.length());
                rsp.Balance[index].Free = free;
                rsp.Balance[index].Freezed = locked;
             }

             mSpi->OnRspQryUserInfo(&rsp, mSourceId, requestID);
        }
        else
        {
            mSpi->OnRspQryUserInfo(nullptr, mSourceId, requestID, jobj.at("code").as_integer(), jobj.at("msg").as_string().c_str());
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
        if (jv.is_array())
        {
            const json::array& arr = jv.as_array();

            DCSSKlineHeaderField header;
            strcpy(header.Symbol, req->Symbol);
            header.Type = req->Type;
            header.Size = arr.size();

            std::vector<DCSSKlineField> klineVec(header.Size);
            int idx = 0;
            for (const auto& item : arr)
            {
                if (item.is_array())
                {
                    DCSSKlineField& kline = klineVec[idx++];
                    const json::array& klineArr = item.as_array();
                    SplitLongTime(klineArr.at(0).as_number().to_int64(), kline.Date, kline.Time, kline.Millisec);
                    kline.OpenPrice = std::stod(klineArr.at(1).as_string());
                    kline.Highest = std::stod(klineArr.at(2).as_string());
                    kline.Lowest = std::stod(klineArr.at(3).as_string());
                    kline.ClosePrice = std::stod(klineArr.at(4).as_string());
                    kline.Volume = std::stod(klineArr.at(5).as_string());
                }
            }

            mSpi->OnRspQryKline(&header, mSourceId, klineVec, requestID);
        }
        else if (jv.is_object())
        {
            const json::object& obj = jv.as_object();
            if (obj.find("code") != obj.end())
            {
                mSpi->OnRspQryKline(nullptr, mSourceId, std::vector<DCSSKlineField>(),
                        requestID, obj.at("code").as_integer(), obj.at("msg").as_string().c_str());
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

        if (jo.has_integer_field("code"))
        {
            mSpi->OnRspOrderInsert(nullptr, mSourceId, requestID, jo.at("code").as_integer(), jo.at("msg").as_string().c_str());
        }
        else
        {
            DCSSRspInsertOrderField rsp;
            rsp.OrderID = jo.at("orderId").as_number().to_int64();
            rsp.Result = jo.at("status").as_string() != "REJECTED";

            mSpi->OnRspOrderInsert(&rsp, mSourceId, requestID);
        }
    }
    catch (const std::exception& e)
    {
        DCSS_LOG_ERROR(mLogger, "error during parse (exception)" << e.what());
    }
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