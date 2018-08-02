//
// Created by wangzhen on 18-6-7.
//

#include "OKTGApi.h"
#include "OKAddress.h"
#include "Helper.h"
#include "MD5.h"

#include <algorithm>
#include <thread>

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

OKTGApi::OKTGApi(uint8_t source)
        :ITGApi(source), IsRestConnected(false), IsWsConnected(false), IsPonged(true)
{

}

OKTGApi::~OKTGApi()
{
    mWsClient->close();
    mWsClient.reset();
    mRestClient.reset();
    mPingThread->join();
    mPingThread.reset();
}

void OKTGApi::OnWsClose(web::websockets::client::websocket_close_status close_status,
        const utility::string_t& reason, const std::error_code& error)
{
    DCSS_LOG_INFO(mLogger, "[ok_tg][wsclose](" << mApiKey << ")(close_status)" << (int)close_status
    << "(reason)" << reason << "(error)" << error.message());

    IsWsConnected = false;

    mPingThread->join();
    mPingThread.reset();

    mSpi->OnRtnTdStatus(TD_STATUS_DISCONNECTED, mSourceId);
}

void OKTGApi::ResetRestClient()
{
    web_proxy proxy("http://192.168.1.164:1080");
    http_client_config http_config;
    http_config.set_proxy(proxy);

    mRestClient.reset(new http_client(U(OK_REST_ROOT_URL), http_config));
}

void OKTGApi::OnWSMessage(const web::websockets::client::websocket_incoming_message& msg)
{
    std::string s = msg.extract_string().get();
    std::cout << s << std::endl;
    try
    {
        json::value jvalue = json::value::parse(s);
        if (jvalue.is_object())
        {
            std::string event = jvalue.as_object().at(U("event")).as_string();
            if (event == "pong")
                IsPonged = true;
        }
        else if (jvalue.is_array())
        {
            const json::array& jarray = jvalue.as_array();
            for (auto& i : jarray)
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
    }
    catch (const std::exception& e)
    {
        DCSS_LOG_ERROR(mLogger, "[ok_tg][onwsmessage]" << "(error)" << e.what());
    }

}

void OKTGApi::OnRtnOrder(const json::object& jorder)
{
    DCSSOrderField order;

    strcpy(order.Symbol, jorder.at(U("symbol")).as_string().c_str());
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
    order.OrderStatus = (OrderStatusType) (jorder.at(U("status")).as_integer());


    mSpi->OnRtnOrder(&order, mSourceId);
}

void OKTGApi::OnRtnBalance(const web::json::object& j)
{
    DCSSBalanceField balance;
    const json::object& info = j.at(U("info")).as_object();
    const json::object& free = info.at(U("free")).as_object();
    const json::object& freezed = info.at(U("freezed")).as_object();
    memcpy(balance.Currency, free.begin()->first.c_str(), free.begin()->first.length());
    balance.Free = free.begin()->second.as_double();
    balance.Freezed = freezed.begin()->second.as_double();

    mSpi->OnRtnBalance(&balance, mSourceId);
}

void OKTGApi::OnUserLogin(const web::json::object& j)
{
    IsLoggined = j.at(U("result")).as_bool();
    if (IsLoggined)
    {
        DCSS_LOG_INFO(mLogger, "(api key)" << mApiKey << " login success");
        mSpi->OnRtnTdStatus(TD_STATUS_LOGINED, mSourceId);
    }
    else
    {
        DCSS_LOG_INFO(mLogger, "(api key)" << mApiKey << " login fail!" << "(error_msg)" <<
        j.at(U("error_msg")).as_string());
    }
}

void OKTGApi::LoadAccount(const nlohmann::json& config)
{
    mApiKey = config.at("api_key");
    mSecretKey = config.at("secret_key");
}

void OKTGApi::Connect()
{
    web_proxy proxy("http://192.168.1.164:1080");
//    http_client_config http_config;
//    http_config.set_proxy(proxy);
//
//    mRestClient.reset(new http_client(U(OK_REST_ROOT_URL), http_config));

    websocket_client_config ws_config;
    ws_config.set_proxy(proxy);

    mWsClient.reset(new websocket_callback_client(ws_config));
    mWsClient->set_message_handler(std::bind(&OKTGApi::OnWSMessage, this, std::placeholders::_1));
    mWsClient->set_close_handler(std::bind(&OKTGApi::OnWsClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    mWsClient->connect(U(OK_WS_ADDRESS))
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
                    DCSS_LOG_ERROR(mLogger,
                            "(api key)" << mApiKey << "ws send connect failed!(exception)"
                                                  << e.what());
                }
            });
}

