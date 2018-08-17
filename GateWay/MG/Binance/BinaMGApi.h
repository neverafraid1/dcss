//
// Created by wangzhen on 18-8-14.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_BINAMGAPI_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_BINAMGAPI_H

#include "../IMGEngine.h"
#include "Helper.h"

#include <cpprest/ws_client.h>
#include <cpprest/details/web_utilities.h>
#include <cpprest/json.h>

using namespace web::websockets;
using namespace web::websockets::client;
using namespace web::json;
using namespace utility;
using namespace web;

class BinaMGApi : public IMGApi
{
public:
    explicit BinaMGApi(uint8_t source);
    ~BinaMGApi() override;

public:
    void Connect() override ;
    bool IsConnected() const override ;

public:
    void ReqSubTicker(const std::string& symbol) override ;
    void ReqSubDepth(const std::string& symbol, int depth) override ;
    void ReqSubKline(const std::string& symbol, KlineType klineType) override ;

    void ReqUnSubTicker(const std::string& symbol) override {}
    void ReqUnSubDepth(const std::string& symbol, int depth) override {}
    void ReqUnSubKline(const std::string& symbol, KlineType klineType) override {}

    std::string Name() const override {return "BINA_MG"; }

private:
    void OnWsMessage(const websocket_incoming_message& msg);
    void OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error);

    void OnRtnKline(const json::object& jo);
    void OnRtnTicker(const json::object& jo);

    void OnWsConnect();

private:
    static std::unordered_map<KlineType, std::string, EnumClassHash> klineStringMap;

    std::unique_ptr<websocket_callback_client> mWsClient;
    volatile bool mWsConnected;

    std::unordered_map<std::string, int> mSubTickNum;
    std::unordered_map<std::string, std::unordered_map<KlineType, int, EnumClassHash> > mSubKlineNum;
    std::unordered_map<std::string, std::unordered_map<int, int> > mSubDepthNum;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_BINAMGAPI_H