//
// Created by wangzhen on 18-6-15.
//

#include "OKMGApi.h"
#include "Timer.h"
#include "OkexConstant.h"
using namespace OkexConstant;

std::unordered_map<KlineType, std::string, EnumClassHash> OKMGApi::klineStringMap = {
        {KlineType::Min1,   "1min"},
        {KlineType::Min3,   "3min"},
        {KlineType::Min5,   "5min"},
        {KlineType::Min15,  "15min"},
        {KlineType::Min30,  "30min"},
        {KlineType::Hour1,  "1hour"},
        {KlineType::Hour2,  "2hour"},
        {KlineType::Hour4,  "4hour"},
        {KlineType::Hour6,  "6hour"},
        {KlineType::Hour12, "12hour"},
        {KlineType::Day1,   "1day"},
        {KlineType::Day3,   "3day"},
        {KlineType::Week1,  "1week"}
};

OKMGApi::OKMGApi(uint8_t source)
: IMGApi(source), IsWsConnected(false), Ponged(true)
{
}

OKMGApi::~OKMGApi()
{
    mWsClient->close();
    mWsClient.reset();
    mPingThread.reset();
}

void OKMGApi::Connect()
{
	if (mProxy.empty())
	{
		mWsClient.reset(new websocket_callback_client());
	}
	else
	{
		web_proxy proxy(mProxy);
		websocket_client_config config;
		config.set_proxy(proxy);
		mWsClient.reset(new websocket_callback_client(config));
	}

	mWsClient->set_message_handler(std::bind(&OKMGApi::OnWsMessage, this, std::placeholders::_1));
	mWsClient->set_close_handler(std::bind(&OKMGApi::OnWsClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    mWsClient->connect(U(WS_API_BASE_URL))
            .then([this]()
            {
                OnWsConnect();
            })
            .then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[connect][okex mg] send connect failed!(exception)" << e.what());
                }
            });
}

void OKMGApi::OnWsMessage(const web::websockets::client::websocket_incoming_message& msg)
{
    json::value jv = json::value::parse(msg.extract_string().get());
    if (jv.is_object())
    {
        std::string event = jv.as_object().at(U("event")).as_string();
        if (event == "pong")
        {
            Ponged = true;
        }
    }
    else if (jv.is_array())
    {
        const json::array& ja = jv.as_array();

        for (auto& i : ja)
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
                    DCSS_LOG_ERROR(mLogger, "[sub] (channel)" << channel << " fail");
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
                // TODO
//            bool result = data.at(U("result")).as_bool();
                break;
            }
        }
    }
}

void OKMGApi::OnWsClose(web::websockets::client::websocket_close_status close_status,
        const utility::string_t& reason, const std::error_code& error)
{
    DCSS_LOG_INFO(mLogger, "[wsclose] (close_status)" << (int)close_status
                                              <<  "(error)" << error.message());

    IsWsConnected = false;

    if (close_status != websocket_close_status::normal)
    {
    	std::thread t(std::bind(&OKMGApi::Connect, this));
    }
}

void OKMGApi::OnWsConnect()
{
    DCSS_LOG_INFO(mLogger, "[connect] connect to okex success!");
    IsWsConnected = true;

    mPingThread.reset(new std::thread(&OKMGApi::Ping, this));
}

void OKMGApi::OnRtnTick(const json::value& v, const std::string& symbol)
{
    const json::object& j = v.as_object();
    DCSSTickerField ticker;

    strcpy(ticker.Symbol, symbol.c_str());
    ticker.UpdateTime = j.at(U("timestamp")).as_number().to_int64();
    ticker.BuyPrice = std::stod(j.at(U("buy")).as_string());
    ticker.SellPrice = std::stod(j.at(U("sell")).as_string());
    ticker.Highest = std::stod(j.at(U("high")).as_string());
    ticker.Lowest = std::stod(j.at(U("low")).as_string());
    ticker.LastPrice = std::stod(j.at(U("last")).as_string());
    ticker.Volume = std::stod(j.at(U("vol")).as_string());

    mSpi->OnRtnTicker(&ticker, mSourceId);
}

