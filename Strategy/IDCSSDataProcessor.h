//
// Created by wangzhen on 18-6-22.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_IDATAPROCESSOR_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_IDATAPROCESSOR_H

#include "DataStruct.h"
#include "Declare.h"

class IDCSSDataProcessor
{
public:
    virtual void OnRtnTicker(const DCSSTickerField* ticker, uint8_t source, long recvTime) = 0;

    virtual void OnRtnKline(const DCSSKlineHeaderField* header,
            const std::vector<const DCSSKlineField* >& kline,
            uint8_t source,
            long recvTime) = 0;

    virtual void OnRtnDepth(const DCSSDepthHeaderField* header,
            const std::vector<const DCSSDepthField* >& ask,
            const std::vector<const DCSSDepthField* >& bid,
            uint8_t source,
            long recvTime) = 0;

    virtual void OnRtnOrder(const DCSSOrderField* order, uint8_t source, long recvTime) = 0;

    virtual void OnRtnBalance(const DCSSBalanceField* balance, uint8_t source, long recvTime) = 0;

    virtual void OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) = 0;

    virtual void OnRspQryTicker(const DCSSTickerField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) = 0;

    virtual void OnRspQryKline(const DCSSKlineHeaderField* header, const std::vector<const DCSSKlineField* >& kline,
            int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) = 0;

    virtual void OnRspQryOrder(const DCSSOrderField* rsp, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) = 0;

    virtual void OnRspQryTradingAccount(const DCSSTradingAccountField* account, int requestId, int errorId, const char* errorMsg, uint8_t source, long recvTime) = 0;
    /*for log*/
    virtual void Debug(const char* msg) = 0;

    virtual void OnTime(long curTime) = 0;

    virtual std::string GetName() const = 0;

    static volatile int mSingalReceived;

    static void SignalHandler(int sig) {mSingalReceived = sig;}

};

DECLARE_PTR(IDCSSDataProcessor);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_IDATAPROCESSOR_H
