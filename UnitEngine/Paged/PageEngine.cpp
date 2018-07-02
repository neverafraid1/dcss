//
// Created by wangzhen on 18-6-26.
//

#include "PageEngine.h"
#include "Page.h"
#include "Timer.h"
#include "PageUtil.h"
#include "json.hpp"
#include "SysMessages.h"

#include <mutex>
#include <csignal>
#include <iostream>
#include <unistd.h>

using json = nlohmann::json;

std::mutex gPagedMutex;

const int SLEEP_MILLISEC =1000000;

DCSSLogPtr gStaticLogger;

void SignalCallback(int sig)
{
    DCSS_LOG_INFO(gStaticLogger, "PageEngine Caught signal : " << sig);
    exit(sig);
}

void PageEngine::AcquireMutex() const
{
    gPagedMutex.lock();
}

void PageEngine::ReleaseMutex() const
{
    gPagedMutex.unlock();
}

PageEngine::PageEngine()
{
    mLogger = DCSSLog::GetLogger("PageEngine");
    gStaticLogger = mLogger;

    for (int s = 1; s < 32; ++s)
    {
        signal(s, SignalCallback);
    }

    mTasks.clear();
    AddTask(PstBasePtr(new PstPidCheck(this)));
}

PageEngine::~PageEngine()
{ }

bool PageEngine::Write(const std::string& content, uint8_t msgType, bool isLast, short source)
{
    if (mWriter.get() == nullptr)
        return false;
    mWriter->WriteFrame(content.c_str(), content.length() + 1, source, msgType, -1);
    return true;
}

void PageEngine::Start()
{
    DCSS_LOG_INFO(mLogger, "reset socket: " << PAGED_SOCKET_FILE);
    remove(PAGED_SOCKET_FILE);

    /*step 0: init communicate buffer*/
    DCSS_LOG_INFO(mLogger, "loading page buffer: " << mCommFile);
    mCommBuffer = PageUtil::LoadPageBuffer(mCommFile, COMM_SIZE, true, true);
    memset(mCommBuffer, 0, COMM_SIZE);

    /*step 1: start communicate buffer check thread*/
    mCommRunning = false;
    mCommThread = ThreadPtr(new std::thread(std::bind(&PageEngine::StartComm, this)));

    /*step 2: start listening socket*/
    mSocketThread = ThreadPtr(new std::thread(std::bind(&PageEngine::StartSocket, this)));

    /*wait until buffer/socket thread are running*/
    while (!(PageSocketHandler::GetInstance()->IsRunning()) && mCommRunning)
    {
        std::this_thread::sleep_for(std::chrono::microseconds(SLEEP_MILLISEC / 10));
    }
    DCSS_LOG_INFO(mLogger, "creating writer: (Folder)" << PAGED_UNIT_FOLDER << "(Name)" << PAGED_UNIT_NAME);
    mWriter = UnitWriter::Create(PAGED_UNIT_FOLDER, PAGED_UNIT_NAME, "paged");

    if (mMicroSecFreq <= 0)
        throw std::runtime_error("unaccepted task time interval");

    mTaskRunning = true;
    mTaskThread = ThreadPtr(new std::thread(std::bind(&PageEngine::StartTask, this)));
}

void PageEngine::SetFreq(double secondInterVal)
{
    mMicroSecFreq = (int)(secondInterVal * MICROSECONDS_PER_SECOND);
    DCSS_LOG_INFO(mLogger, "microsecond frequency update to " << mMicroSecFreq);
}