void OKMGApi::OnRtnKline(const web::json::value& v, const std::pair<std::string, KlineType>& pair)
{
    const json::array& arr = v.as_array();

    DCSSKlineField kline;
    strcpy(kline.Symbol, pair.first.c_str());
    kline.Type = pair.second;

    for (size_t i = 0; i < arr.size(); ++i)
    {
        const json::array& klineArr = arr.at(i).as_array();
        kline.UpdateTime = std::stol(klineArr.at(0).as_string());
        kline.OpenPrice = std::stod(klineArr.at(1).as_string());
        kline.Highest = std::stod(klineArr.at(2).as_string());
        kline.Lowest = std::stod(klineArr.at(3).as_string());
        kline.ClosePrice = std::stod(klineArr.at(4).as_string());
        kline.Volume = std::stod(klineArr.at(5).as_string());

        mSpi->OnRtnKline(&kline, mSourceId);
    }
}

void OKMGApi::OnRtnDepth(const web::json::value& v, const std::pair<std::string, int>& pair)
{
    const json::array& ask = v.as_object().at(U("asks")).as_array();
    const json::array& bid = v.as_object().at(U("bids")).as_array();

    DCSSDepthField depth;
	strcpy(depth.Symbol, pair.first.c_str());
	depth.UpdateTime = v.at(U("timestamp")).as_number().to_int64();

    for (size_t i = 0; i < ask.size() && i < MAX_DEPTH_NUM; ++i)
    {
        const json::array& arr = ask.at(i).as_array();
        depth.AskDepth[i].Price = std::stod(arr.at(0).as_string());
        depth.AskDepth[i].Volume = std::stod(arr.at(1).as_string());
    }
    for (size_t i = 0; i < bid.size() && i < MAX_DEPTH_NUM; ++i)
    {
        const json::array& arr = bid.at(i).as_array();
        depth.BidDepth[i].Price = std::stod(arr.at(0).as_string());
        depth.BidDepth[i].Volume = std::stod(arr.at(1).as_string());
    }

    mSpi->OnRtnDepth(&depth, mSourceId);
}

bool OKMGApi::IsConnected() const
{
    return IsWsConnected;
}

void OKMGApi::Ping()
{
    while (IsConnected())
    {
        if (!Ponged)
        {
            DCSS_LOG_ERROR(mLogger, "[Ping] don't recv pong in the last 30 seconds");
            IsWsConnected = false;
        }
        else
        {
            Ponged = false;
            json::value jv;
            jv[U("event")] = json::value::string("ping");
            websocket_outgoing_message msg;
            msg.set_utf8_message(jv.serialize());
            mWsClient->send(msg);
        }
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
}

void OKMGApi::ReqSubTicker(const std::string& symbol)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(mLogger, "[sub_tick] not connected to exchange now!");
        return;
    }

    if (mSubTickNum.count(symbol) > 0)
    {
        ++mSubTickNum.at(symbol);
        return;
    }

    mSubTickNum[symbol] = 1;
    std::string sub = "ok_sub_spot_" + symbol + "_ticker";
    json::value jv;
    jv[U("event")] = json::value::string("addChannel");
    jv[U("channel")] = json::value::string(sub);

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([=]()
            {
                DCSS_LOG_INFO(mLogger, "[okex mg] send sub "
                                      << symbol << " tick request success");
                mSubTickMap[sub] = symbol;
            }).then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[okex mg] send sub "
                    << symbol << " tick request fail!(exception)" << e.what());
                }
            });
}

void OKMGApi::ReqSubDepth(const std::string& symbol, int depth)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(mLogger, "[okex mg] not connected to exchange now!");
        return;
    }

    if (mSubDepthNum.count(symbol) > 0 && mSubDepthNum.at(symbol).count(depth) > 0)
    {
        ++mSubDepthNum.at(symbol).at(depth);
        return;
    }
    mSubDepthNum[symbol][depth] = 1;

    std::string sub = "ok_sub_spot_" + symbol + "_depth";
    if (depth > 0)
        sub.append("_").append(std::to_string(depth));

    json::value jv;
    jv[U("event")] = json::value::string("addChannel");
    jv[U("channel")] = json::value::string(sub);

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([=]()
            {
                DCSS_LOG_INFO(mLogger, "[okex mg] send sub "
                        << symbol << " depth " << depth << " request success");

                mSubDepthMap[sub] = std::make_pair(symbol, depth);
            }).then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[okex mg] send sub "
                            << symbol << " depth "<< depth << "  request fail!(exception)" << e.what());
                }
            });
}

