//
// Created by wangzhen on 18-6-7.
//

#include "OKTGApi.h"
#include "OKAddress.h"
#include "Helper.h"
#include "MD5.h"

#include <algorithm>
#include <thread>

using namespace DCSS;

std::map<KlineTypeType, std::string> OKTGApi::klineStringMap = {
        {KLINE_1MIN, "1min"},
        {KLINE_3MIN, "3min"},
        {KLINE_5MIN, "5min"},
        {KLINE_15MIN, "15min"},
        {KLINE_30MIN, "30min"},
        {KLINE_1HOUR, "1hour"},
        {KLINE_2HOUR, "2hour"},
        {KLINE_4HOUR, "4hour"},
        {KLINE_6HOUR, "6hour"},
        {KLINE_12HOUR, "12hour"},
        {KLINE_1DAY, "1day"},
        {KLINE_3DAY, "3day"},
        {KLINE_1WEEK, "1week"}
};

std::map<std::string, KlineTypeType> OKTGApi::stringKlineMap = {
        {"1min", KLINE_1MIN},
        {"3min", KLINE_3MIN},
        {"5min", KLINE_5MIN},
        {"15min", KLINE_15MIN},
        {"30min", KLINE_30MIN},
        {"1hour", KLINE_1HOUR},
        {"2hour", KLINE_2HOUR},
        {"4hour", KLINE_4HOUR},
        {"6hour", KLINE_6HOUR},
        {"12hour", KLINE_12HOUR},
        {"1day", KLINE_1DAY},
        {"3day", KLINE_3DAY},
        {"1week", KLINE_1WEEK}
};

std::map<TradeTypeType, std::string> OKTGApi::tradeTypeStringMap = {
        {BUY, "buy"},
        {SELL, "sell"},
        {BUY_MARKET, "buy_market"},
        {SELL_MARKET, "sell_market"}
};

std::map<std::string, TradeTypeType> OKTGApi::stringTradeTypeMap = {
        {"buy", BUY},
        {"sell", SELL},
        {"buy_market", BUY_MARKET},
        {"sell_market", SELL_MARKET}
};



