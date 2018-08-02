//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_IENGINE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_IENGINE_H

#include "UnitReader.h"
#include "UnitWriter.h"
#include "DCSSLog.h"
#include "json.hpp"
#include "UnitDeclare.h"
#include "SysMessages.h"

USING_UNIT_NAMESPACE

/**
 * base class of all engines
 * such as TGEngine/MGEngine
 */
class IEngine
{
public:
    /** pure virtual functions */
    virtual void SetReaderThread() = 0;
    /** init writer,reader etc... */
    virtual void Init() = 0;

    virtual void Load(const nlohmann::json& config) = 0;

    /** connect to front */
    virtual void Connect() = 0;
    /** disconnect to front */
    virtual void Disconnect() = 0;
    /** release api */
    virtual void ReleaseApi() = 0;
    /** return true if engine is connected to front */
    virtual bool IsConnected() const = 0;

    DCSSLogPtr GetLogger() const
    {
        return mLogger;
    }

public:
    /*initialize engine, pass-in parameters as json format*/
    void Initialize(const std::string& jsonStr);
    /*start engine*/
    bool Start();
    /*stop all threads*/
    bool Stop();
    /*block main thread*/
    void WaitForStop();


protected:
    IEngine();

    virtual ~IEngine();

    static volatile int SignalReceived;

    static void SignalHandler(int signum)
    {
        SignalReceived = signum;
    }

    inline void WriteEngineStatus(const std::string& status, uint8_t source)
    {
        if (mWriter.get() != nullptr)
            mWriter->WriteFrame(status.c_str(), status.length(), source, MSG_TYPE_ENGINE_STATUS, getpid());
    }
protected:
    /*map: <clientname, writer>*/
    UnitWriterPtr mWriter;
    /*reader*/
    UnitReaderPtr mReader;
    /*dcss logger*/
    DCSSLogPtr mLogger;
    /*flag of reader thread*/
    volatile bool mIsRunning;
    /*reader thread*/
    ThreadPtr mReaderThread;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_IENGINE_H