void OKMGApi::ReqSubKline(const std::string& symbol, KlineType klineType)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(mLogger, "[okex mg] not connected to exchange now!");
        return;
    }

    if (klineStringMap.count(klineType) == 0)
    {
        DCSS_LOG_INFO(mLogger, "[okex mg] don't support this kline type!");
        return;
    }

    if (mSubKlineNum.count(symbol) > 0 && mSubKlineNum.at(symbol).count(klineType) > 0)
    {
        ++mSubKlineNum.at(symbol).at(klineType);
        return;
    }

    mSubKlineNum[symbol][klineType] = 1;

    std::string sub = "ok_sub_spot_" + symbol + "_kline_" + klineStringMap.at(klineType);

    json::value jv;
    jv[U("event")] = json::value::string("addChannel");
    jv[U("channel")] = json::value::string(sub);

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([=]()
            {
                DCSS_LOG_INFO(mLogger, "[okex mg] send sub "
                        << symbol << " kline request success");

                mSubKlineMap[sub] = std::make_pair(symbol, klineType);
            }).then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[okex mg] send sub "
                            << symbol << " kline request fail!(exception)" << e.what());
                }
            });
}

void OKMGApi::ReqUnSubTicker(const std::string& symbol)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(mLogger, "[okex mg] not connected to exchange now!");
        return;
    }

    if (--mSubTickNum[symbol] > 0)
        return;

    std::string unsub = "ok_sub_spot_" + symbol + "_ticker";

    if (mSubTickMap.count(unsub) == 0)
    {
        DCSS_LOG_INFO(mLogger, "[okex mg] " << symbol << " tick is not sub");
        return;
    }

    json::value jv;
    jv[U("event")] = json::value::string("removeChannel");
    jv[U("channel")] = json::value::string(unsub);

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([=]()
            {
                DCSS_LOG_INFO(mLogger, "[okex mg] send unsub "
                        << symbol << " tick request success");
            }).then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[okex mg] send unsub "
                            << symbol << " tick request fail!(exception)" << e.what());
                }
            });
}

void OKMGApi::ReqUnSubDepth(const std::string& symbol, int depth)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(mLogger, "[okex mg] not connected to exchange now!");
        return;
    }

    if (--mSubDepthNum[symbol][depth] > 0)
        return;

    std::string unsub = "ok_sub_spot_" + symbol + "_depth";
    if (depth > 0)
        unsub + "_" + std::to_string(depth);

    json::value jv;
    jv[U("event")] = json::value::string("removeChannel");
    jv[U("channel")] = json::value::string(unsub);

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([=]()
            {
                DCSS_LOG_INFO(mLogger, "[okex mg] send unsub "
                        << symbol << " depth request success");

                mSubDepthMap[unsub] = std::make_pair(symbol, depth);
            }).then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[okex mg] send unsub "
                            << symbol << " depth request fail!(exception)" << e.what());
                }
            });
}

void OKMGApi::ReqUnSubKline(const std::string& symbol, KlineType klineType)
{
    if (!IsWsConnected)
    {
        DCSS_LOG_INFO(mLogger, "[okex mg] not connected to exchange now!");
        return;
    }

    if (klineStringMap.count(klineType) == 0)
    {
        DCSS_LOG_INFO(mLogger, "[okex mg] don't support this kline type!");
        return;
    }

    if (--mSubKlineNum[symbol][klineType] > 0)
        return;

    std::string unsub = "ok_sub_spot_" + symbol + "_kline_" + klineStringMap.at(klineType);

    json::value jv;
    jv[U("event")] = json::value::string("removeChannel");
    jv[U("channel")] = json::value::string(unsub);

    websocket_outgoing_message msg;
    msg.set_utf8_message(jv.serialize());
    mWsClient->send(msg)
            .then([=]()
            {
                DCSS_LOG_INFO(mLogger, "[okex mg] send unsub "
                        << symbol << " kline request success");

                mSubKlineMap[unsub] = std::make_pair(symbol, klineType);
            }).then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger, "[okex mg] send unsub "
                            << symbol << " kline request fail!(exception)" << e.what());
                }
            });
}