OKTGApi::OKTGApi(std::string apiKey, std::string secretKey)
        :mApiKey(apiKey), mSecretKey(secretKey), IsRestConnected(false), IsWsConnected(false)
{
    web_proxy proxy("http://192.168.1.164:1080");
    websocket_client_config config;
//    config.set_t
    config.set_proxy(proxy);

    http_client_config config1;
    config1.set_timeout(seconds(5));
    config1.set_proxy(proxy);

    mWsClient.reset(new websocket_callback_client(config));
    mRestClient.reset(new http_client(U(OK_REST_ROOT_URL), config1));

    mWsClient->set_message_handler(std::bind(&OKTGApi::OnWSMessage, this, std::placeholders::_1));
    mWsClient->set_close_handler(std::bind(&OKTGApi::OnWsClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

OKTGApi::~OKTGApi()
{
    mWsClient->close();
    mWsClient.reset();
    mRestClient.reset();
}

void OKTGApi::OnWsClose(web::websockets::client::websocket_close_status close_status,
        const utility::string_t& reason, const std::error_code& error)
{
    DCSS_LOG_INFO(logger, "[ok_tg][wsclose](" << mApiKey << ")(close_status)" << (int)close_status
    << "(reason)" << reason << "(error)" << error.message());

    IsWsConnected = false;
}

void OKTGApi::OnWSMessage(const web::websockets::client::websocket_incoming_message& msg)
{
    try
    {
        json::value jvalue = json::value::parse(msg.extract_string().get());
        std::cout << jvalue.serialize() << std::endl;
        const json::array& jarray = jvalue.as_array();
        for (auto i : jarray)
        {
            const json::object& jobj = i.as_object();
            std::string channel = jobj.at(U("channel")).as_string();
            const json::object& data = jobj.at(U("data")).as_object();
            if (channel.find("order") != std::string::npos)
            {
                OnRtnOrder(data);
                break;
            }
            if (channel.find("balance") != std::string::npos)
            {
                std::string ss = jvalue.serialize();
                OnRtnBalance(data);
                break;
            }
            if (channel.find("login") != std::string::npos)
            {
                OnUserLogin(data);
                break;
            }
        }
    }
    catch (const std::exception& e)
    {
        DCSS_LOG_ERROR(logger, "[ok_tg][onwsmessage]" << "(error)" << e.what());
    }

}

void OKTGApi::OnRtnOrder(const json::object& jorder)
{
    DCSSOrderField order;

    memcpy(order.Symbol, jorder.at(U("symbol")).as_string().c_str(),
            sizeof(jorder.at(U("symbol")).as_string().length()));
    SplitLongTime(std::stol(jorder.at(U("createdDate")).as_string()), order.CreateDate, order.CreateTime,
            order.Millisec);
    order.OrderID = jorder.at(U("orderId")).as_number().to_int64();
    order.TradeType = stringTradeTypeMap.at(jorder.at(U("tradeType")).as_string());
    if (jorder.find(U("sigTradeAmount"))!=jorder.end())
        order.SigTradeAmout = std::stod(jorder.at(U("sigTradeAmount")).as_string());
    if (jorder.find(U("sigTradePrice"))!=jorder.end())
        order.SigTradePrice = std::stod(jorder.at(U("sigTradePrice")).as_string());
    order.TradeAmount = std::stod(jorder.at(U("tradeAmount")).as_string());
    order.TradeUnitPrice = std::stod(jorder.at(U("tradeUnitPrice")).as_string());
    order.CompletedTradeAmount = std::stod(jorder.at(U("completedTradeAmount")).as_string());
    order.TradePrice = std::stod(jorder.at(U("tradePrice")).as_string());
    order.AveragePrice = std::stod(jorder.at(U("averagePrice")).as_string());
    if (jorder.find(U("unTrade"))!=jorder.end())
        order.UnTrade = std::stod(jorder.at(U("unTrade")).as_string());
    order.OrderStatus = (OrderStatusType) (jorder.at(U("Status")).as_integer());


    mSpi->OnRtnOrder(&order);
}

void OKTGApi::OnRtnBalance(const web::json::object& j)
{
}

void OKTGApi::OnUserLogin(const web::json::object& j)
{
    IsLoggined = j.at(U("result")).as_bool();
    if (IsLoggined)
    {
        DCSS_LOG_INFO(logger, "[ok_tg][login]" << "\t(" << mApiKey << ")"
        << " login success");
    }
    else
    {
        DCSS_LOG_INFO(logger, "[ok_tg][login]" << "\t(" << mApiKey << ")"
        << " login fail!");
        DCSS_LOG_INFO(logger, "[ok_tg][login]" << "(error_msg)" <<
        j.at(U("error_msg")).as_string());
    }
}

void OKTGApi::Connect()
{
    mWsClient->connect(U(OK_WS_ADDRESS))
            .then([this]()
            {
              OnWsConnected();
            })
            .then([&](pplx::task<void> task)
            {
              try
              {
                  task.get();
              }
              catch (const std::exception& e)
              {
                  DCSS_LOG_ERROR(logger, "[ok_tg][connect]\t(" << mApiKey << ")" << "ws send connect failed!(exception)" << e.what());
              }
            });
}

void OKTGApi::OnWsConnected()
{
    DCSS_LOG_INFO(logger, "[ok_tg][connect]\t(" << mApiKey << ")" << " ws connect success!");
    IsWsConnected = true;
}

void OKTGApi::Login()
{
    int i = 0;
    while (!IsWsConnected)
    {
        if (i++ >= 10)
        {
            DCSS_LOG_ERROR(logger, "[ok_tg][login]\t" << "(" << mApiKey << ")"
            << "connect to " << OK_WS_ADDRESS << " timeout!");
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    uri_builder builder;
    builder.append_query(U("api_key"), U(mApiKey));
    builder.append_query(U("secret_key"), U(mSecretKey));
    std::string sign = MD5::MD5String(builder.query().c_str());
    std::transform(sign.begin(), sign.end(), sign.begin(), ::toupper);

    json::value jv;
    jv[U(OK_WS_EVENT)] = json::value::string("login");
    jv[U("parameters")] = json::value::object(
            {{U("api_key"), json::value::string(mApiKey)},
                    {U("sign"), json::value::string(sign)}});

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());

    mWsClient->send(msg).then([&]()
    {
        DCSS_LOG_INFO(logger, "[ok_tg][login]\t" << "(" << mApiKey << ")" << " send request success!");
    }).then([&](pplx::task<void> task)
    {
        try
        {
            task.get();
        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(logger, "[ok_tg][login]\t" << "(" << mApiKey << ")"
                                                   << " send request failed!(exception)" << e.what() );
        }
    });

}

void OKTGApi::Register(ITGEnginePtr spi)
{
    mSpi = spi;
}

bool OKTGApi::IsLogged()
{
    return IsLoggined;
}

bool OKTGApi::IsConnected() const
{
    return IsWsConnected;
}

void OKTGApi::ReqQryTicker(const DCSSReqQryTickerField* req, int requestID)
{
    uri_builder builder(U(OK_REST_TICK_URL));
    builder.append_query(U(OK_SYMBOL_STRING), U(req->Symbol));
    mRestClient->request(methods::GET, builder.to_string())
            .then([&](http_response response)
            {
              OnRspQryTicker(response, requestID, req->Symbol);
            })
            .then([&](pplx::task<void> task)
            {
              try
              {
                  task.get();
              }
              catch (const std::exception& e)
              {
                  DCSS_LOG_ERROR(logger, "[ok_tg][qryticker]\t" << "(" << mApiKey << ")"
                                                         << " send request failed!(exception)" << e.what());
              }
            });
}


void OKTGApi::ReqQryKline(const DCSSReqQryKlineField* req, int requestID)
{
    if (klineStringMap.find(req->KlineType)==klineStringMap.end())
    {
        DCSS_LOG_ERROR(logger, "[ok_tg][qrykline]\t" << "(" << mApiKey << ")"
                                                  << " unknown kline type "<< req->KlineType);
        return;
    }
    uri_builder builder(U("kline.do"));
    builder.append_query(U("symbol"), U(req->Symbol));
    builder.append_query(U("type"), U(klineStringMap.at(req->KlineType)));
    if (req->Size!=0)
        builder.append_query(U("size"), req->Size);
    if (req->Since!=0)
        builder.append_query(U("since"), req->Since);

    mRestClient->request(methods::GET, builder.to_string())
            .then([&](http_response response)
            {
                OnRspQryKline(response, req, requestID);
            })
            .then([&](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_tg][qrykline]\t" << "(" << mApiKey << ")"
                                                              << " send request failed!(exception)" << e.what());
                }
            });
}

void OKTGApi::ReqQryUserInfo(int requestID)
{
    uri_builder builder(U("userinfo.do"));
    AddApiKey(builder);
    AddSecKeyAndSign(builder);

    mRestClient->request(methods::POST, builder.to_string())
            .then([&](http_response response)
            {
                OnRspQryUserInfo(response, requestID);
            })
            .then([&](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_tg][qryuserinfo]\t" << "(" << mApiKey << ")"
                                                              << " send request failed!(exception)" << e.what());
                }

            });
}

