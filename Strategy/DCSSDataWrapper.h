//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_DATAWRAPPER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_DATAWRAPPER_H

#include <unordered_map>
#include <unordered_set>
#include "Declare.h"
#include "UnitReader.h"
#include "IDCSSDataProcessor.h"
#include "DCSSStrategyUtil.h"
#include "Timer.h"

class DCSSDataWrapper
{
public:
    /*setup reader*/
    void PreRun();
    /*request td connect, default timeout is 10s*/
    void Connect(const std::string& config, long time = 5 * NANOSECONDS_PER_SECOND);

public:
    DCSSDataWrapper(IDCSSDataProcessor* processor, DCSSStrategyUtil* util);

    void AddRegisterTd(uint8_t source);

    void AddMarketData(uint8_t source);

    void AddTicker(const std::string& symbol, uint8_t source);

    void AddKline(const std::string& symbol, KlineTypeType klineType, uint8_t source);

    void AddDepth(const std::string& symbol, int depth, uint8_t source);

    /*looping*/
    void Run();
    /*Stop*/
    void Stop();
    /*get td status*/
    uint8_t GetTdStatus(uint8_t source);

    bool IsAllLogined();

protected:

    void ProcessTdAck(const std::string& content, uint8_t source, long recvTime);

    volatile bool mForceStop;

    std::vector<std::string> mFolders;
    std::vector<std::string> mNames;
    IDCSSDataProcessor*     mProcessor;
    DCSSStrategyUtil*       mUtil;
    UnitReaderPtr           mReader;

    std::map<uint8_t, uint8_t>    mTdStatus;
    std::unordered_map<std::string, double> mLastPriceMap;

    int mRidStart;
    int mRidEnd;
    long mCurTime;

private:
    std::unordered_map<uint8_t, std::unordered_set<std::string> > mSubedTicker;
    std::unordered_map<uint8_t, std::unordered_map<std::string, std::unordered_set<KlineTypeType> > > mSubedKline;
    std::unordered_map<uint8_t, std::unordered_map<std::string, std::unordered_set<int> > > mSubedDepth;
};

DECLARE_PTR(DCSSDataWrapper);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_DATAWRAPPER_H
