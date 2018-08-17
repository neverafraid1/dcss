//
// Created by wangzhen on 18-8-6.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_BINATGAPI_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_BINATGAPI_H

#include "../ITGApi.h"
#include "SymbolDao.hpp"
#include "Helper.h"

#include <cpprest/http_client.h>
#include <cpprest/ws_client.h>
#include <cpprest/details/web_utilities.h>

using namespace web::http;
using namespace web::http::client;
using namespace web::websockets;
using namespace web::websockets::client;
using namespace web::json;
using namespace utility;
using namespace web;

class BinaTGApi : public ITGApi
{
public:
    explicit BinaTGApi(uint8_t source);
    ~BinaTGApi() override ;
public:
    void LoadAccount(const nlohmann::json& config) override ;
    void Connect() override ;

    void Disconnect() override ;
    void Login() override ;
    void Logout() override {}
    bool IsConnected() const override ;
    bool IsLogged() const override ;
    void ReqQryTicker(const DCSSReqQryTickerField* req, int requestID) override ;
    void ReqQryKline(const DCSSReqQryKlineField* req, int requestID) override ;
    void ReqQryUserInfo(int requestID) override ;
    void ReqQryOrder(const DCSSReqQryOrderField* req, int requestID) override {}

    void ReqInsertOrder(const DCSSReqInsertOrderField* req, int requestID) override ;
    void ReqCancelOrder(const DCSSReqCancelOrderField* req, int requestID) override {}

private:
    void OnWsMessage(const websocket_incoming_message& msg);
    void OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error);
    void OnWsConnected();

    void OnRspQryUserInfo(http_response& response, int requestID);
    void OnRspQryKline(http_response& response, const DCSSReqQryKlineField* req, int requestID);
    void OnRspOrderInsert(http_response& response, const DCSSReqInsertOrderField* req, int requestID);
    void OnRtnAccount(const json::object& jo);
    void OnRtnOrder(const json::object& jo);

    void HMAC_SHA256(uri_builder& builder);

    void Ping();

private:
    static std::unordered_map<OrderDirection, std::string, EnumClassHash> enumDirectionMap;
    static std::unordered_map<std::string, OrderDirection> directionEnumMap;
    static std::unordered_map<OrderType, std::string, EnumClassHash> enumOrderTypeMap;
    static std::unordered_map<std::string, OrderType> orderTypeEnumMap;
    static std::unordered_map<KlineType, std::string, EnumClassHash> klineMap;

    std::string mApiKey;
    std::string mSecretKey;

    std::string mListenKey;

    std::unique_ptr<http_client> mRestClient;
    std::unique_ptr<websocket_callback_client> mWsClient;

    std::unique_ptr<std::thread> mPingThread;

    std::atomic_bool mWsConnected;
    std::atomic_bool mLogined;

    long mAccountLastUpdateTime;
    std::unordered_map<int, long> mOrderLastUpdateTime;

    //map<BTCLTC, btc_ltc>
    std::unordered_map<std::string, std::string> mBinaToCommonSymbolMap;
    //map<btc_ltc, BTCLTC>
    std::unordered_map<std::string, std::string> mCommonToBinaSymbolMap;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_BINATGAPI_H
