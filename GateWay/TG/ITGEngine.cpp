//
// Created by wangzhen on 18-6-13.
//

#include <thread>
#include <csignal>
#include "ITGEngine.h"
#include "OKEX/OKTGApi.h"
#include "util.h"
#include "SysMessages.h"

ITGEngine::ITGEngine()
: mDefaultAccountIndex(-1), mName("TG"), mCurTime(0)
{
    mReader = UnitReader::CreateRevisableReader(mName);
    mLogger = DCSSLog::GetLogger(mName);
}

void ITGEngine::SetReaderThread()
{
    mReaderThread.reset(new std::thread(std::bind(&ITGEngine::Listening, this)));
    DCSS_LOG_INFO(mLogger, "reader thread is setted");
}

bool ITGEngine::RegisterClient(const std::string& name, const std::string& request)
{
    if (mSpiMap.count(name) > 0)
    {
        DCSS_LOG_ERROR(mLogger, "client already exists... (client)" << name);
        return false;
    }
    else
    {
        ITGSpiPtr spiPtr(new ITGSpi(name, mLogger, mProxy));
        if (spiPtr->Load(request))
        {
            spiPtr->SetReaderThread();
            mSpiMap[name] = std::move(spiPtr);
            return true;
        }
        else
        {
            DCSS_LOG_ERROR(mLogger, "load tg for (client)" << name << " failed!");
            return false;
        }
    }
}

bool ITGEngine::RemoveClient(const std::string& name)
{
    if (mSpiMap.count(name) > 0)
    {
        auto ptr = mSpiMap.at(name);
        mSpiMap.erase(name);
        ptr->ForceStop();
        return true;
    }
    return false;
}

void ITGEngine::Listening()
{
    if (mReader.get() == nullptr)
    {
        throw std::runtime_error("reader is not inited! please call init() before start()");
    }

    FramePtr frame;
    while (mIsRunning && SignalReceived < 0)
    {
        frame = mReader->GetNextFrame();
        if (frame.get() != nullptr)
        {
            mCurTime = frame->GetNano();
            FH_MSG_TP_TYPE msgType = frame->GetMsgType();

            if (msgType < 200)
            {
                if (msgType == MSG_TYPE_TRADE_ENGINE_LOGIN)
                {
                    try
                    {
                        std::string content((char*) frame->GetData());
                        nlohmann::json j_request = nlohmann::json::parse(content);
                        std::string clientName = j_request.at("name");
                        if (RegisterClient(clientName, j_request.at("config")))
                            DCSS_LOG_INFO(mLogger, "[user] Accecpted: " << clientName);
                        else
                            DCSS_LOG_INFO(mLogger, "[user] Rejected: " << clientName);
                    }
                    catch (...)
                    {
                        DCSS_LOG_ERROR(mLogger, "error in parsing TRADE_ENGINE_LOGIN: " << (char*) frame->GetData());
                    }
                }
                else if (msgType == MSG_TYPE_STRATEGY_END)
                {
                    try
                    {
                        std::string content((char*) frame->GetData());
                        nlohmann::json j_request = nlohmann::json::parse(content);
                        std::string clientName = j_request.at("name");
                        if (RemoveClient(clientName))
                            DCSS_LOG_INFO(mLogger, "[user] Removed: " << clientName);
                    }
                    catch (...)
                    {
                        DCSS_LOG_ERROR(mLogger, "error in parsing STRATEGY_END: " << (char*) frame->GetData());
                    }
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (SignalReceived >= 0)
    {
        DCSS_LOG_INFO(mLogger, "[IEngine] signal received: " << SignalReceived);
        for (auto& item : mSpiMap)
            item.second->SetSignal(SignalReceived);
    }

    if (!mIsRunning)
    {
        DCSS_LOG_INFO(mLogger, "[IEngine] forced to stop.");
        for (auto& item : mSpiMap)
            item.second->ForceStop();
    }
}