void OKTGApi::Disconnect()
{
    mWsClient->close(websocket_close_status::normal, "disconnect by client");
}

void OKTGApi::OnWsConnected()
{
    DCSS_LOG_INFO(mLogger, "(api key)" << mApiKey << " ws connect success!");
    IsWsConnected = true;

    mSpi->OnRtnTdStatus(TD_STATUS_CONNECTED, mSourceId);

    mPingThread.reset(new std::thread(&OKTGApi::Ping, this));

    Login();
}

void OKTGApi::Ping()
{
    /* every loop is 500ms, otherwise join will block too long*/
    int i = 0;
    while (true)
    {
        if (!IsPonged && i > 60)
        {
            DCSS_LOG_ERROR(mLogger, "[Ping] don't recv pong in the last 30 seconds");
            IsWsConnected = false;
        }
        else
        {
            IsPonged = false;
            json::value jv;
            jv[U("event")] = json::value::string("ping");
            websocket_outgoing_message msg;
            msg.set_utf8_message(jv.serialize());
            if (IsConnected() && i++ >= 60)
            {
                i = 0;
                mWsClient->send(msg);
                DCSS_LOG_DEBUG(mLogger, "[Ping] send ping to remote");
            }
            else
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void OKTGApi::Login()
{
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

    mWsClient->send(msg).then([this]()
    {
        DCSS_LOG_INFO(mLogger, "(api key)" << mApiKey << " send request success!");
    }).then([=](pplx::task<void> task)
    {
        try
        {
            task.get();
        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(mLogger, "(api key)" << mApiKey
                                                   << " send request failed!(exception)" << e.what() );
        }
    });

}

void OKTGApi::Register(ITGEnginePtr spi)
{
    if (spi.get() == nullptr)
        return;

    mSpi = spi;
    mLogger = mSpi->GetLogger();
}

bool OKTGApi::IsLogged() const
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
    builder.append_query(U(OK_SYMBOL_STRING), U(std::string(req->Symbol)));
    mRestClient->request(methods::GET, builder.to_string())
            .then([=](http_response response)
            {
              OnRspQryTicker(response, requestID, req->Symbol);
            })
            .then([=](pplx::task<void> task)
            {
              try
              {
                  task.get();
              }
              catch (const std::exception& e)
              {
                  DCSS_LOG_ERROR(mLogger, "[ok_tg][qryticker]\t" << "(" << mApiKey << ")"
                                                         << " send request failed!(exception)" << e.what());
              }
            });
}

void OKTGApi::ReqQryKline(const DCSSReqQryKlineField* req, int requestID)
{
    if (klineStringMap.find(req->KlineType)==klineStringMap.end())
    {
        DCSS_LOG_ERROR(mLogger, "[ok_tg][qrykline]\t" << "(" << mApiKey << ")"
                                                  << " unknown kline type "<< req->KlineType);
        return;
    }

    ResetRestClient();

    uri_builder builder(U("kline.do"));
    builder.append_query(U("symbol"), U(std::string(req->Symbol)));
    builder.append_query(U("type"), U(klineStringMap.at(req->KlineType)));
    if (req->Size!=0)
        builder.append_query(U("size"), req->Size);
    if (req->Since!=0)
        builder.append_query(U("since"), req->Since);

    mRestClient->request(methods::GET, builder.to_string())
            .then([=](http_response response)
            {
                OnRspQryKline(response, req, requestID);
            })
            .then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[ok_tg][qrykline]\t" << "(" << mApiKey << ")"
                                                              << " send request failed!(exception)" << e.what());
                }
            });
}

