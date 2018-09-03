/*
 * IDCSSStrategyImpl.h
 *
 *  Created on: 2018年8月30日
 *      Author: wangzhen
 */

#ifndef STRATEGY_DCSSSTRATEGYIMPL_H_
#define STRATEGY_DCSSSTRATEGYIMPL_H_

#include <unordered_map>
#include <unordered_set>
#include "DCSSLog.h"
#include "DCSSDataWrapper.h"
#include "DCSSStrategyUtil.h"
#include "IDCSSDataProcessor.h"

class IDCSSStrategy;

class DCSSStrategyImpl : public IDCSSDataProcessor
{
public:
	DCSSStrategyImpl(const std::string& name, IDCSSStrategy* strategy);

	~DCSSStrategyImpl();

public:
    bool Init(const std::string& path);

    int InsertOrder(uint8_t source, const std::string& symbol, double price, double volume, OrderDirection direction, OrderType type);

    int CancelOrder(uint8_t source, const std::string& symbol, long orderId);

    int QryTicker(uint8_t source, const std::string& symbol);

    int QryKline(uint8_t source, const std::string& symbol, KlineType klineType, int size = 0, long since = 0);

    void SubscribeTicker(uint8_t source, const std::string& symbol);

    void SubscribeKline(uint8_t source, const std::string& symbol, KlineType klineType);

    void SubscribeDepth(uint8_t source, const std::string& symbol, int depth);

    DCSSLog* GetLogger() const ;

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

public:
    void OnRtnTicker(const DCSSTickerField* ticker, uint8_t source, long recvTime) override ;

    void OnRtnKline(const DCSSKlineField* kline, uint8_t source, long recvTime) override ;

    void OnRtnDepth(const DCSSDepthField* depth, uint8_t source, long recvTime) override ;

    void OnRtnOrder(const DCSSOrderField* order, uint8_t source, long recvTime) override ;

    void OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, int requestID, int errorId, const char* errorMsg, uint8_t source, long recvTime) override ;

    void OnRspQryTicker(const DCSSTickerField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) override ;

    void OnRspQryKline(const DCSSKlineField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) override ;

    void OnRspQryOrder(const DCSSOrderField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) override ;

    void Debug(const char* msg) override ;

    void OnTime(long curTime) override ;

    std::string GetName() const override;

    void OnRspQryTradingAccount(const DCSSTradingAccountField* account, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) final ;

    void OnRtnBalance(const DCSSBalanceField* balance, uint8_t source, long recvTime) final ;

private:
    int QryTradingAccount(uint8_t source);

    int QryOpenOrder(uint8_t source);

    bool IsTdReady(uint8_t source) const;

    bool IsTdConnected(uint8_t source) const;

    void CheckOrder(uint8_t source, const std::string& symbol, double price, double volume, OrderDirection direction, OrderType type);

	std::pair<std::string, std::string> SplitStrSymbol(const std::string& symbol);

private:
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

    std::string mConfigPath;

    std::string mConfig;

    IDCSSStrategy* mStrategy;

    std::mutex mMutex;

    std::condition_variable mCondVar;
};

#endif /* STRATEGY_DCSSSTRATEGYIMPL_H_ */
