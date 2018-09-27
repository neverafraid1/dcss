//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_IENGINE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_IENGINE_H

#include "UnitDeclare.h"

UNIT_NAMESPACE_START
PRE_DECLARE_PTR(UnitReader);
PRE_DECLARE_PTR(DCSSLog);
UNIT_NAMESPACE_END

USING_UNIT_NAMESPACE

/**
 * base class of all engines
 * such as TGEngine/MGEngine
 */
class IEngine
{
public:
//    /** pure virtual functions */
    virtual void SetReaderThread() = 0;
//    /** init writer,reader etc... */
//    virtual void Init() = 0;
//
//    virtual void Load() = 0;
//
//    /** connect to front */
//    virtual void Connect(const std::string& client) = 0;
//    /** disconnect to front */
//    virtual void Disconnect(const std::string& client) = 0;
//    /** release api */
//    virtual void ReleaseApi() = 0;
//    /** return true if engine is connected to front */
//    virtual bool IsConnected(const std::string& client) const = 0;

    DCSSLogPtr GetLogger() const
    {
        return mLogger;
    }

    virtual std::string Name() = 0;

public:
    /*initialize engine, pass-in parameters as json format*/
//    void Initialize();
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

protected:
    UnitReaderPtr mReader;
    /*dcss logger*/
    DCSSLogPtr mLogger;
    /*flag of reader thread*/
    volatile bool mIsRunning;
    /*reader thread*/
    ThreadPtr mReaderThread;

    std::string mProxy;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_IENGINE_H
