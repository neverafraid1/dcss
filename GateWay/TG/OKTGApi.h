//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_OKTGENGINE_H
#define DCSS_OKTGENGINE_H

#include "ITGEngine.h"

#include "Constants.h"
#include "DataStruct.h"

#include <string>
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

struct AccountUnitOK
{
    std::string ApiKey;
    std::string SecretKey;
    bool Connected;
    bool LoggedIn;
};

class OKTGApi: public ITGApi
{
public:
    explicit OKTGApi(uint8_t source);
    virtual ~OKTGApi() override ;

public:
    void LoadAccount(const nlohmann::json& config) override ;
    void Connect() override ;
    void Disconnect() override;
    void Login() override ;
    void Logout() override {};
    bool IsConnected() const override;
    bool IsLogged() const override;
    void Register(ITGEnginePtr spi) override;

public:
    void ReqQryTicker(const DCSSReqQryTickerField* req, int requestID) override ;
    void ReqQryKline(const DCSSReqQryKlineField* req, int requestID) override ;
    void ReqQryUserInfo(int requestID) override ;
    void ReqQryOrder(const DCSSReqQryOrderField* req, int requestID) override ;

    void ReqInsertOrder(const DCSSReqInsertOrderField* req, int requestID) override ;
    void ReqCancelOrder(const DCSSReqCancelOrderField* req, int requestID) override ;

private:
    void OnWsConnected();
    void OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error);
    void WsConnectFail(const std::string& reason);

    void OnRspQryTicker(http_response& response, int requestID, const char21& symbol);
    void OnRspQryKline(http_response& response, const DCSSReqQryKlineField* req, int requestID);
    void OnRspQryUserInfo(http_response& response, int requestID);
    void OnRspInsertOrder(http_response& response, int requestID);
    void OnRspCancelOrder(http_response& response, int requestID);
    void OnRspQryOrder(http_response& response, const DCSSReqQryOrderField* req, int requestID);

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
    static std::map<KlineTypeType, std::string> klineStringMap;
    static std::map<TradeTypeType, std::string> tradeTypeStringMap;
    static std::map<std::string, TradeTypeType> stringTradeTypeMap;

    std::string mApiKey;
    std::string mSecretKey;

    std::unique_ptr<http_client> mRestClient;
    std::unique_ptr<websocket_callback_client> mWsClient;

    volatile bool IsRestConnected;
    volatile bool IsWsConnected;
    volatile bool IsLoggined;
    volatile bool IsPonged;

    ITGEnginePtr mSpi;

    DCSSLogPtr mLogger;

    std::unique_ptr<std::thread> mPingThread;

};

#endif //DEMO_OKTGENGINE_H
