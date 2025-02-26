//
// Created by wangzhen on 18-6-20.
//

#include <unistd.h>
#include <csignal>
#include "IEngine.h"
#include "UnitReader.h"
#include "DCSSLog.h"

volatile int IEngine::SignalReceived = -1;

IEngine::IEngine():mIsRunning(false)
{
}

IEngine::~IEngine()
{
    Stop();
}

//void IEngine::Initialize()
//{
//    Init();
//
//    mLogger = DCSSLog::GetLogger(Name());
//
//    Load();
//}

bool IEngine::Start()
{
    mIsRunning = false;
    if (true)
    {
        mIsRunning = true;

        std::signal(SIGTERM, IEngine::SignalHandler);
        std::signal(SIGINT, IEngine::SignalHandler);
        std::signal(SIGHUP, IEngine::SignalHandler);
        std::signal(SIGQUIT, IEngine::SignalHandler);

        SetReaderThread();
    }

    return mIsRunning;
}

bool IEngine::Stop()
{
    mIsRunning = false;

    if (mReaderThread != nullptr)
    {
        mReaderThread->join();
        mReaderThread.reset();
        DCSS_LOG_INFO(mLogger, "reader thread expired");
        return true;
    }

    return false;
}

void IEngine::WaitForStop()
{
    if (mReaderThread != nullptr)
    {
        mReaderThread->join();
        mReaderThread.reset();
    }
}

