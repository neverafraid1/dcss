//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGY_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGY_H

#include <memory>
#include "DataStruct.h"

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

public:
    bool Init(const std::string& path);

    int InsertOrder(uint8_t source, const std::string& symbol, double price, double volume, OrderDirection direction, OrderType type);

    int CancelOrder(uint8_t source, const std::string& symbol, long orderId);

    int QryTicker(uint8_t source, const std::string& symbol);

    int QryKline(uint8_t source, const std::string& symbol, KlineType klineType, int size = 0, long since = 0);

    void SubscribeTicker(uint8_t source, const std::string& symbol);

    void SubscribeKline(uint8_t source, const std::string& symbol, KlineType klineType);

    void SubscribeDepth(uint8_t source, const std::string& symbol, int depth);

    void Debug(const char* msg);

    void Info(const char* msg);

    void Error(const char* msg);

public:
    /*start data thread*/
    virtual void Start();
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
