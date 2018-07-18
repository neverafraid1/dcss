//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGY_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGY_H

#include "IDCSSDataProcessor.h"
#include "DCSSDataWrapper.h"
#include "DCSSLog.h"

class IDCSSStrategy : public IDCSSDataProcessor
{
public:
    void OnRtnTicker(const DCSSTickerField& ticker, short source, long recvTime) override ;

    void OnRtnKline(const DCSSKlineHeaderField& header,
    const std::vector<DCSSKlineField>& kline,
    short source,
    long recvTime) override ;

    void OnRtnDepth(const DCSSDepthHeaderField& header,
    const std::vector<DCSSDepthField>& ask,
    const std::vector<DCSSDepthField>& bid,
    short source,
    long recvTime) override {}

    void OnRtnOrder(const DCSSOrderField& order, int requestID, short source, long recvTime) override ;

    void OnRspOrderInsert(const DCSSRspInsertOrderField& rsp, int requestID, short source, long recvTime) override ;

    void Debug(const char* msg) override;

    void OnTime(long curTime) override ;

    std::string GetName() const override { return mName; };

public:
    int InsertLimitOrder(short source, const DCSSSymbolField& symbol, double price, double volume, TradeTypeType tradeType);

    int InsertMarketOrder(short source, const DCSSSymbolField& symbol, double volume, TradeTypeType tradeType);

    int CancelOrder(short source, const DCSSSymbolField& symbol, long orderId);

//    int ReqQryTicker(short source, const DCSSSymbolField& symbol);
public:
    virtual ~IDCSSStrategy();

    explicit IDCSSStrategy(const std::string& name);
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

protected:
    bool IsTdReady(short source) const;

    bool IsTdConnected(short source) const;

protected:
    /*logger*/
    DCSSLogPtr mLogger;
    /*strategy name*/
    const std::string mName;
    /*data thread*/
    ThreadPtr mDataThread;
    /*data wrapper*/
    DCSSDataWrapperPtr mData;
    /*strategy utils*/
    DCSSStrategyUtilPtr mUtil;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGY_H
