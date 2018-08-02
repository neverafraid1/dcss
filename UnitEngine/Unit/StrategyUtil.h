//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYUTIL_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYUTIL_H

#include "IStrategyUtil.h"
#include "UnitWriter.h"

UNIT_NAMESPACE_START
PRE_DECLARE_PTR(StrategyUtil);
PRE_DECLARE_PTR(StrategySocketHandler);

/**
 * wrapper of strategy utilities for dcss strategy
 */
class StrategyUtil : public IStrategyUtil
{
public:
    /*override IStrategyUtil's func*/
    bool TdConnect(uint8_t source) override ;

    bool MdSubscribeTicker(const std::string& tickers, uint8_t source) override ;

    bool MdSubscribeKline(const std::string& symbol, char klineType, uint8_t source) override;

    bool MdSubscribeDepth(const std::string& symbol, int depth, uint8_t source) override;

    bool RegisterStrategy(int& ridStart, int& ridEnd) override ;

public:
    explicit StrategyUtil(const std::string& strategyName);

    ~StrategyUtil() override ;

    static StrategyUtilPtr Create(const std::string& strategyName);

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

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYUTIL_H