void PageEngine::Stop()
{
    DCSS_LOG_INFO(mLogger, "(stop) try...");

    mWriter.reset();

    /*stop task thread first*/
    mTaskRunning = false;
    if (mTaskThread.get() != nullptr)
    {
        mTaskThread->join();
        mTaskThread.reset();
    }
    DCSS_LOG_INFO(mLogger, "(stop task) done");

    /*stop comm thread*/
    mCommRunning = false;
    if (mCommThread.get() != nullptr)
    {
        mCommThread->join();
        mCommThread.reset();
    }
    DCSS_LOG_INFO(mLogger, "(stop comm) done");

    /*stop socket io thread*/
    PageSocketHandler::GetInstance()->Stop();
    if (mSocketThread.get() != nullptr)
    {
        mSocketThread->join();
        mSocketThread.reset();
    }
    DCSS_LOG_INFO(mLogger, "(stop socket) done");

    DCSS_LOG_INFO(mLogger, "(stop) done");
}

void PageEngine::StartTask()
{
    DCSS_LOG_INFO(mLogger, "(start task) (microseconds)" << mMicroSecFreq);
    while (mTaskRunning)
    {
        AcquireMutex();
        for (auto item : mTasks)
        {
            item.second->Go();
        }
        ReleaseMutex();
        std::this_thread::sleep_for(std::chrono::microseconds(mMicroSecFreq));
    }
}

bool PageEngine::AddTask(PstBasePtr task)
{
    AcquireMutex();
    std::string name = task->GetName();
    bool exist = (mTasks.find(name) != mTasks.end());
    mTasks[name] = task;
    DCSS_LOG_INFO(mLogger, "(add task) (Name)" << name << " (exist)" << (int)exist);
    ReleaseMutex();
    return !exist;
}

bool PageEngine::RemoveTask(PstBasePtr task)
{
    std::string name = task->GetName();
    return RemoveTaskByName(name);
}

bool PageEngine::RemoveTaskByName(const std::string& name)
{
    AcquireMutex();
    auto iter = mTasks.find(name);
    if (iter == mTasks.end())
    {
        ReleaseMutex();
        return false;
    }
    mTasks.erase(iter);
    DCSS_LOG_INFO(mLogger, "(rm task) (Name)" << name);
    ReleaseMutex();
    return true;
}

void PageEngine::StartSocket()
{
    PageSocketHandler::GetInstance()->Run(shared_from_this());
}

int PageEngine::RegUnit(const std::string& clientName)
{
    size_t idx = 0;
    for (; idx < MAX_COMM_USER_NUMBER; ++idx)
    {
        if (GET_COMM_MSG(mCommBuffer, idx)->Status == PAGED_COMM_RAW)
            break;
    }

    if (idx >= MAX_COMM_USER_NUMBER)
    {
        DCSS_LOG_ERROR(mLogger, "cannot allocate idx in comm file");
        return -1;
    }

    if (idx > mMaxIdx)
        mMaxIdx = idx;

    PageCommMsg* msg = GET_COMM_MSG(mCommBuffer, idx);
    msg->Status = PAGED_COMM_OCCUPIED;
    msg->LastPageNum = 0;

    auto it = mClientUnits.find(clientName);
    if (it == mClientUnits.end())
    {
        DCSS_LOG_ERROR(mLogger, "cannot find the client in reg unit (client)" << clientName);
        return -1;
    }
    DCSS_LOG_INFO(mLogger, "[RegUnit] (client)" << clientName << " (idx)" << idx);
    return idx;
}

