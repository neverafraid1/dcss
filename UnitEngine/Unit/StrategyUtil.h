//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYUTIL_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYUTIL_H

#include "IStrategyUtil.h"
#include "UnitWriter.h"
#include "StrategySocketHandler.h"

#define STRATEGY_LOG_FOLDER "/shared/dcss/log/"

class StrategyUtil;
DECLARE_PTR(StrategyUtil);

/**
 * wrapper of strategy utilities
 */
class StrategyUtil : public IStrategyUtil
{
public:
    /*override IStrategyUtil's func*/
    bool TdConnect(short source) override ;

    bool MdSubscribe(const std::vector<DCSSSymbolField>& tickers, short source) override ;

    bool MdSubscribeKline(const DCSSSymbolField& symbol, KlineTypeType klineType, short source) override ;

    bool MdSubscribeDepth(const DCSSSymbolField& symbol, int depth, short source) override ;

    bool RegisterStrategy(int& ridStart, int& ridEnd) override ;

public:
    explicit StrategyUtil(const std::string& strategyName);

    ~StrategyUtil() override ;

    static StrategyUtilPtr Create(const std::string& strategyName);

    bool SubscribeTicker(const std::vector<std::string>& tickers);

    long WriteFrame(const void* data, FH_LENGTH_TYPE length,
    FH_SOURCE_TYPE source, FH_MSG_TP_TYPE msgType,
    FH_REQID_TYPE requestId);

    long WriteFrameExtra(const void* data, FH_LENGTH_TYPE length,
    FH_SOURCE_TYPE source, FH_MSG_TP_TYPE msgType,
    FH_REQID_TYPE requestId, FH_NANO_TYPE extraNano);

    int GetRid();

    IntPair GetRidRange() const;

protected:
    /*init, register strategy to page engine*/
    void Init();

protected:
    /*start rid, allocated by page service*/
    int mRidStart;
    /*end rid, allocated by page service*/
    int mRidEnd;
    /*curr rid*/
    int mCurRid;

private:
    /*handler for socket*/
    StrategySocketHandlerPtr mHandler;
    /*writer*/
    UnitWriterPtr mWriter;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYUTIL_H
