//
// Created by wangzhen on 18-8-14.
//

#include "BinaMGApi.h"
#include "Helper.h"
#include "SymbolDao.hpp"

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
        std::transform(common.begin(), common.end(), common.begin(), ::toupper);
        mBinaToCommonSymbolMap[std::get<0>(item)] = common;
        mCommonToBinaSymbolMap[common] = std::get<0>(item);
    }

    websocket_client_config config;
    mWsClient.reset(new websocket_callback_client(config));

    mWsClient->set_message_handler(std::bind(&BinaMGApi::OnWsMessage, this, std::placeholders::_1));
    mWsClient->set_close_handler(std::bind(&BinaMGApi::OnWsClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

BinaMGApi::~BinaMGApi()
{
    mWsClient->close(websocket_close_status::normal, "destruct");
}

void BinaMGApi::Connect()
{
    mWsClient->connect("wss://stream.binance.com:9443")
            .then(
                    [this]()
                    {
                        OnWsConnect();
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
                            DCSS_LOG_ERROR(mLogger, "[BinaMGApi]send connect failed!(exception)" << e.what());
                        }
                    }
            );
}

void BinaMGApi::OnWsConnect()
{
    DCSS_LOG_INFO(mLogger, "connect to binance success!");
    mWsConnected = true;
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

    std::string sub = mCommonToBinaSymbolMap.at(symbol) + "@ticker";
    websocket_outgoing_message msg;
    msg.set_utf8_message(sub);
    mWsClient->send(msg)
            .then(
                    [=]()
                    {
                        DCSS_LOG_INFO(mLogger,
                                "[binance mg] send sub " << symbol << " tick request success");
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
                            DCSS_LOG_ERROR(mLogger,
                                    "[binance mg] send sub " << symbol << " tick request fail!(expection)"
                                                          << e.what());
                        }

                    });
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

    std::string sub = mCommonToBinaSymbolMap.at(symbol) + "@depth" + std::to_string(depth);
    websocket_outgoing_message msg;
    msg.set_utf8_message(sub);
    mWsClient->send(msg)
            .then(
                    [=]()
                    {
                        DCSS_LOG_INFO(mLogger, "[binance mg] send sub " << symbol << " depth "
                                                                     << depth << " request success");
                    }
            )
            .then(
                    [=](pplx::task<void> task)
                    {
                        try
                        {
                            task.get();
                        }
                        catch (const std::exception& e)
                        {
                            DCSS_LOG_ERROR(mLogger, "[binance mg] send sub "
                                    << symbol << " depth " << depth << " request fail!(exception)" << e.what());
                        }
                    }
            );
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

    websocket_outgoing_message msg;
    msg.set_utf8_message(mCommonToBinaSymbolMap.at(symbol) + "@kline_" + klineStringMap.at(klineType));
    mWsClient->send(msg)
            .then([=]()
            {
                DCSS_LOG_INFO(mLogger, "[binance] send sub " << symbol << " kline request success");
            }).then([=](pplx::task<void> task)
            {
                try
                {
                    task.get();
                }
                catch (const std::exception& e)
                {
                    DCSS_LOG_ERROR(mLogger,
                            "[binance] send sub " << symbol << " kline request failed (exception)" << e.what());
                }

            });
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

    mSpi->OnRtnTicker(&ticker, mSourceId);
}
