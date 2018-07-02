//
// Created by wangzhen on 18-6-28.
//

#include "DCSSDataWrapper.h"

DCSSDataWrapper::DCSSDataWrapper(IDCSSDataProcessor* processor, DCSSStrategyUtil* util)
: mProcessor(processor), mUtil(util), mForceStop(false)
{
    auto rids = mUtil->GetRidRange();
    mRidStart = rids.first;
    mRidEnd = rids.second;
    mCurTime = MemoryBuffer::GetNanoTime();

    mProcessor->OnTime(mCurTime);
}

void DCSSDataWrapper::PreRun()
{
    mReader = UnitReader::CreateReaderWithSys(mFolders, mNames, MemoryBuffer::GetNanoTime(), mProcessor->GetName());

    for (auto& it : mTdStatus)
    {
        mUtil->TdConnect(it.first);
        it.second = CONNECT_TD_STATUS_REQUESTED;
    }
}

void DCSSDataWrapper::Stop()
{
    mForceStop = true;
}

void DCSSDataWrapper::Run()
{
    PreRun();

    FramePtr frame;
    mForceStop = false;
    mProcessor->mSingalReceived = -1;


    while (!mForceStop && mProcessor->mSingalReceived <= 0)
    {
        frame = mReader->GetNextFrame();
        if (frame.get() != nullptr)
        {
            //TODO
        }
        else
        {
            mCurTime = MemoryBuffer::GetNanoTime();
        }
        mProcessor->OnTime(mCurTime);
    }

    if (mProcessor->mSingalReceived > 0)
    {
    }

    if (mForceStop)
    {

    }
}