//
// Created by wangzhen on 18-8-14.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_BINAMGAPI_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_BINAMGAPI_H

#include <cpprest/ws_client.h>
#include <cpprest/details/web_utilities.h>
#include "../IMGEngine.h"
#include "EnumClassHash.h"

using namespace web::websockets;
using namespace web::websockets::client;
using namespace web::json;
using namespace utility;
using namespace web;

class BinaMGApi : public IMGApi
{
public:
    BinaMGApi();
    ~BinaMGApi() override;

public:
    void Connect() override ;
    bool IsConnected() const override ;

public:
    void ReqSubTicker(const std::string& symbol) override ;
    void ReqSubDepth(const std::string& symbol) override ;
    void ReqSubKline(const std::string& symbol, KlineType klineType) override ;

    void ReqUnSubTicker(const std::string& symbol) override ;
    void ReqUnSubDepth(const std::string& symbol) override ;
    void ReqUnSubKline(const std::string& symbol, KlineType klineType) override ;

    std::string Name() const override {return "BINA_MG"; }

private:
    void OnWsMessage(const websocket_incoming_message& msg);
    void OnWsTicker(const websocket_incoming_message& msg);
    void OnWsKline(const websocket_incoming_message& msg);
    void OnWsDepth(const websocket_incoming_message& msg);
    void OnWsClose(websocket_close_status close_status, const utility::string_t& reason, const std::error_code& error);

    void OnRtnKline(const json::object& jo);
    void OnRtnTicker(const json::object& jo);

    void OnWsConnect();

    std::shared_ptr<websocket_callback_client> CreateNewWsClient();

    std::string GetTickerChannel(const std::string& symbol);
    std::string GetKlineChannel(const std::string& symbol, KlineType type);
    std::string GetDepthChannel(const std::string& symbol);

    void GetDepthSnapShot(const std::string& symbol);

private:
    static std::unordered_map<KlineType, std::string, EnumClassHash> klineStringMap;
    static std::unordered_map<std::string, KlineType> stringKlineMap;

//    std::unique_ptr<websocket_callback_client> mWsClient;
    volatile bool mWsConnected;

    std::unordered_map<std::string, int> mSubTickNum;
    std::unordered_map<std::string, std::unordered_map<KlineType, int, EnumClassHash> > mSubKlineNum;
    std::unordered_map<std::string, int> mSubDepthNum;

    //map<BTCLTC, btc_ltc>
    std::unordered_map<std::string, std::string> mBinaToCommonSymbolMap;
    //map<btc_ltc, BTCLTC>
    std::unordered_map<std::string, DCSSSymbolField> mCommonToBinaSymbolMap;

    std::unordered_map<std::string, std::shared_ptr<websocket_callback_client> > mSubTickerClient;
    std::unordered_map<std::string, std::shared_ptr<websocket_callback_client> > mSubDepthClient;
    std::unordered_map<std::string, std::shared_ptr<websocket_callback_client> > mSubKlineClient;

    std::unordered_map<std::string, std::map<double, double, std::greater<double> > > mBidDepth;
    std::unordered_map<std::string, std::map<double, double, std::less<double> > > mAskDepth;

    // <symbo, pair<last final updateid, last updatetime> >
    std::unordered_map<std::string, std::pair<long, long> > mLastUpdateIdMap;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_BINAMGAPI_H
