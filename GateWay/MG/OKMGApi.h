//
// Created by wangzhen on 18-6-15.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_OKMGENGINE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_OKMGENGINE_H

#include "IMGEngine.h"
#include "Constants.h"
#include "DataStruct.h"

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
    OKMGApi();
    virtual ~OKMGApi();

public:
    virtual void Register(IMGApiPtr spi);

    virtual void Connect();
    virtual bool IsConnected();

public:
    virtual void ReqSubTicker(const char10& Symbol);
    virtual void ReqSubDepth(const char10& Symbol, int depth);
    virtual void ReqSubKline(const char10& Symbol, KlineTypeType klineType);

    virtual void ReqUnSubTicker(const char10& Symbol);
    virtual void ReqUnSubDepth(const char10& Symbol, int depth);
    virtual void ReqUnSubKline(const char10& Symbol, KlineTypeType klineType);

private:
    void OnWsMessage(const websocket_incoming_message& msg);
    void OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error);

    void OnWsConnect();

    void OnRtnTick(const json::value& v, const std::string& symbol);
    void OnRtnKline(const json::value& v, const std::pair<std::string, KlineTypeType>& pair);
    void OnRtnDepth(const json::value& v, const std::pair<std::string, int>& pair);

private:
    static std::map<KlineTypeType, std::string> klineStringMap;
    static std::map<std::string, KlineTypeType> stringKlineMap;

    std::shared_ptr<websocket_callback_client> mWsClient;
    volatile bool IsWsConnected;

    std::map<std::string, std::string> mSubTickMap; // <request, symbol>
    std::map<std::string, std::pair<std::string, KlineTypeType> > mSubKlineMap;
    std::map<std::string, std::pair<std::string, int> > mSubDepthMap;

    IMGEnginePtr mSpi;

};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_OKMGENGINE_H