void PageEngine::ReleasePage(const PageCommMsg& msg)
{
    DCSS_LOG_INFO(mLogger, "[release page] (folder)" << msg.Folder
                                                     << " (name)" << msg.Name << " (page num)" << msg.PageNum
                                                     << " (last page num)" << msg.LastPageNum);

    std::map<PageCommMsg, int>::iterator countIt;

    if (msg.IsWriter)
    {
        countIt = mFileWriterCounts.find(msg);
        if (countIt == mFileWriterCounts.end())
        {
            DCSS_LOG_ERROR(mLogger, "cannot find key at mFileWriterCounts in exit_client");
            return;
        }
    }
    else
    {
        countIt = mFileReaderCounts.find(msg);
        if (countIt == mFileReaderCounts.end())
        {
            DCSS_LOG_ERROR(mLogger, "cannot find key at mFileReaderCounts in exit_client");
            return;
        }
    }

    --countIt->second;
    if (countIt->second == 0)
    {
        bool otherSideIsEmpty = false;
        if (msg.IsWriter)
        {
            mFileWriterCounts.erase(countIt);
            otherSideIsEmpty = mFileReaderCounts.find(msg) == mFileReaderCounts.end();
        }
        else
        {
            mFileReaderCounts.erase(countIt);
            otherSideIsEmpty = mFileWriterCounts.find(msg) == mFileWriterCounts.end();
        }

        if (otherSideIsEmpty)
        {
            std::string path = PageUtil::GenPageFullPath(msg.Folder, msg.Name, msg.PageNum);
            auto fileIt = mFildAddrs.find(path);
            if (fileIt != mFildAddrs.end())
            {
                void * addr = fileIt->second;
                DCSS_LOG_INFO(mLogger, "[addr rm] (path)" << path << " (addr)" << addr);
                PageUtil::ReleasePageBuffer(addr, UNIT_PAGE_SIZE, true);
                mFildAddrs.erase(fileIt);
            }
        }
    }
}

uint8_t PageEngine::InitiatePage(const PageCommMsg& msg)
{
    DCSS_LOG_INFO(mLogger,
            "[init page] (folder)" << msg.Folder << " (uname)" << msg.Name << " (page num)" << msg.PageNum
                                   << " (last page num)" << msg.LastPageNum);

    std::string path = PageUtil::GenPageFullPath(msg.Folder, msg.Name, msg.PageNum);
    if (mFildAddrs.find(path) == mFildAddrs.end())
    {
        void* buffer(nullptr);
        if (!PageUtil::FileExists(path))
        {
            if (!msg.IsWriter)
                return PAGED_COMM_NON_EXIST;
            else
            {
                /*load temp page*/
                auto tempPageIter = mFildAddrs.find(PstTempPage::PageFullPath);
                if (tempPageIter != mFildAddrs.end())
                {
                    if (rename(PstTempPage::PageFullPath.c_str(), path.c_str()) < 0)
                    {
                        DCSS_LOG_ERROR(mLogger, "[init page] ERROR: cannot rename from" << PstTempPage::PageFullPath << " to" << path);
                        return PAGED_COMM_CANNOT_RENAME_FROM_TEMP;
                    }
                    else
                    {
                        DCSS_LOG_INFO(mLogger, "[init page] TEMP POOL: " <<PstTempPage::PageFullPath << " to" << path);
                        buffer = tempPageIter->second;
                        mFildAddrs.erase(tempPageIter);
                    }
                }

                if (buffer == nullptr)
                    buffer = PageUtil::LoadPageBuffer(path, UNIT_PAGE_SIZE, true, true);
            }
        }
        else
        {
            buffer = PageUtil::LoadPageBuffer(path, UNIT_PAGE_SIZE, false, true);
        }
        DCSS_LOG_INFO(mLogger, "[addr add] (path)" << path << " (addr)" << buffer);
        mFildAddrs[path] = buffer;
    }

    if (msg.IsWriter)
    {
        auto countIt = mFileWriterCounts.find(msg);
        if (countIt == mFileWriterCounts.end())
            mFileWriterCounts[msg] = 1;
        else
            return PAGED_COMM_MORE_THAN_ONE_WRITER;
    }
    else
    {
        ++mFileReaderCounts[msg];
    }

    return PAGED_COMM_ALLOCATED;
}

void PageEngine::StartComm()
{
    mCommRunning = true;
    for (size_t idx = 0; mCommRunning; idx = (idx + 1) % (mMaxIdx + 1))
    {
        PageCommMsg* msg = GET_COMM_MSG(mCommBuffer, idx);
        if (msg->Status == PAGED_COMM_REQUESTING)
        {
            /*need new page*/
            AcquireMutex();

            if(msg->LastPageNum > 0 && msg->LastPageNum != msg->PageNum)
            {
                short currPage = msg->PageNum;
                msg->PageNum = msg->LastPageNum;
                ReleasePage(*msg);
                msg->PageNum = currPage;
            }
            msg->Status = InitiatePage(*msg);
            msg->LastPageNum = msg->PageNum;

            ReleaseMutex();
        }
    }
}

