//
// Created by wangzhen on 18-8-14.
//

#include "BinaMGApi.h"
#include "GWHelper.hpp"
#include "Helper.h"

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

BinaMGApi::BinaMGApi(uint8_t source)
: IMGApi(source), mWsConnected(false)
{
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

    std::string sub = GetBinaSymbol(symbol) + "@ticker";
    websocket_outgoing_message msg;
    msg.set_utf8_message(sub);
    mWsClient->send(msg)
            .then(
                    [=]()
                    {
                        DCSS_LOG_INFO(mLogger,
                                "[binance] send sub " << GetBinaSymbol(symbol) << " tick request success");
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
                                    "[binance] send sub " << GetBinaSymbol(symbol) << " tick request fail!(expection)"
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

    std::string sub = GetBinaSymbol(symbol) + "@depth" + std::to_string(depth);
    websocket_outgoing_message msg;
    msg.set_utf8_message(sub);
    mWsClient->send(msg)
            .then(
                    [=]()
                    {
                        DCSS_LOG_INFO(mLogger, "[binance] send sub " << symbol << " depth "
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
                            DCSS_LOG_ERROR(mLogger, "[binance] send sub "
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

    if (BinanceKlineStringMap.count(klineType) == 0)
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
    msg.set_utf8_message(GetBinaSymbol(symbol) + "@kline_" + BinanceKlineStringMap.at(klineType))
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

void BinaMGApi::OnRtnKline(const json::object& jo)
{
    DCSSKlineHeaderField header;
    strcpy(header.Symbol, GetSymbolFromBina(jo.at("s").as_string()).c_str());
    header.Size = 1;
    const json::object& jkline = jo.at("k").as_object();
    header.Type = BinanceStringKlineMap.at(jkline.at("i").as_string());

    DCSSKlineField kline;
    SplitLongTime(jo.at("E").as_number().to_int64(), kline.Date, kline.Time, kline.Millisec);
    kline.OpenPrice = std::stod(jkline.at("o").as_string());
    kline.ClosePrice = std::stod(jkline.at("c").as_string());
    kline.Highest = std::stod(jkline.at("h").as_string());;
    kline.Lowest = std::stod(jkline.at("l").as_string());
    kline.Volume = std::stod(jkline.at("v").as_string());
    kline.StartTime = jkline.at("t").as_number().to_int64();
    kline.CloseTime = jkline.at("T").as_number().to_int64();

    mSpi->OnRtnKline(&header, {kline}, mSourceId);
}

void BinaMGApi::OnRtnTicker(const json::object& jo)
{
    DCSSTickerField ticker;
    SplitLongTime(jo.at("E").as_number().to_int64(), ticker.Date, ticker.Time, ticker.MilliSec);
    strcpy(ticker.Symbol, GetSymbolFromBina(jo.at("s").as_string()).c_str());
    ticker.LastPrice = std::stod(jo.at("w").as_string());
    ticker.BuyPrice = std::stod(jo.at("b").as_string());
    ticker.SellPrice = std::stod(jo.at("a").as_string());
    ticker.Highest = std::stod(jo.at("h").as_string());
    ticker.Lowest = std::stod(jo.at("l").as_string());

    mSpi->OnRtnTicker(&ticker, mSourceId);
}