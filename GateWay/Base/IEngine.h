//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_IENGINE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_IENGINE_H

#include "UnitReader.h"
#include "UnitWriter.h"
#include "DCSSLog.h"

using namespace DCSS;

class IEngine
{
public:
    /** pure virtual functions */
    virtual void SetReaderThread() = 0;
    /** init writer,reader etc... */
    virtual void Init() = 0;

    virtual void Load() = 0;

    /** connect to front */
    virtual void Connect() = 0;
    /** release api */
    virtual void ReleaseApi() = 0;
    /** return true if engine is connected to front */
    virtual bool IsConnected() const = 0;
    /** get this engine's name */
    virtual std::string Name() const = 0;
public:
    void Initialize();

    bool Start();

    bool Stop();


protected:
    IEngine();

    virtual ~IEngine();

    static volatile int SignalReceived;

    static void SignalHandler(int signum)
    {
        SignalReceived = signum;
    }
protected:
    UnitWriterPtr mWriter;
    UnitReaderPtr mReader;
    DCSSLogPtr mLogger;
    volatile bool mIsRunning;
    ThreadPtr mReaderThread;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_IENGINE_H
