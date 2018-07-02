//
// Created by wangzhen on 18-6-15.
//

#include <Helper.h>
#include "OKMGApi.h"


std::map<KlineTypeType, std::string> OKMGApi::klineStringMap = {
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

std::map<std::string, KlineTypeType> OKMGApi::stringKlineMap = {
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

OKMGApi::OKMGApi()
{
    web_proxy proxy("http://192.168.1.164:1080");
    websocket_client_config config;
    config.set_proxy(proxy);

    mWsClient.reset(new websocket_callback_client(config));

    mWsClient->set_message_handler(std::bind(&OKMGApi::OnWsMessage, this, std::placeholders::_1));
    mWsClient->set_close_handler(std::bind(&OKMGApi::OnWsClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

OKMGApi::~OKMGApi()
{
    mWsClient->close();
    mWsClient.reset();
}

void OKMGApi::Register(IMGApiPtr spi)
{
    mSpi = spi;
}

void OKMGApi::Connect()
{
    mWsClient->connect(U("wss://real.okex.com:10441/websocket"))
            .then([this]()
            {
                OnWsConnect();
            })
            .then([&](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_mg][connect]\tsend connect failed!(exception)" << e.what());
                }
            });
}

void OKMGApi::OnWsMessage(const web::websockets::client::websocket_incoming_message& msg)
{
    std::string sss = msg.extract_string().get();
    std::cout << sss << std::endl;
    json::value jv = json::value::parse(sss);
    const json::array& ja = jv.as_array();

    for (auto i : ja)
    {
        const json::object& jo = i.as_object();
        const std::string& channel = jo.at(U("channel")).as_string();
        const json::value& data = jo.at(U("data"));
        if (mSubDepthMap.count(channel) > 0)
        {
            OnRtnDepth(data, mSubDepthMap.at(channel));
            break;
        }
        if (mSubTickMap.count(channel) > 0)
        {
            OnRtnTick(data, mSubTickMap.at(channel));
            break;
        }
        if (mSubKlineMap.count(channel) > 0)
        {
            OnRtnKline(data, mSubKlineMap.at(channel));
            break;
        }

        if (channel == "addChannel")
        {
            bool result = data.at(U("result")).as_bool();
            if (!result)
            {
                DCSS_LOG_INFO(logger, "[ok_mg][sub]\t" << channel << " fail");
                if (channel.find("ticker") != std::string::npos)
                    mSubTickMap.erase(channel);
                else if (channel.find("kline") != std::string::npos)
                    mSubKlineMap.erase(channel);
                else
                    mSubDepthMap.erase(channel);
            }
            break;
        }

        if (channel == "removeChannel")
        {
            bool result = data.at(U("result")).as_bool();
            break;
        }
    }
}

void OKMGApi::OnWsClose(web::websockets::client::websocket_close_status close_status,
        const utility::string_t& reason, const std::error_code& error)
{
    DCSS_LOG_INFO(logger, "[ok_mg][wsclose](close_status)" << (int)close_status
                                              << "(reason)" << reason << "(error)" << error.message());

    IsWsConnected = false;
}

void OKMGApi::OnWsConnect()
{
    DCSS_LOG_INFO(logger, "[ok_mg][connect]\tconnect to okes success!")
    IsWsConnected = true;
}

void OKMGApi::OnRtnTick(const json::value& v, const std::string& symbol)
{
    const json::object& j = v.as_object();
    DCSSTickerField ticker;

    SplitLongTime(j.at(U("timestamp")).as_number().to_int64(), ticker.Date, ticker.Time, ticker.MilliSec);
    memcpy(ticker.Symbol, symbol.c_str(), sizeof(symbol.length()));
    ticker.BuyPrice = std::stod(j.at(U("buy")).as_string());
    ticker.SellPrice = std::stod(j.at(U("sell")).as_string());
    ticker.Highest = std::stod(j.at(U("high")).as_string());
    ticker.Lowest = std::stod(j.at(U("low")).as_string());
    ticker.LastPrice = std::stod(j.at(U("last")).as_string());
    ticker.Volume = std::stod(j.at(U("vol")).as_string());

    mSpi->OnRtnTicker(&ticker);
}

void OKMGApi::OnRtnKline(const web::json::value& v, const std::pair<std::string, KlineTypeType>& pair)
{
    const json::array& arr = v.as_array();
    int size = arr.size();

    DCSSKlineHeaderField header;
    memcpy(header.Symbol, pair.first.c_str(), pair.first.length());
    header.KlineType = pair.second;
    header.Size = size;

    std::vector<DCSSKlineField> klineVec(size);

    for (int i = 0; i < size; ++i)
    {
        DCSSKlineField& kline = klineVec[i];
        const json::array& klineArr = arr.at(i).as_array();
        SplitLongTime(std::stol(klineArr.at(0).as_string()), kline.Date, kline.Time, kline.Millisec);
        kline.OpenPrice = std::stod(klineArr.at(1).as_string());
        kline.Highest = std::stod(klineArr.at(2).as_string());
        kline.Lowest = std::stod(klineArr.at(3).as_string());
        kline.ClosePrice = std::stod(klineArr.at(4).as_string());
        kline.Volume = std::stod(klineArr.at(5).as_string());
    }

    mSpi->OnRtnKline(&header, klineVec);
}

void OKMGApi::OnRtnDepth(const web::json::value& v, const std::pair<std::string, int>& pair)
{
    const json::array& ask = v.as_object().at(U("asks")).as_array();
    const json::array& bid = v.as_object().at(U("bids")).as_array();

    DCSSDepthHeaderField header;
    memcpy(header.Symbol, pair.first.c_str(), pair.first.length());
    SplitLongTime(std::stol(v.at(U("timestamp")).as_string()), header.Date, header.Time, header.Millisec));
    header.AskNum = ask.size();
    header.BidNum = bid.size();

    int length = header.AskNum + header.BidNum;

    std::vector<DCSSDepthField> depthVec(length);

    for (int i = 0; i < header.AskNum; ++i)
    {
        const json::array& arr = ask.at(i).as_array();
        depthVec[i].Price = std::stod(arr.at(0).as_string());
        depthVec[i].Volume = std::stod(arr.at(1).as_string());
    }
    for (int i = header.AskNum; i < length; ++i)
    {
        const json::array& arr = bid.at(i).as_array();
        depthVec[i].Price = std::stod(arr.at(0).as_string());
        depthVec[i].Volume = std::stod(arr.at(1).as_string());
    }

    mSpi->OnRtnDepth(&header, depthVec);
}

bool OKMGApi::IsConnected()
{
    return IsWsConnected;
}

void OKMGApi::ReqSubTicker(const char10& Symbol)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(logger, "[ok_mg][sub_tick]\tnot connected to exchange now!");
        return;
    }

    std::stringstream ss;
    ss << "ok_sub_spot_" << Symbol << "_ticker";
    json::value jv;
    jv[U("event")] = json::value::string("addChannel");
    jv[U("channel")] = json::value::string(ss.str());

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([&]()
            {
                DCSS_LOG_INFO(logger, "[ok_mg][sub_tick]\tsend sub "
                                      << Symbol << " tick request success");
                mSubTickMap[ss.str()] = Symbol;
            }).then([&](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_mg][sub_tick]\tsend sub "
                    << Symbol << " tick request fail!(exception)" << e.what());
                }
            });
}

