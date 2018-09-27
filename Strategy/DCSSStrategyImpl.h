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
#include <mutex>
#include <condition_variable>
#include "DataStruct.h"
#include "DCSSLog.h"
#include "DCSSDataWrapper.h"
#include "StrategyUtil.h"

USING_UNIT_NAMESPACE

class IDCSSStrategy;

class DCSSStrategyImpl : public StrategyUtil
{
public:
    static volatile int mSingalReceived;

    static void SignalHandler(int sig) {mSingalReceived = sig;}
public:
	DCSSStrategyImpl(const std::string& name, IDCSSStrategy* strategy);

	~DCSSStrategyImpl();

public:
    bool Init(const std::string& path);

    int InsertOrder(uint8_t source, const std::string& symbol, double price, double volume, OrderDirection direction, OrderType type);

    int CancelOrder(uint8_t source, const std::string& symbol, long orderId);

    int QryOrder(uint8_t source, const std::string& symbol, long orderId);

    int QryTicker(uint8_t source, const std::string& symbol);

    int QryKline(uint8_t source, const std::string& symbol, KlineType klineType, int size = 0, long since = 0);

    int QrySymbol(uint8_t source, const std::string& symbol);

    bool QrySymbolSync(uint8_t source, const std::string& symbol, DCSSSymbolField& symbolField, std::string& errorMsg, int timeOut = 10);

    bool QryCurrencyBalance(uint8_t source, const std::string& currency, DCSSBalanceField& balance, std::string& errorMsg);

//    bool QryAccountBalanceSync(uint8_t source, std::string& errorMsg, int timeOut = 10);

    void SubscribeTicker(uint8_t source, const std::string& symbol);

    void SubscribeKline(uint8_t source, const std::string& symbol, KlineType klineType);

    void SubscribeDepth(uint8_t source, const std::string& symbol);

    DCSSLog* GetLogger() const ;

public:
    void OnRtnTicker(const DCSSTickerField* ticker, uint8_t source, long recvTime);

    void OnRtnKline(const DCSSKlineField* kline, uint8_t source,
            long recvTime);

    void OnRtnDepth(const DCSSDepthField* depth, uint8_t source,
            long recvTime);

    void OnRtnOrder(const DCSSOrderField* order, uint8_t source,
            long recvTime);

    void OnRtnBalance(const DCSSBalanceField* balance, uint8_t source,
            long recvTime);

    void OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, int requestId,
            int errorId, const char* errorMsg, uint8_t source,
            long recvTime);

    void OnRspQryTicker(const DCSSTickerField* rsp, int requestId, int errorId,
            const char* errorMsg, uint8_t source, long recvTime);

    void OnRspQryKline(const DCSSKlineField* kline, int requestId, int errorId,
            const char* errorMsg, uint8_t source, long recvTime);

    void OnRspQryOrder(const DCSSOrderField* rsp, int requestId, int errorId,
            const char* errorMsg, uint8_t source, long recvTime);

    void OnRspQrySymbol(const DCSSSymbolField* rsp, int requestId, int errorId,
            const char* errorMsg, uint8_t source, long recvTime);

    void OnRspQryTradingAccount(const DCSSTradingAccountField* account,
            int requestId, int errorId, const char* errorMsg, uint8_t source,
            long recvTime);

    void OnTime(long curTime);

    std::string GetName() const;

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
    void SetMdNano(long curTime) { mMdNano = curTime;}

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

    IDCSSStrategy* mStrategy;

private:

    long mCurNano;

    long mMdNano;

    struct Balance
    {
        double Free;
        double Freezed;

        Balance() : Free(0.0), Freezed(0.0) {}
    };

    /*map<source, map<currency, {free, frozen}> >*/
    std::unordered_map<uint8_t, std::unordered_map<std::string, Balance> > mBalance;

    std::unordered_set<uint8_t> mSourceSet;

    std::string mConfigPath;

    std::string mConfig;

    int mWaitingRid;

    std::mutex mMutex;

    std::condition_variable mCondVar;

    void* mQryResult;

    char* mErrorMsg;
};

#endif /* STRATEGY_DCSSSTRATEGYIMPL_H_ */
