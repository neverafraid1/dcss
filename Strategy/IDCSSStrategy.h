//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGY_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGY_H

#include <unordered_set>

#include "IDCSSDataProcessor.h"
#include "DCSSDataWrapper.h"
#include "DCSSLog.h"

class IDCSSStrategy : public IDCSSDataProcessor
{
public:
    explicit IDCSSStrategy(const std::string& name);

    virtual ~IDCSSStrategy();

public:
    void OnRtnTicker(const DCSSTickerField* ticker, uint8_t source, long recvTime) override ;

    void OnRtnKline(const DCSSKlineHeaderField* header,
            const std::vector<const DCSSKlineField*>& kline,
            uint8_t source,
            long recvTime) override;

    void OnRtnDepth(const DCSSDepthHeaderField* header,
            const std::vector<const DCSSDepthField*>& ask,
            const std::vector<const DCSSDepthField*>& bid,
            uint8_t source,
            long recvTime) override;

    void OnRtnOrder(const DCSSOrderField* order, uint8_t source, long recvTime) override ;

    void OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, int requestID, int errorId, const char* errorMsg, uint8_t source, long recvTime) override ;

    void OnRspQryTicker(const DCSSTickerField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) override ;

    void OnRspQryKline(const DCSSKlineHeaderField* header, const std::vector<const DCSSKlineField* >& kline,
            int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) override ;

    void OnRspQryOrder(const DCSSOrderField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) override ;

    void Debug(const char* msg) override;

    void OnTime(long curTime) override ;

    std::string GetName() const override { return mName; };

    void OnRspQryTradingAccount(const DCSSTradingAccountField* account, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) final ;

    void OnRtnBalance(const DCSSBalanceField* balance, uint8_t source, long recvTime) final ;

public:
    void Init(const std::vector<uint8_t>& tgSources, const std::vector<uint8_t>& mgSources);

    int InsertOrder(uint8_t source, const std::string& symbol, double price, double volume, TradeTypeType tradeType);

    int CancelOrder(uint8_t source, const std::string& symbol, long orderId);

    int QryTradingAccount(uint8_t source);

    int QryTicker(uint8_t source, const std::string& symbol);

    int QryKline(uint8_t source, const std::string& symbol, KlineTypeType klineType, int size = 0, long since = 0);

    void SubscribeTicker(uint8_t source, const std::string& symbol);

    void SubscribeKline(uint8_t source, const std::string& symbol, KlineTypeType klineType);

    void SubscribeDepth(uint8_t source, const std::string& symbol, int depth);

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

protected:
    bool IsTdReady(uint8_t source) const;

    bool IsTdConnected(uint8_t source) const;

private:
    void CheckOrder(uint8_t source, const std::string& symbol, double price, double volume, TradeTypeType tradeType);

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

private:

    struct Balance
    {
        double Free;
        double Freezed;

        Balance() : Free(0.0), Freezed(0.0) {}
    };

    /*map<source, map<currency, {free, frozen}> >*/
    std::unordered_map<uint8_t, std::unordered_map<std::string, Balance> > mBalance;

    std::unordered_set<uint8_t> mSourceSet;

    std::unordered_map<int, bool> mReqReady;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_ISTRATEGY_H
