//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGEENGINE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGEENGINE_H

#include "PageSocketHandler.h"
#include "PageCommStruct.h"
#include "UnitWriter.h"
#include "DCSSLog.h"
#include "PageServiceTask.h"

#include <utility>
#include <thread>

struct PageClientInfo
{
    /*the index of each user linked by this client*/
    std::vector<int> UserIndexVec;
    /*register nano time*/
    long    RegNano;
    /*process id*/
    int     Pid;
    /*true if this client is a writer*/
    bool    IsWriter;
    /*true if this writer is associated with a strategy*/
    bool    IsStrategy;
    /*start req id of this strategy (only for strategy)*/
    int     RidStart;
    /*end req if of this strategy (only for strategy)*/
    int     RidEnd;
    /*all sources of trade engine that registered (only for strategy)*/
    std::vector<short> TradeEngineVec;
};


class PageEngine : public IPageSocketUtil, public std::enable_shared_from_this<PageEngine>
{
    friend class PstPidCheck;
    friend class PstTimeTick;
    friend class PstTempPage;
public:
    PageEngine();

    virtual ~PageEngine();

    void Start();

    void Stop();

    void SetFreq(double secondInterVal);

    bool AddTask(PstBasePtr task);

    bool RemoveTask(PstBasePtr task);

    bool RemoveTaskByName(const std::string& name);
    /* write string content to system unit
     * return true if msg is written in system unit*/
    bool Write(const std::string& content, uint8_t msgType, bool isLast = true, short source = 0);

public:
    /** functions requested by IPageSocketUtil */
    DCSSLogPtr GetLogger() const { return mLogger;}
    int RegUnit(const std::string& clientName);
    virtual IntPair RegStrategy(const std::string& strategyName);
    virtual bool RegClient();
    virtual void ExitClient(const std::string& clientName);
    virtual bool LoginTd(const std::string& clientName, short source);
    virtual bool SubTicker(const std::vector<std::string>& tickers, short source, short msgType, bool isLast);
    virtual void AcquireMutex() const;
    virtual void ReleaseMutex() const;

private:
    /*comm = communicate*/
    /*writer for system unit*/
    UnitWriterPtr mWriter;
    /*ogger*/
    DCSSLogPtr mLogger;
    /*communicate memory*/
    void* mCommBuffer;
    /*communicate file linked to memory*/
    std::string mCommFile;
    /*task frequency in microseconds*/
    int mMicroSecFreq;
    /*whether task thread is running*/
    bool mTaskRunning;
    /*whether communicate buffer checking thread is running*/
    volatile bool mCommRunning;
    /*max index of current assigned comm block*/
    size_t mMaxIdx;

    /*thread for task running*/
    ThreadPtr mTaskThread;
    /*thread for communicate buffer checking*/
    ThreadPtr mCommThread;
    /*thread for socket listening*/
    ThreadPtr mSocketThread;

private:
    /*start several thread*/
    /*check communicate memory*/
    void StartComm();
    /*socket listening*/
    void StartSocket();
    /*start all tasks*/
    void StartTask();

    /*release the page assigned in comm msg*/
    void ReleasePage(const PageCommMsg& msg);
    /*initialize the page assigned in comm msg*/
    uint8_t InitiatePage(const PageCommMsg& msg);

private:
    /*map : <client name, info>*/
    std::map<std::string, PageClientInfo> mClientUnits;
    /*map : file attached with number of writers*/
    std::map<PageCommMsg, int> mFileWriterCounts;
    /*map : file attached with number of readers*/
    std::map<PageCommMsg, int> mFileReaderCounts;
    /*map : <pid, client>*/
    std::map<int, std::vector<std::string> > mPidClientMap;
    /*map : <file, page buffer>*/
    std::map<std::string, void*> mFildAddrs;
    /*map : <task name; task ptr>*/
    std::map<std::string, PstBasePtr> mTasks;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGEENGINE_H
