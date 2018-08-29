//
// Created by wangzhen on 18-6-15.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_OKMGENGINE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_OKMGENGINE_H

#include "../IMGEngine.h"
#include "EnumClassHash.h"

#include <cpprest/ws_client.h>
#include <cpprest/details/web_utilities.h>
#include <cpprest/json.h>

using namespace web::websockets;
using namespace web::websockets::client;
using namespace web::json;
using namespace utility;
using namespace web;

class OKMGApi : public IMGApi
{
public:
    explicit OKMGApi(uint8_t source);
    ~OKMGApi() override ;

public:
    void Connect() override ;
    bool IsConnected() const override ;

public:
    void ReqSubTicker(const std::string& symbol) override ;
    void ReqSubDepth(const std::string& symbol, int depth) override ;
    void ReqSubKline(const std::string& symbol, KlineType klineType) override ;

    void ReqUnSubTicker(const std::string& symbol) override ;
    void ReqUnSubDepth(const std::string& symbol, int depth) override ;
    void ReqUnSubKline(const std::string& symbol, KlineType klineType) override ;

    std::string Name() const override {return "OK_MG"; }

private:
    void OnWsMessage(const websocket_incoming_message& msg);
    void OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error);

    void OnWsConnect();

    void OnRtnTick(const json::value& v, const std::string& symbol);
    void OnRtnKline(const json::value& v, const std::pair<std::string, KlineType>& pair);
    void OnRtnDepth(const json::value& v, const std::pair<std::string, int>& pair);

    void Ping();

private:

    static std::unordered_map<KlineType, std::string, EnumClassHash> klineStringMap;

    std::unique_ptr<websocket_callback_client> mWsClient;
    volatile bool IsWsConnected;
    volatile bool Ponged;

    std::map<std::string, std::string> mSubTickMap; // <request, symbol>
    std::map<std::string, std::pair<std::string, KlineType> > mSubKlineMap;
    std::map<std::string, std::pair<std::string, int> > mSubDepthMap;

    std::map<std::string, int> mSubTickNum;
    std::map<std::string, std::map<KlineType, int> > mSubKlineNum;
    std::map<std::string, std::map<int, int> > mSubDepthNum;

    std::unique_ptr<std::thread> mPingThread;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_OKMGENGINE_H
