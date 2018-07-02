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
    virtual void OnRtnTicker(const DCSSTickerField& ticker, long recvTime) = 0;
    virtual void OnRtnDepth(const DCSSDepthHeaderField& header,
            const std::vector<DCSSDepthField>& ask,
            const std::vector<DCSSDepthField>& bid,
            long recvTime) = 0;
    virtual void OnRtnKline(const DCSSKlineHeaderField& header,
            const std::vector<DCSSKlineField>& kline,
            long recvTime) = 0;

    virtual void OnRspOrderInsert(const DCSSRspInsertOrderField& rsp,
    int requestId, long recvTime, short errorId = 0, const char* errorMsg = nullptr) = 0;

    virtual void OnTime(long curTime) = 0;


    virtual std::string GetName() const = 0;

    static volatile int mSingalReceived;
    static void SignalHandler(int sig) {mSingalReceived = sig;}
};

DECLARE_PTR(IDCSSDataProcessor);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_IDATAPROCESSOR_H