IntPair PageEngine::RegStrategy(const std::string& strategyName)
{
    auto it = mClientUnits.find(strategyName);
    if (it == mClientUnits.end())
    {
        DCSS_LOG_ERROR(mLogger, "[ERROR] cannot find client " << strategyName << " in RegStrategy");
        return std::make_pair(-1, -1);
    }

    PageClientInfo& info = it->second;
    int idx = info.UserIndexVec[0];
    info.IsStrategy = true;
    info.RidStart = (idx + 1) * REQUEST_ID_RANGE;
    info.RidEnd = (idx + 2) * REQUEST_ID_RANGE - 1;

    return std::make_pair(info.RidStart, info.RidEnd);
}

bool PageEngine::LoginTd(const std::string& clientName, short source)
{
    DCSS_LOG_INFO(mLogger, "[TGLogin] (name)" << clientName << " (source)" << source);

    auto it = mClientUnits.find(clientName);
    if (it == mClientUnits.end())
    {
        DCSS_LOG_ERROR(mLogger, "[ERROR][TGLogin] this client does not exist! (name)" << clientName);
        return false;
    }
    PageClientInfo& info = it->second;
    if (info.UserIndexVec.size() != 1)
    {
        DCSS_LOG_ERROR(mLogger, "[ERROR][TGLogin] this client is supposed to have only one unit (name)" << clientName);
        return false;
    }
    PageCommMsg* msg = GET_COMM_MSG(mCommBuffer, info.UserIndexVec[0]);
    json request;
    request["name"] = clientName;
    request["folder"] = msg->Folder;
    request["rid_s"] = info.RidStart;
    request["rid_e"] = info.RidEnd;
    request["pid"] = info.Pid;
    Write(request.dump(), MSG_TYPE_TRADE_ENGINE_LOGIN, true, source);
    info.TradeEngineVec.push_back(source);
    return true;
}

bool PageEngine::SubTicker(const std::vector<std::string>& tickers, short source, short msgType, bool isLast)
{
    DCSS_LOG_INFO(mLogger, "(subscribe) (source)" << source << " (msg type)" << )

    bool written = true;
    for (size_t i = 0; i < tickers.size(); ++i)
        written &= Write(tickers[i], msgType, isLast && (i == tickers.size() - 1), source);

    return written;
}

void PageEngine::ExitClient(const std::string& clientName)
{
    auto it = mClientUnits.find(clientName);
    if (it == mClientUnits.end())
        return;

    PageClientInfo& info = it->second;
    if (info.IsStrategy)
    {
        int idx = info.UserIndexVec[0];
        PageCommMsg* msg = GET_COMM_MSG(mCommBuffer, idx);
        json j_request;
        j_request["name"] = clientName;
        j_request["folder"] = msg->Folder;
        j_request["rid_s"] = info.RidStart;
        j_request["rid_e"] = info.RidEnd;
        j_request["pid"] = info.Pid;
        Write(j_request.dump(), MSG_TYPE_STRATEGY_END);
    }

    for (auto idx : info.UserIndexVec)
    {
        PageCommMsg* msg = GET_COMM_MSG(mCommBuffer, idx);
        if (msg->Status == PAGED_COMM_ALLOCATED)
            ReleasePage(*msg);
        msg->Status = PAGED_COMM_RAW;
    }

    DCSS_LOG_INFO(mLogger, "[rm client] (name)" << clientName << " (start)" << info.RegNano << " (end)" << GetNanoTime());
    auto& clients = mPidClientMap[info.Pid];
    clients.erase(remove(clients.begin(), clients.end(), clientName), clients.end());
    if (clients.size() == 0)
        mPidClientMap.erase(info.Pid);

    mClientUnits.erase(it);
}