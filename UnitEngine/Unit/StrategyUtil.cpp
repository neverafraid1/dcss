//
// Created by wangzhen on 18-6-25.
//

#include "StrategyUtil.h"
#include "PageCommStruct.h"

StrategyUtil::StrategyUtil(const std::string& strategyName)
{
    mHandler = std::make_shared(new StrategySocketHandler(strategyName));
    mWriter = UnitWriter::Create(STRATEGY_LOG_FOLDER, strategyName, mHandler);
    Init();
}

StrategyUtilPtr StrategyUtil::Create(const std::string& strategyName)
{
    return std::make_shared(new StrategyUtil(strategyName));
}

StrategyUtil::~StrategyUtil()
{
    mWriter.reset();
    mHandler.reset();
}

void StrategyUtil::Init()
{
    RegisterStrategy(mRidStart, mRidEnd);
    mCurRid = 0;
}

bool StrategyUtil::TdConnect(short source)
{
    if (mHandler.get() != nullptr)
        return mHandler->TdConnect(source);
    else
        return false;
}

IntPair StrategyUtil::GetRidRange() const
{
    return std::make_pair(mRidStart, mRidEnd);
}

bool StrategyUtil::MdSubscribe(const std::vector<DCSSSymbolField>& tickers, short source)
{
    if (mHandler.get() != nullptr)
        return mHandler->MdSubscribe(tickers, source);
    else
        return false;
}

bool StrategyUtil::RegisterStrategy(int& ridStart, int& ridEnd)
{
    if (mHandler.get() != nullptr)
        return mHandler->RegisterStrategy(ridStart, ridEnd);
    else
        return false;
}

long StrategyUtil::WriteFrame(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source, FH_MSG_TP_TYPE msgType,
        FH_REQID_TYPE requestId)
{
    return mWriter->WriteFrame(data, length, source, msgType, requestId);
}

long StrategyUtil::WriteFrameExtra(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source,
        FH_MSG_TP_TYPE msgType, FH_REQID_TYPE requestId, FH_NANO_TYPE extraNano)
{
    return mWriter->WriteFrameExtra(data, length, source, msgType, requestId, extraNano);
}

int StrategyUtil::GetRid()
{
    return (mCurRid++ % REQUEST_ID_RANGE) + mRidStart;
}