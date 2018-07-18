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
    explicit OKMGApi(short source);
    ~OKMGApi() override ;

public:
    void Register(IMGEnginePtr spi) override ;

    void Connect() override ;
    bool IsConnected() const override ;

public:
    void ReqSubTicker(const DCSSSymbolField& symbol) override ;
    void ReqSubDepth(const DCSSSymbolField& symbol, int depth) override ;
    void ReqSubKline(const DCSSSymbolField& symbol, KlineTypeType klineType) override ;

    void ReqUnSubTicker(const DCSSSymbolField& symbol) override ;
    void ReqUnSubDepth(const DCSSSymbolField& symbol, int depth) override ;
    void ReqUnSubKline(const DCSSSymbolField& symbol, KlineTypeType klineType) override ;

    std::string Name() const override {return "OK_MG"; }

private:
    void OnWsMessage(const websocket_incoming_message& msg);
    void OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error);

    void OnWsConnect();

    void OnRtnTick(const json::value& v, const DCSSSymbolField& symbol);
    void OnRtnKline(const json::value& v, const std::pair<DCSSSymbolField, KlineTypeType>& pair);
    void OnRtnDepth(const json::value& v, const std::pair<DCSSSymbolField, int>& pair);

    void Ping();

private:

    DCSSLogPtr mLogger;

    static std::map<KlineTypeType, std::string> klineStringMap;
    static std::map<std::string, KlineTypeType> stringKlineMap;

    std::shared_ptr<websocket_callback_client> mWsClient;
    volatile bool IsWsConnected;
    volatile bool Ponged;

    std::map<std::string, DCSSSymbolField> mSubTickMap; // <request, symbol>
    std::map<std::string, std::pair<DCSSSymbolField, KlineTypeType> > mSubKlineMap;
    std::map<std::string, std::pair<DCSSSymbolField, int> > mSubDepthMap;

    std::unique_ptr<std::thread> mPingThread;

    IMGEnginePtr mSpi;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_OKMGENGINE_H