void OKMGApi::ReqSubDepth(const char10& Symbol, int depth)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(logger, "[ok_mg][sub_depth]\tnot connected to exchange now!");
        return;
    }

    std::stringstream ss;
    ss << "ok_sub_spot_" << Symbol << "_depth";
    if (depth > 0)
        ss << "_" << depth;

    json::value jv;
    jv[U("event")] = json::value::string("addChannel");
    jv[U("channel")] = json::value::string(ss.str());

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([&]()
            {
                DCSS_LOG_INFO(logger, "[ok_mg][sub_depth]\tsend sub "
                        << Symbol << " tick request success");

                mSubDepthMap[ss.str()] = std::make_pair(Symbol, depth);
            }).then([&](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_mg][sub_depth]\tsend sub "
                            << Symbol << " tick request fail!(exception)" << e.what());
                }
            });
}

void OKMGApi::ReqSubKline(const char* Symbol, KlineTypeType klineType)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(logger, "[ok_mg][sub_kline]\tnot connected to exchange now!");
        return;
    }

    if (klineStringMap.count(klineType) == 0)
    {
        DCSS_LOG_INFO(logger, "[ok_mg][sub_kline]\tdon't support this kline type!");
        return;
    }

    std::stringstream ss;
    ss << "ok_sub_spot_" << Symbol << "_kline_" << klineStringMap.at(klineType);

    json::value jv;
    jv[U("event")] = json::value::string("addChannel");
    jv[U("channel")] = json::value::string(ss.str());

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([&]()
            {
                DCSS_LOG_INFO(logger, "[ok_mg][sub_kline]\tsend sub "
                        << Symbol << " tick request success");

                mSubKlineMap[ss.str()] = std::make_pair(Symbol, klineType);
            }).then([&](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_mg][sub_kline]\tsend sub "
                            << Symbol << " tick request fail!(exception)" << e.what());
                }
            });
}

