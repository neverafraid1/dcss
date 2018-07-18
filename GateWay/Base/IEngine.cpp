//
// Created by wangzhen on 18-6-20.
//

#include "IEngine.h"
#include <unistd.h>
#include <csignal>

volatile int IEngine::SignalReceived = -1;

IEngine::IEngine():mIsRunning(false)
{
}

IEngine::~IEngine()
{
    Stop();
}

void IEngine::Initialize(const std::string& jsonStr)
{
    Init();

    nlohmann::json config = nlohmann::json::parse(jsonStr);

    mLogger = DCSSLog::GetLogger(config.at("name"));

    for (nlohmann::json::const_iterator iter = config.begin(); iter != config.end(); ++iter)
    {
        DCSS_LOG_INFO(mLogger, "IEngine::Init " << iter.key() << "->" << iter.value().dump());
    }

    Load(config);
}

bool IEngine::Start()
{
    mIsRunning = false;
    if (true)
    {
        mIsRunning = true;
        if (mReader.get() == nullptr)
        {
            throw std::runtime_error("reader is not inited! please call init() before start()");
        }

        std::signal(SIGTERM, IEngine::SignalHandler);
        std::signal(SIGINT, IEngine::SignalHandler);
        std::signal(SIGHUP, IEngine::SignalHandler);
        std::signal(SIGQUIT, IEngine::SignalHandler);
        std::signal(SIGKILL, IEngine::SignalHandler);

        SetReaderThread();
    }

    return mIsRunning;
}

bool IEngine::Stop()
{
    mIsRunning = false;

    if (mReaderThread.get() != nullptr)
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
    if (mReaderThread.get() != nullptr)
    {
        mReaderThread->join();
        mReaderThread.reset();
    }
}
