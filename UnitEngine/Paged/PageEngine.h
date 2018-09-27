//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGEENGINE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGEENGINE_H

#include "PageSocketHandler.h"
#include "PageCommStruct.h"
#include "DCSSLog.h"
#include "PageServiceTask.h"

#include <utility>
#include <thread>

UNIT_NAMESPACE_START

PRE_DECLARE_PTR(UnitWriter);

/**
 * we call each unit handler(writer or reader) a client for page engine
 * we call each unit linked by client a user of page engine
 * client == handler(writer or reader)
 * user == unit
 * so:
 * each writer client may have only 1 user
 * each reader client may have several users
 * all the necessary information are stored here
 */
struct PageClientInfo
{
    /*the index of each user linked by this client
     * when is a writer, size==1*/
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
};


class PageEngine : public IPageSocketUtil
{
    friend class PstPidCheck;
    friend class PstTimeTick;
    friend class PstTempPage;
public:
    PageEngine();

    virtual ~PageEngine() = default;

    void Start();

    void Stop();

    void SetFreq(double secondInterVal);

    bool AddTask(PstBasePtr task);

    bool RemoveTask(PstBasePtr task);

    bool RemoveTaskByName(const std::string& name);
    /* write string content to system unit
     * return true if msg is written in system unit*/
    bool Write(const std::string& content, short msgType, bool isLast = true, uint8_t source = 0);

public:
    /** functions requested by IPageSocketUtil */
    DCSSLogPtr GetLogger() const override { return mLogger;}
    int RegUnit(const std::string& clientName) override ;
    IntPair RegStrategy(const std::string& strategyName) override ;
    bool RegClient(std::string& commFile, size_t& fileSize, const std::string& clientName, int pid, bool isWriter) override ;
    void ExitClient(const std::string& clientName) override ;
    bool LoginTd(const std::string& clientName, std::string config) override ;
    bool SubTicker(const std::string& symbol, uint8_t source) override ;
    bool SubKline(const std::string& symbol, int klineType, uint8_t source) override ;
    bool SubDepth(const std::string& symbol, uint8_t source) override ;
    bool UnSubTicker(const std::string& symbol, uint8_t source) override ;
    bool UnSubKline(const std::string& symbol, int klineType, uint8_t source) override ;
    bool UnSubDepth(const std::string& symbol, uint8_t source) override ;
    void AcquireMutex() const override ;
    void ReleaseMutex() const override ;

private:
    /*comm = communicate*/
    /*writer for system unit*/
    UnitWriterPtr mWriter;
    /*logger*/
    DCSSLogPtr mLogger;
    /*communicate memory*/
    void* mCommBuffer;
    /*communicate file linked to memory*/
    std::string mCommFile;
    /*task frequency in microseconds*/
    int mMicroSecFreq;
    /*whether task thread is running*/
    bool mTaskRunning;
    /*whether communicate mBuffer checking thread is running*/
    volatile bool mCommRunning;
    /*max index of current assigned comm block*/
    size_t mMaxIdx;

    /*thread for task running*/
    ThreadPtr mTaskThread;
    /*thread for communicate mBuffer checking*/
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
    /*map : <file, page mBuffer>*/
    std::map<std::string, void*> mFildAddrs;
    /*map : <task name; task ptr>*/
    std::map<std::string, PstBasePtr> mTasks;
};

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGEENGINE_H