void OKTGApi::ReqQryOrder(const DCSSReqQryOrderField* req, int requestID)
{
    uri_builder builder(U("order_info.do"));

    AddApiKey(builder);
    builder.append_query(U("order_id"), req->OrderID);
    builder.append_query(U("symbol"), U(req->Symbol));
    AddSecKeyAndSign(builder);

    mRestClient->request(methods::POST, builder.to_string())
            .then([&](http_response response)
            {
                OnRspQryOrder(response, req, requestID);

            }).then([&](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_tg][qryorder]\t" << "(" << mApiKey << ")"
                                                              << " send request failed!(exception)" << e.what());
                }
            });
}

void OKTGApi::ReqInsertOrder(const DCSSReqInsertOrderField* req, int requestID)
{
    uri_builder builder(U("trade.do"));

    builder.append_query(U("amount"), req->Amount);
    AddApiKey(builder);
    if (req->TradeType == BUY || req->TradeType == SELL)
        builder.append_query(U("price"), req->Price);
    builder.append_query(U("symbol"), U(req->Symbol));
    builder.append_query(U("type"), U(tradeTypeStringMap.at(req->TradeType)));
    AddSecKeyAndSign(builder);

    mRestClient->request(methods::POST, builder.to_string())
            .then([&](http_response response) mutable
            {
                OnRspInsertOrder(response, requestID);
            }).then([=](pplx::task<void> task) mutable
            {

                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_tg][insertorder]\t" << "(" << mApiKey << ")"
                                                              << " send request failed!(exception)" << e.what());
                }
            });

}

