//
// Created by wangzhen on 18-6-20.
//

#include "PageProvider.h"
#include "PageCommStruct.h"
#include "PageUtil.h"
#include "StrategySocketHandler.h"
#include "Page.h"
#include <unistd.h>

ClientPageProvider::ClientPageProvider(const std::string& clientName, bool isWriting, bool reviseAllowed)
: mClientName(clientName), mBuffer(nullptr), mIsWriter(isWriting)
{
    mReviseAllowed = mIsWriter || reviseAllowed;
}

void ClientPageProvider::RegisterClient()
{
    PagedSocketRequest req = {};
    req.Type = mIsWriter ? PAGED_SOCKET_WRITER_REGISTER : PAGED_SOCKET_READER_REGISTER;
    req.Pid = getpid();
    SocketArray rspArray;
    GetSocketRspOnReq(req, rspArray, mClientName);

    auto* rsp = reinterpret_cast<PagedSocketRspClient*>(&rspArray[0]);
    if (rsp->Type == req.Type && rsp->Success)
        mBuffer = PageUtil::LoadPageBuffer(rsp->CommFile, rsp->FileSize, true, false);
    else
        throw std::runtime_error("cannot register client : " + mClientName);
}

void ClientPageProvider::ClientExit()
{
    PagedSocketRequest req = {};
    req.Type = PAGED_SOCKET_CLIENT_EXIT;
    SocketArray rspArray;
    GetSocketRspOnReq(req, rspArray, mClientName);
}

int ClientPageProvider::RegisterUnit(const std::string& dir, const std::string& uname)
{
    PagedSocketRequest req = {};
    req.Type = PAGED_SOCKET_UNIT_REGISTER;
    SocketArray rspArray;
    GetSocketRspOnReq(req, rspArray, mClientName);

    auto* rsp = reinterpret_cast<PagedSocketRspUnit*>(&rspArray[0]);
    int commIdx = -1;
    if (rsp->Type == req.Type && rsp->Success)
        commIdx = rsp->CommIndex;
    else
        throw std::runtime_error("connnot register unit:" + mClientName);

    PageCommMsg* msg = GET_COMM_MSG(mCommBuffer, commIdx);
    if (msg->Status == PAGED_COMM_OCCUPIED)
    {
        memcpy(msg->Folder, dir.c_str(), dir.length() + 1);
        memcpy(msg->Name, uname.c_str(), uname.length() + 1);
        msg->IsWriter = mIsWriter;
        msg->Status = PAGED_COMM_HOLDING;
    }
    else
        throw std::runtime_error("server buffer is not allocated: " + mClientName);

    return commIdx;
}

PagePtr ClientPageProvider::GetPage(const std::string& dir, const std::string& uname, int serviceIdx, short pageNum)
{
    PageCommMsg* msg = GET_COMM_MSG(mCommBuffer, serviceIdx);
    msg->PageNum = pageNum;
    msg->Status = PAGED_COMM_REQUESTING;
    while (msg->Status == PAGED_COMM_REQUESTING) {}

    if (msg->Status != PAGED_COMM_ALLOCATED)
    {
        if (msg->Status == PAGED_COMM_MORE_THAN_ONE_WRITER)
            throw std::runtime_error("more than one writer is writing " + dir + " " + uname);
        else
            return PagePtr();
    }
    return Page::Load(dir, uname, pageNum, mReviseAllowed, true);
}

void ClientPageProvider::ReleasePage(void* buffer, int size, int serviceIdx)
{
    PageUtil::ReleasePageBuffer(buffer, size, true);
}
