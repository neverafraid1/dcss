//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGY_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGY_H

#include <memory>
#include "DataStruct.h"

#define IN
#define OUT
#define INOUT

class DCSSStrategyImpl;

class IDCSSStrategy
{
public:
    explicit IDCSSStrategy(const std::string& name);

    virtual ~IDCSSStrategy() = default;

public:
    virtual void OnRtnTicker(const DCSSTickerField* ticker, uint8_t source, long recvTime);

    virtual void OnRtnKline(const DCSSKlineField* kline, uint8_t source, long recvTime);

    virtual void OnRtnDepth(const DCSSDepthField* depth, uint8_t source, long recvTime);

    virtual void OnRtnOrder(const DCSSOrderField* order, uint8_t source, long recvTime);

    virtual void OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, int requestID, int errorId, const char* errorMsg, uint8_t source, long recvTime);

    virtual void OnRspQryTicker(const DCSSTickerField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime);

    virtual void OnRspQryKline(const DCSSKlineField* kline, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime);

    virtual void OnRspQryOrder(const DCSSOrderField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime);

    virtual void OnRspQrySymbol(const DCSSSymbolField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime);

public:
    bool SetConfigPath(const std::string& path);

    int InsertOrder(uint8_t source, const std::string& symbol, double price, double volume, OrderDirection direction, OrderType type);

    int CancelOrder(uint8_t source, const std::string& symbol, long orderId);

    int QryOrder(uint8_t source, const std::string& symbol, long orderId);

    int QryTicker(uint8_t source, const std::string& symbol);

    int QryKline(uint8_t source, const std::string& symbol, KlineType klineType, int size = 0, long since = 0);

    int QrySymbol(uint8_t source, const std::string& symbol);

    bool QrySymbolSync(IN uint8_t source, IN const std::string& symbol, OUT DCSSSymbolField& symbolField, OUT std::string& errorMsg, IN int timeOut = 10);

    bool QryCurrencyBalance(IN uint8_t source, IN const std::string& currency, OUT DCSSBalanceField& balance, OUT std::string& errorMsg);

    void SubscribeTicker(uint8_t source, const std::string& symbol);

    void SubscribeKline(uint8_t source, const std::string& symbol, KlineType klineType);

    void SubscribeDepth(uint8_t source, const std::string& symbol);

    void Debug(const std::string& msg);

    void Info(const std::string& msg);

    void Error(const std::string& msg);

public:
    /*start data thread*/
    void Start();
    /*run strategy in front end*/
    void Run();
    /*terminate data thread, should never be called within data thread*/
    void Terminate();
    /*send stop signal to data thread*/
    void Stop();
    /*block process by data thread*/
    void Block();

private:
    // can't use unique_ptr due to incomplete type
    std::shared_ptr<DCSSStrategyImpl> pImpl;

};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGY_H