void OKTGApi::ReqQryUserInfo(int requestID)
{
    ResetRestClient();

    uri_builder builder(U("userinfo.do"));
    AddApiKey(builder);
    AddSecKeyAndSign(builder);

    mRestClient->request(methods::POST, builder.to_string())
            .then([=](http_response response)
            {
                OnRspQryUserInfo(response, requestID);
            })
            .then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[ok_tg][qryuserinfo] " << "(" << mApiKey << ")"
                                                              << " send request failed!(exception)" << e.what());
                }

            });
}

void OKTGApi::ReqQryOrder(const DCSSReqQryOrderField* req, int requestID)
{
    uri_builder builder(U("order_info.do"));

    AddApiKey(builder);
    builder.append_query(U("order_id"), req->OrderID);
    builder.append_query(U("symbol"), U(std::string(req->Symbol)));
    AddSecKeyAndSign(builder);

    mRestClient->request(methods::POST, builder.to_string())
            .then([=](http_response response)
            {
                OnRspQryOrder(response, req, requestID);

            }).then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[ok_tg][qryorder]\t" << "(" << mApiKey << ")"
                                                              << " send request failed!(exception)" << e.what());
                }
            });
}

void OKTGApi::ReqInsertOrder(const DCSSReqInsertOrderField* req, int requestID)
{
    ResetRestClient();

    uri_builder builder(U("trade.do"));
    if (req->TradeType != BUY_MARKET)
    {
        builder.append_query(U("amount"), req->Amount);
    }
    AddApiKey(builder);
    if (req->TradeType != SELL_MARKET)
    {
        builder.append_query(U("price"), req->Price);
    }

    builder.append_query(U("symbol"), U(std::string(req->Symbol)));
    builder.append_query(U("type"), U(tradeTypeStringMap.at(req->TradeType)));
    AddSecKeyAndSign(builder);

    mRestClient->request(methods::POST, builder.to_string())
            .then([=](http_response response) mutable
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
                    DCSS_LOG_ERROR(mLogger, "[ok_tg][insertorder]\t" << "(" << mApiKey << ")"
                                                              << " send request failed!(exception)" << e.what());
                }
            });

}

void OKTGApi::ReqCancelOrder(const DCSSReqCancelOrderField* req, int requestID)
{
    uri_builder builder(U("cancel_order.do"));

    AddApiKey(builder);
    builder.append_query(U("symbol"), U(std::string(req->Symbol)));

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
            .then([=](http_response response)
            {
                OnRspCancelOrder(response, requestID);
            }).then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[ok_tg][cancelorder]\t" << "(" << mApiKey << ")"
                                                              << " send request failed!(exception)" << e.what());
                }

            });
}

