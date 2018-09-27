//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_OKTGENGINE_H
#define DCSS_OKTGENGINE_H

#include "../ITGApi.h"
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

class OKTGApi: public ITGApi
{
public:
    OKTGApi();
    ~OKTGApi() override ;

public:
    void LoadAccount(const nlohmann::json& config) override ;
    void Connect() override ;
    void Disconnect() override;
    void Login() override ;
    void Logout() override {};
    bool IsConnected() const override;
    bool IsLogged() const override;

public:
    void ReqQryTicker(const DCSSReqQryTickerField* req, int requestID) override ;
    void ReqQryKline(const DCSSReqQryKlineField* req, int requestID) override ;
    void ReqQryUserInfo(int requestID) override ;
    void ReqQryOrder(const DCSSReqQryOrderField* req, int requestID) override ;
    void ReqInsertOrder(const DCSSReqInsertOrderField* req, int requestID) override ;
    void ReqCancelOrder(const DCSSReqCancelOrderField* req, int requestID) override ;
    void ReqQryOpenOrder(const DCSSReqQryOrderField* req, int requestID) override ;
    void ReqQrySymbol(const DCSSReqQrySymbolField* req, int requestID) override ;

private:
    void OnWsConnected();
    void OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error);

    void OnRspQryTicker(http_response& response, int requestID, const char21& symbol);
    void OnRspQryKline(http_response& response, const DCSSReqQryKlineField* req, int requestID);
    void OnRspQryUserInfo(http_response& response, int requestID);
    void OnRspInsertOrder(http_response& response, int requestID);
    void OnRspCancelOrder(http_response& response, const DCSSReqCancelOrderField* req, int requestID);
    void OnRspQryOrder(http_response& response, const DCSSReqQryOrderField* req, int requestID);
    void OnRspQryOpenOrder(http_response& response, const DCSSReqQryOrderField* req, int requestID);

private:

    void ResetRestClient();

    void OnWSMessage(const websocket_incoming_message& msg);

    void AddApiKey(uri_builder& builder);
    void AddSecKeyAndSign(uri_builder& builder);

    void OnRtnOrder(const json::object& j);
    void OnRtnBalance(const json::object& j);
    void OnUserLogin(const json::object& j);

    void Ping();

private:
    static std::unordered_map<KlineType, std::string, EnumClassHash> klineStringMap;
    static std::unordered_map<OrderType, std::string, EnumClassHash> orderTypeStringMap;
    static std::unordered_map<OrderDirection, std::string, EnumClassHash> directionStringMap;
    static std::unordered_map<int, OrderStatus> intOrderStatusMap;

    std::string mApiKey;
    std::string mSecretKey;
    std::string mProxy;

    http_client* mRestClient;
    websocket_callback_client* mWsClient;

    volatile bool IsRestConnected;
    volatile bool IsWsConnected;
    volatile bool IsLoggined;
    volatile bool IsPonged;

    std::unique_ptr<std::thread> mPingThread;

    std::unordered_map<std::string, DCSSSymbolField> mSymbolInfo;
};

#endif //DEMO_OKTGENGINE_H