void OKMGApi::ReqUnSubTicker(const char10& Symbol)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(logger, "[ok_mg][unsub_tick]\tnot connected to exchange now!");
        return;
    }

    std::stringstream ss;
    ss << "ok_sub_spot_" << Symbol << "_ticker";

    if (mSubTickMap.count(ss.str()) == 0)
    {
        DCSS_LOG_INFO(logger, "[ok_mg][unsub_tick]\t" << Symbol << " tick is not sub");
        return;
    }

    json::value jv;
    jv[U("event")] = json::value::string("removeChannel");
    jv[U("channel")] = json::value::string(ss.str());

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([&]()
            {
                DCSS_LOG_INFO(logger, "[ok_mg][unsub_tick]\tsend unsub "
                        << Symbol << " tick request success");
            }).then([&](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_mg][unsub_tick]\tsend unsub "
                            << Symbol << " tick request fail!(exception)" << e.what());
                }
            });
}

void OKMGApi::ReqUnSubDepth(const char10& Symbol, int depth)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(logger, "[ok_mg][unsub_depth]\tnot connected to exchange now!");
        return;
    }

    std::stringstream ss;
    ss << "ok_sub_spot_" << Symbol << "_depth";
    if (depth > 0)
        ss << "_" << depth;

    json::value jv;
    jv[U("event")] = json::value::string("removeChannel");
    jv[U("channel")] = json::value::string(ss.str());

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([&]()
            {
                DCSS_LOG_INFO(logger, "[ok_mg][unsub_depth]\tsend unsub "
                        << Symbol << " tick request success");

                mSubDepthMap[ss.str()] = std::make_pair(Symbol, depth);
            }).then([&](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_mg][unsub_depth]\tsend unsub "
                            << Symbol << " tick request fail!(exception)" << e.what());
                }
            });
}

void OKMGApi::ReqUnSubKline(const char10& Symbol, KlineTypeType klineType)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(logger, "[ok_mg][unsub_kline]\tnot connected to exchange now!");
        return;
    }

    if (klineStringMap.count(klineType) == 0)
    {
        DCSS_LOG_INFO(logger, "[ok_mg][unsub_kline]\tdon't support this kline type!");
        return;
    }

    std::stringstream ss;
    ss << "ok_sub_spot_" << Symbol << "_kline_" << klineStringMap.at(klineType);

    json::value jv;
    jv[U("event")] = json::value::string("removeChannel");
    jv[U("channel")] = json::value::string(ss.str());

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([&]()
            {
                DCSS_LOG_INFO(logger, "[ok_mg][unsub_kline]\tsend unsub "
                        << Symbol << " tick request success");

                mSubKlineMap[ss.str()] = std::make_pair(Symbol, klineType);
            }).then([&](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(logger, "[ok_mg][unsub_kline]\tsend unsub "
                            << Symbol << " tick request fail!(exception)" << e.what());
                }
            });
}