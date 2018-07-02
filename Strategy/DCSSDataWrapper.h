//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_DATAWRAPPER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_DATAWRAPPER_H

#include "Declare.h"
#include "UnitReader.h"
#include "IDCSSDataProcessor.h"
#include "DCSSStrategyUtil.h"
#include "Timer.h"

/** unknown, as default value */
#define CONNECT_TD_STATUS_UNKNOWN   0
/** td connect just added */
#define CONNECT_TD_STATUS_ADDED     1
/** td connect requested */
#define CONNECT_TD_STATUS_REQUESTED 2
/** got td ack msg, with valid pos */
#define CONNECT_TD_STATUS_ACK_POS   3
/** got td ack msg, no valid pos */
#define CONNECT_TD_STATUS_ACK_NONE  4
/** got td ack msg, set back */
#define CONNECT_TD_STATUS_SET_BACK  5


class DCSSDataWrapper
{
protected:
    /*setup reader and request td connect*/
    void PreRun();

public:
    DCSSDataWrapper(IDCSSDataProcessor* processor, DCSSStrategyUtil* util);

    void AddRegisterTd(short source);

    void AddMarketData(short source);

    /*looping*/
    void Run();
    /*Stop*/
    void Stop();
    /*get td status*/
    uint8_t GetTdStatus(short source);

protected:
    volatile bool mForceStop;

    std::vector<std::string> mFolders;
    std::vector<std::string> mNames;
    IDCSSDataProcessor*     mProcessor;
    DCSSStrategyUtil*       mUtil;
    UnitReaderPtr           mReader;
    std::map<short, uint8_t>    mTdStatus;

    int mRidStart;
    int mRidEnd;
    long mCurTime;
};

DECLARE_PTR(DCSSDataWrapper);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_DATAWRAPPER_H
