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
    virtual void OnRtnTicker(const DCSSTickerField& ticker, long recvTime);
    virtual void OnRtnDepth(const DCSSDepthHeaderField& header,
            const std::vector<DCSSDepthField>& ask,
            const std::vector<DCSSDepthField>& bid,
            long recvTime);
    virtual void OnRtnKline(const DCSSKlineHeaderField& header,
            const std::vector<DCSSKlineField>& kline,
            long recvTime);

    virtual void OnRspOrderInsert(const DCSSRspInsertOrderField& rsp,
            int requestId, long recvTime, short errorId = 0, const char* errorMsg = nullptr);

    virtual void OnTime(long curTime);


    virtual std::string GetName() const { return  mName; };

public:
    virtual ~IDCSSStrategy();

    IDCSSStrategy(const std::string& name);

    virtual void Start();

    void Run();

    void Terminate();

    void Stop();

    void Block();

protected:
    bool IsTdReady(short source) const;
    bool IsTdConnected(short source) const;
protected:
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