void OKTGApi::ReqCancelOrder(const DCSSReqCancelOrderField* req, int requestID)
{
    uri_builder builder(U("cancel_order.do"));

    AddApiKey(builder);
    builder.append_query(U("symbol"), U(req->Symbol));

    std::stringstream ssorderid;

    for (int i = 0; i < 3; ++i)
    {
        if (0 == req->OrderID[i])
            break;

        ssorderid << req->OrderID[i];

        if (i < 2)
            ssorderid << ",";
    }
    builder.append_query(U("order_id"), U(ssorderid.str()));
    AddSecKeyAndSign(builder);

    mRestClient->request(methods::POST, builder.to_string())
            .then([&](http_response response)
            {
                OnRspCancelOrder(response, requestID);
            }).then([&](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_tg][cancelorder]\t" << "(" << mApiKey << ")"
                                                              << " send request failed!(exception)" << e.what());
                }

            });
}

void OKTGApi::OnRspQryTicker(http_response& response, int requestID, const char10& symbol)
{
    const json::value& jv = response.extract_json().get();
    const json::object& jobj = jv.as_object();
    if (jobj.find(U(OK_REST_ERROR_CODE))==jobj.end())
    {
        // success
        try
        {
            DCSSTickerField ticker;
            SplitTime(std::stol(jobj.at(U("date")).as_string()), ticker.Date, ticker.Time);
            memcpy(ticker.Symbol, symbol, sizeof(symbol));
            const json::object& tickobj = jobj.at(U("ticker")).as_object();
            ticker.BuyPrice = std::stod(tickobj.at(U("buy")).as_string());
            ticker.SellPrice = std::stod(tickobj.at(U("sell")).as_string());
            ticker.Highest = std::stod(tickobj.at(U("high")).as_string());
            ticker.Lowest = std::stod(tickobj.at(U("low")).as_string());
            ticker.LastPrice = std::stod(tickobj.at(U("last")).as_string());
            ticker.Volume = std::stod(tickobj.at(U("vol")).as_string());

            mSpi->OnRspQryTicker(&ticker, requestID);

            // writer
        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(logger, "[ok_tg][qryticker]\t" << "(" << mApiKey << ")"
                                                      << " parse rsp failed!(exception)" << e.what());
        }
    }
    else
    {
        mSpi->OnRspQryTicker(nullptr, requestID, jobj.at(U("error_code")).as_integer(), nullptr);
        DCSS_LOG_INFO(logger, "[ok_tg][qryticker]\t" << "(" << mApiKey << ")"
                                                    << " qry failed !(error_id)" << jobj.at(U("error_code")).as_integer());
    }

}

void OKTGApi::OnRspQryKline(http_response& response, const DCSSReqQryKlineField* req, int requestID)
{
    const json::value& jv = response.extract_json().get();

    if (jv.is_array())
    {
        try
        {
            const json::array& jarr = jv.as_array();

            DCSSKlineHeaderField header;
            memcpy(header.Symbol, req->Symbol, sizeof(req->Symbol));
            header.KlineType = req->KlineType;
            header.Size = jarr.size();

            std::vector<DCSSKlineField> klineVec(header.Size);
            int idx = 0;
            for (const auto& i : jarr)
            {
                if (i.is_array())
                {
                    DCSSKlineField& kline = klineVec[idx];
                    const json::array& klineArr = i.as_array();
                    SplitLongTime(klineArr.at(0).as_number().to_int64(), kline.Date, kline.Time, kline.Millisec);
                    kline.OpenPrice = std::stod(klineArr.at(1).as_string());
                    kline.Highest = std::stod(klineArr.at(2).as_string());
                    kline.Lowest = std::stod(klineArr.at(3).as_string());
                    kline.ClosePrice = std::stod(klineArr.at(4).as_string());
                    kline.Volume = std::stod(klineArr.at(5).as_string());
                    klineVec.push_back(kline);
                }
                ++idx;
            }

            mSpi->OnRspQryKline(&header, klineVec, true, requestID);
        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(logger, "[ok_tg][qrykline]\t" << "(" << mApiKey << ")"
                                                       << " parse rsp failed!(exception)" << e.what());
        }
    }
    else if (jv.is_object())
    {
        const json::object& jobj = jv.as_object();
        if (jobj.find(U(OK_REST_ERROR_CODE))!=jobj.end())
        {
            mSpi->OnRspQryKline(nullptr, std::vector<DCSSKlineField>(), requestID, jobj.at(U("error_code")).as_integer(),
                    nullptr);
            DCSS_LOG_INFO(logger, "[ok_tg][qrykline]\t" << "(" << mApiKey << ")"
                                                      << " qry failed !(error_id)" << jobj.at(U("error_code")).as_integer());
        }
    }
}

