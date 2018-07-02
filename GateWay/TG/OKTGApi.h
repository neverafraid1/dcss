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

namespace DCSS
{

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
    OKTGApi(std::string apiKey, std::string secretKey);
    virtual ~OKTGApi();

public:
    virtual void Connect();
    virtual void Login();
    virtual void Logout(){};
    virtual bool IsConnected() const;
    virtual bool IsLogged();
    virtual void Register(ITGEnginePtr spi);

public:
    virtual void ReqQryTicker(const DCSSReqQryTickerField* req, int requestID);
    virtual void ReqQryKline(const DCSSReqQryKlineField* req, int requestID);
    virtual void ReqQryUserInfo(int requestID);
    virtual void ReqQryOrder(const DCSSReqQryOrderField* req, int requestID);

    virtual void ReqInsertOrder(const DCSSReqInsertOrderField* req, int requestID);
    virtual void ReqCancelOrder(const DCSSReqCancelOrderField* req, int requestID);

private:
    void OnRestConnected();
    void OnWsConnected();
    void OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error);
    void WsConnectFail(const std::string& reason);

    void OnRspError(std::string reason, int requestID);
    void OnRspQryTicker(http_response& response, int requestID, const char10& symbol);
    void OnRspQryKline(http_response& response, const DCSSReqQryKlineField* req, int requestID);
    void OnRspQryUserInfo(http_response& response, int requestID);
    void OnRspInsertOrder(http_response& response, int requestID);
    void OnRspCancelOrder(http_response& response, int requestID);
    void OnRspQryOrder(http_response& response, const DCSSReqQryOrderField* req, int requestID);

private:

    void OnWSMessage(const websocket_incoming_message& msg);

    void AddApiKey(uri_builder& builder);
    void AddSecKeyAndSign(uri_builder& builder);

    void OnRtnOrder(const json::object& j);
    void OnRtnBalance(const json::object& j);
    void OnUserLogin(const json::object& j);

private:

    static std::map<KlineTypeType, std::string> klineStringMap;
    static std::map<std::string, KlineTypeType> stringKlineMap;
    static std::map<TradeTypeType, std::string> tradeTypeStringMap;
    static std::map<std::string, TradeTypeType> stringTradeTypeMap;

    const std::string mApiKey;
    const std::string mSecretKey;

    std::unique_ptr<http_client> mRestClient;
    std::unique_ptr<websocket_callback_client> mWsClient;

    volatile bool IsRestConnected;
    volatile bool IsWsConnected;
    volatile bool IsLoggined;

    std::map<std::string, std::string> mSubOrderMap;
    std::map<std::string, std::string> mSubBalanceMap;

    ITGEnginePtr mSpi;

};

}

#endif //DEMO_OKTGENGINE_H