void OKTGApi::OnRspQryTicker(http_response& response, int requestID, const char21& symbol)
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
            strcpy(ticker.Symbol, symbol);
            const json::object& tickobj = jobj.at(U("ticker")).as_object();
            ticker.BuyPrice = std::stod(tickobj.at(U("buy")).as_string());
            ticker.SellPrice = std::stod(tickobj.at(U("sell")).as_string());
            ticker.Highest = std::stod(tickobj.at(U("high")).as_string());
            ticker.Lowest = std::stod(tickobj.at(U("low")).as_string());
            ticker.LastPrice = std::stod(tickobj.at(U("last")).as_string());
            ticker.Volume = std::stod(tickobj.at(U("vol")).as_string());

            mSpi->OnRspQryTicker(&ticker, mSourceId, requestID);
        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(mLogger, "(api key)" << mApiKey << " parse rsp failed!(exception)" << e.what());
        }
    }
    else
    {
        mSpi->OnRspQryTicker(nullptr, mSourceId, requestID, jobj.at(U("error_code")).as_integer(), nullptr);
        DCSS_LOG_INFO(mLogger, "(api key)" << mApiKey << " qry failed !(error_id)" << jobj.at(U("error_code")).as_integer());
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
            strcpy(header.Symbol, req->Symbol);
            header.KlineType = req->KlineType;
            header.Size = jarr.size();

            std::vector<DCSSKlineField> klineVec(header.Size);
            int idx = 0;
            for (const auto& i : jarr)
            {
                if (i.is_array())
                {
                    DCSSKlineField& kline = klineVec[idx++];
                    const json::array& klineArr = i.as_array();
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
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(mLogger, "(api key)" << mApiKey << " parse rsp failed!(exception)" << e.what());
        }
    }
    else if (jv.is_object())
    {
        const json::object& jobj = jv.as_object();
        if (jobj.find(U(OK_REST_ERROR_CODE))!=jobj.end())
        {
            mSpi->OnRspQryKline(nullptr, mSourceId, std::vector<DCSSKlineField>(), requestID, jobj.at(U("error_code")).as_integer(),
                    nullptr);
            DCSS_LOG_INFO(mLogger, "(api key)" << mApiKey << " qry failed !(error_id)" << jobj.at(U("error_code")).as_integer());
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
                DCSSTradingAccountField rsp;
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
                        memcpy(rsp.Balance[index].Currency, i.first.c_str(), i.first.length());
                        rsp.Balance[index].Free = balance;
                        ++index;
                    }
                }
                for (const auto& i : freezed)
                {
                    double balance = std::stod(i.second.as_string());
                    if (!IsEqual(balance, 0.0))
                    {
                        const std::string& currency = i.first;
                        bool found = false;
                        for (index = 0; index < MAX_CURRENCY_NUM && strlen(rsp.Balance[index].Currency) != 0; ++index)
                        {
                            if (std::string(rsp.Balance[index].Currency) == currency)
                            {
                                rsp.Balance[index].Freezed = balance;
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                        {
                            memcpy(rsp.Balance[index].Currency, currency.c_str(), currency.length());
                            rsp.Balance[index].Freezed = balance;
                        }
                    }
                }

                mSpi->OnRspQryUserInfo(&rsp, mSourceId, requestID);
            }
            else
            {
                // TODO
                DCSS_LOG_INFO(mLogger, "[ok_tg][qryuserinfo] " << "(" << mApiKey << ")"
                << " result is false !");
            }

        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(mLogger, "[ok_tg][qryuserinfo] " << "(" << mApiKey << ")"
                                                         << " parse rsp failed!(exception)" << e.what());
        }

    }
    else
    {
        mSpi->OnRspQryUserInfo(nullptr, mSourceId, requestID, jobj.at(U("error_code")).as_integer(), nullptr);
        DCSS_LOG_INFO(mLogger, "[ok_tg][qryuserinfo] " << "(" << mApiKey << ")"
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

            mSpi->OnRspOrderInsert(&rsp, mSourceId, requestID);
        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(mLogger, "[ok_tg][insertorder]\t" << "(" << mApiKey << ")"
                                                       << " parse rsp failed!(exception)" << e.what());
        }
    }
    else
    {
        mSpi->OnRspOrderInsert(nullptr, mSourceId, requestID, jobj.at(U("error_code")).as_integer());
        DCSS_LOG_INFO(mLogger, "[ok_tg][insertorder]\t" << "(" << mApiKey << ")"
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
            mSpi->OnRspOrderAction(&rsp, mSourceId, requestID);
        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(mLogger, "[ok_tg][cancelorder]\t" << "(" << mApiKey << ")"
                                                       << " parse rsp failed!(exception)" << e.what());
        }
    }
    else
    {
        mSpi->OnRspOrderAction(nullptr, mSourceId, requestID, jobj.at(U("error_code")).as_integer(), nullptr);
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
                memcpy(&header.Symbol, &req->Symbol, sizeof(header.Symbol));
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

                mSpi->OnRspQryOrder(&header, mSourceId, vec, requestID);
            }
        }
        catch (const std::exception& e)
        {
            DCSS_LOG_ERROR(mLogger, "[ok_tg][qryorder]\t" << "(" << mApiKey << ")"
                                                       << " parse rsp failed!(exception)" << e.what());
        }
    }
    else
    {
        mSpi->OnRspQryOrder(nullptr, mSourceId, std::vector<DCSSRspQryOrderField>(), requestID, jobj.at(U("error_code")).as_integer(), nullptr);
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