void OKTGApi::OnRspQryUserInfo(http_response& response, int requestID)
{
    const json::value& jv = response.extract_json().get();
    const json::object& jobj = jv.as_object();

    if (jobj.find(U("error_code")) == jobj.end())
    {
        try
        {
            bool result = jobj.at(U("result")).as_bool();
            if (result)
            {
                DCSSUserInfoField rsp;
                const json::object& info = jobj.at(U("info")).as_object();
                const json::object& fund = info.at(U("funds")).as_object();
                const json::object& free = fund.at(U("free")).as_object();
                const json::object& freezed = fund.at(U("freezed")).as_object();

                int index = 0;
                for (const auto& i : free)
                {
                    double balance = std::stod(i.second.as_string());
                    if (!IsEqual(balance, 0.0))
                    {
                        memcpy(rsp.Free[index].Currency, i.first.c_str(), i.first.length());
                        rsp.Free[index].Balance = balance;
                        ++index;
                    }
                }
                index = 0;
                for (const auto& i : freezed)
                {
                    double balance = std::stod(i.second.as_string());
                    if (!IsEqual(balance, 0.0))
                    {
                        memcpy(rsp.Freezed[index].Currency, i.first.c_str(), i.first.length());
                        rsp.Freezed[index].Balance = balance;
                        ++index;
                    }
                }

                mSpi->OnRspQryUserInfo(&rsp, requestID);
            }
            else
            {
                // TODO
                DCSS_LOG_INFO(logger, "[ok_tg][qryuserinfo]\t" << "(" << mApiKey << ")"
                << " result is false !");
            }

        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(logger, "[ok_tg][qryuserinfo]\t" << "(" << mApiKey << ")"
                                                         << " parse rsp failed!(exception)" << e.what());
        }

    }
    else
    {
        mSpi->OnRspQryUserInfo(nullptr, requestID, jobj.at(U("error_code")).as_integer(), nullptr);
        DCSS_LOG_INFO(logger, "[ok_tg][qryuserinfo]\t" << "(" << mApiKey << ")"
                << " qry failed !(error_id)" << jobj.at(U("error_code")).as_integer());
    }
}

void OKTGApi::OnRspInsertOrder(web::http::http_response& response, int requestID)
{
    DCSSRspInsertOrderField rsp;

    const json::value& jv = response.extract_json().get();
    const json::object& jobj = jv.as_object();

    if (jobj.find(U("error_code")) == jobj.end())
    {
        try
        {
            rsp.Result = jobj.at(U("result")).as_bool();
            rsp.OrderID = jobj.at(U("order_id")).as_number().to_int64();

            mSpi->OnRspOrderInsert(&rsp, true, requestID);
        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(logger, "[ok_tg][insertorder]\t" << "(" << mApiKey << ")"
                                                       << " parse rsp failed!(exception)" << e.what());
        }
    }
    else
    {
        mSpi->OnRspOrderInsert(nullptr, requestID, jobj.at(U("error_code")).as_integer());
        DCSS_LOG_INFO(logger, "[ok_tg][insertorder]\t" << "(" << mApiKey << ")"
        << " receive error rsp (error_code)" << jobj.at(U("error_code")).as_integer());
    }
}

