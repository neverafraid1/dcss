//
// Created by wangzhen on 18-8-6.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_BINATGAPI_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_BINATGAPI_H

#include "../ITGApi.h"
#include "SymbolDao.hpp"
#include "EnumClassHash.h"

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
    BinaTGApi();
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
    void ReqQryOrder(const DCSSReqQryOrderField* req, int requestID) override ;
    void ReqQryOpenOrder(const DCSSReqQryOrderField* req, int requestID) override ;

    void ReqInsertOrder(const DCSSReqInsertOrderField* req, int requestID) override ;
    void ReqCancelOrder(const DCSSReqCancelOrderField* req, int requestID) override ;
    void ReqQrySymbol(const DCSSReqQrySymbolField* req, int requestID) override ;

private:
    void OnWsMessage(const websocket_incoming_message& msg);
    void OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error);
    void OnWsConnected();

    void OnRspQryUserInfo(http_response& response, int requestID);
    void OnRspQryKline(http_response& response, const DCSSReqQryKlineField* req, int requestID);
    void OnRspQryTicker(http_response& response, const DCSSReqQryTickerField* req, int requestID);
    void OnRspQryOrder(http_response& response, const DCSSReqQryOrderField* req, int requestID);
    void OnRspQryOpenOrder(http_response& response, const DCSSReqQryOrderField* req, int requestID);
    void OnRspOrderInsert(http_response& response, const DCSSReqInsertOrderField* req, int requestID);
    void OnRspCancelOrder(http_response& response, const DCSSReqCancelOrderField* req, int requestID);
    void OnRtnAccount(const json::object& jo);
    void OnRtnOrder(const json::object& jo);

    void AddTimeStamp(uri_builder& builder);
    void HMAC_SHA256(uri_builder& builder);

    void Ping();

    void ResetRestClient();

private:
    static std::unordered_map<OrderDirection, std::string, EnumClassHash> enumDirectionMap;
    static std::unordered_map<std::string, OrderDirection> directionEnumMap;
    static std::unordered_map<OrderType, std::string, EnumClassHash> enumOrderTypeMap;
    static std::unordered_map<std::string, OrderType> orderTypeEnumMap;
    static std::unordered_map<KlineType, std::string, EnumClassHash> klineMap;
    static std::unordered_map<std::string, OrderStatus> statusEnumMap;

    std::string mApiKey;
    std::string mSecretKey;
    std::string mProxy;

    std::string mListenKey;

    http_client* mRestClient;
    websocket_callback_client* mWsClient;

    std::unique_ptr<std::thread> mPingThread;

    volatile bool mWsConnected;
    volatile bool mLogined;

    long mAccountLastUpdateTime;
    std::unordered_map<int, long> mOrderLastUpdateTime;

    //map<BTCLTC, btc_ltc>
    std::unordered_map<std::string, std::string> mBinaToCommonSymbolMap;
    //map<btc_ltc, BTCLTC>
    std::unordered_map<std::string, DCSSSymbolField> mCommonToBinaSymbolMap;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_BINATGAPI_H
