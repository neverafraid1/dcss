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
    virtual void OnRtnTicker(const DCSSTickerField& ticker, short source, long recvTime) = 0;
//    virtual void OnRtnDepth(const DCSSDepthHeaderField& header,
//            const std::vector<DCSSDepthField>& ask,
//            const std::vector<DCSSDepthField>& bid,
//            long recvTime) = 0;
    virtual void OnRtnKline(const DCSSKlineHeaderField& header,
            const std::vector<DCSSKlineField>& kline,
            short source,
            long recvTime) = 0;

    virtual void OnRtnDepth(const DCSSDepthHeaderField& header,
            const std::vector<DCSSDepthField>& ask,
            const std::vector<DCSSDepthField>& bid,
            short source,
            long recvTime) = 0;

    virtual void OnRtnOrder(const DCSSOrderField& order, int requestID, short source, long recvTime) = 0;

    virtual void OnRspOrderInsert(const DCSSRspInsertOrderField& rsp, int requestID, short source, long recvTime) = 0;
    /*for log*/
    virtual void Debug(const char* msg) = 0;

    virtual void OnTime(long curTime) = 0;

    virtual std::string GetName() const = 0;

    static volatile int mSingalReceived;

    static void SignalHandler(int sig) {mSingalReceived = sig;}
};

DECLARE_PTR(IDCSSDataProcessor);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_IDATAPROCESSOR_H