void OKTGApi::OnRspCancelOrder(web::http::http_response& response, int requestID)
{
    const json::value& jv = response.extract_json().get();
    const json::object& jobj = jv.as_object();

    if (jobj.find(U("error_code")) == jobj.end())
    {
        try
        {
            DCSSRspCancelOrderField rsp;
            if (jobj.find(U("result"))!=jobj.end())
            {
                bool result = jobj.at(U("result")).as_bool();
                if (result)
                    rsp.SuccessID[0] = jobj.at(U("order_id")).as_number().to_int64();
                else
                    rsp.ErrorID[0] = jobj.at(U("order_id")).as_number().to_int64();
            }
            else
            {
                if (jobj.find(U("success"))!=jobj.end())
                {
                    std::stringstream ss(jobj.at(U("success")).as_string());
                    std::string tmp;
                    int i = 0;
                    while (getline(ss, tmp, ','))
                    {
                        rsp.SuccessID[i++] = std::stol(tmp);
                    }
                }
                if (jobj.find(U("error"))!=jobj.end())
                {
                    std::stringstream ss(jobj.at(U("error")).as_string());
                    std::string tmp;
                    int i = 0;
                    while (getline(ss, tmp, ','))
                    {
                        rsp.ErrorID[i++] = std::stol(tmp);
                    }
                }
            }
            mSpi->OnRspOrderAction(&rsp, requestID);
        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(logger, "[ok_tg][cancelorder]\t" << "(" << mApiKey << ")"
                                                       << " parse rsp failed!(exception)" << e.what());
        }
    }
    else
    {
        mSpi->OnRspOrderAction(nullptr, requestID, jobj.at(U("error_code")).as_integer(), nullptr);
    }
}

void OKTGApi::OnRspQryOrder(web::http::http_response& response, const DCSSReqQryOrderField* req, int requestID)
{
    const json::value& jv = response.extract_json().get();
    const json::object& jobj = jv.as_object();

    if (jobj.find(U("error_code")) == jobj.end())
    {
        try
        {
            if (jobj.at(U("result")).as_bool())
            {
                const json::array& orderArray = jobj.at(U("orders")).as_array();

                DCSSRspQryOrderHeaderField header;
                memcpy(header.Symbol, req->Symbol, sizeof(header.Symbol));
                header.Size = orderArray.size();

                std::vector<DCSSRspQryOrderField> vec(header.Size);
                int idx = 0;
                for (const auto& i : orderArray)
                {
                    DCSSRspQryOrderField& order = vec[idx++];

                    const json::object& orderObj = i.as_object();
                    order.Amount = orderObj.at(U("amount")).as_double();
                    SplitLongTime(orderObj.at(U("create_date")).as_number().to_int64(), order.CreateDate,
                            order.CreateTime, order.MilliSec);
                    order.AvgPrice = orderObj.at(U("avg_price")).as_double();
                    order.DealAmount = orderObj.at(U("deal_amount")).as_double();
                    order.OrderID = orderObj.at(U("order_id")).as_number().to_int64();
                    order.OrdersID = orderObj.at(U("orders_id")).as_number().to_int64();
                    order.Price = orderObj.at(U("price")).as_double();
                    order.OrderStatus = (OrderStatusType) (orderObj.at(U("Status")).as_integer());
                    order.TradeType = stringTradeTypeMap.at(orderObj.at(U("type")).as_string());
                }

                mSpi->OnRspQryOrder(&header, vec, requestID);
            }
        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(logger, "[ok_tg][qryorder]\t" << "(" << mApiKey << ")"
                                                       << " parse rsp failed!(exception)" << e.what());
        }
    }
    else
    {
        mSpi->OnRspQryOrder(nullptr, std::vector<DCSSRspQryOrderField>(), requestID, jobj.at(U("error_code")).as_integer(), nullptr);
    }
}

void OKTGApi::AddApiKey(web::uri_builder& builder)
{
    builder.append_query(U("api_key"), U(mApiKey));
}

void OKTGApi::AddSecKeyAndSign(web::uri_builder& builder)
{
    builder.append_query(U("secret_key"), U(mSecretKey));
    std::string sign = MD5::MD5String(builder.query().c_str());
    std::transform(sign.begin(), sign.end(), sign.begin(), ::toupper);
    builder.append_query(U("sign"), U(sign));
}