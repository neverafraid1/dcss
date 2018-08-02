//
// Created by wangzhen on 18-6-25.
//

#include "PageSocketHandler.h"
#include "PageSocketStruct.h"
#include "DataStruct.h"
#include "Timer.h"
#include <boost/asio.hpp>

USING_UNIT_NAMESPACE

using namespace boost::asio;

std::array<char, MAX_SOCKET_MESSAGE_LENGTH> gRecvData;
std::array<char, MAX_SOCKET_MESSAGE_LENGTH> gSendData;
std::shared_ptr<local::stream_protocol::acceptor> gAcceptor;
std::shared_ptr<local::stream_protocol::socket> gSocket;
std::shared_ptr<io_service> gIo;

std::shared_ptr<PageSocketHandler> PageSocketHandler::mPtr(nullptr);

PageSocketHandler::PageSocketHandler() : mIoRunning(false)
{ }

PageSocketHandler* PageSocketHandler::GetInstance()
{
    // have problem when multi thread called
    if (mPtr.get() == nullptr)
    {
        mPtr.reset(new PageSocketHandler());
    }

    return mPtr.get();
}

bool PageSocketHandler::IsRunning() const
{
    return mIoRunning;
}

void PageSocketHandler::Run(IPageSocketUtil* util)
{
    mUtil = util;
    mLogger = mUtil->GetLogger();
    gIo.reset(new io_service());
    gAcceptor.reset(new local::stream_protocol::acceptor(
            *gIo,
            local::stream_protocol::endpoint(PAGED_SOCKET_FILE)
            ));

    std::string ss(PAGED_SOCKET_FILE);

    gSocket.reset(new local::stream_protocol::socket(*gIo));

    gAcceptor->async_accept(*gSocket, std::bind(&PageSocketHandler::HandleAccept, this));
    mIoRunning = true;
    gIo->run();
}

void PageSocketHandler::Stop()
{
    if (gIo != nullptr)
        gIo->stop();
    if (gSocket != nullptr)
        gSocket->close();
    if (gAcceptor != nullptr)
        gAcceptor->close();

    mIoRunning = false;
}

void PageSocketHandler::HandleAccept()
{
    gSocket->read_some(buffer(gRecvData));
    mUtil->AcquireMutex();
    ProcessMsg();
    mUtil->ReleaseMutex();
    gSocket.reset(new local::stream_protocol::socket(*gIo));
    gAcceptor->async_accept(*gSocket, std::bind(&PageSocketHandler::HandleAccept, this));
}

void PageSocketHandler::ProcessMsg()
{
    memset(&gSendData[0], 0, gSendData.size());

    auto req = reinterpret_cast<PagedSocketRequest*>(&gRecvData[0]);
    auto reqType = req->Type;

    switch (reqType)
    {
    case PAGED_SOCKET_CONNECTION_PING:
    {
        strcpy(&gSendData[0], "Hello, world");
        break;
    }
    case PAGED_SOCKET_UNIT_REGISTER:
    {
        int idx = mUtil->RegUnit(req->Name);
        PagedSocketRspUnit rsp = {};
        rsp.Type = reqType;
        rsp.Success = idx >= 0;
        rsp.CommIndex = idx;
        memcpy(&gSendData[0], &rsp, sizeof(rsp));
        break;
    }
    case PAGED_SOCKET_STRATEGY_REGISTER:
    {
        IntPair ridPair = mUtil->RegStrategy(req->Name);
        PagedSocketRspStrategy rsp = {};
        rsp.Type = reqType;
        rsp.Success = ridPair.first > 0 && ridPair.first < ridPair.second;
        rsp.RidStart = ridPair.first;
        rsp.RidEnd = ridPair.second;
        memcpy(&gSendData[0], &rsp, sizeof(rsp));
        break;
    }
    case PAGED_SOCKET_READER_REGISTER:
    case PAGED_SOCKET_WRITER_REGISTER:
    {
        std::string commFile;
        size_t fileSize;
        bool ret = mUtil->RegClient(commFile, fileSize, req->Name, req->Pid, reqType == PAGED_SOCKET_WRITER_REGISTER);
        PagedSocketRspClient rsp = {};
        rsp.Type = reqType;
        rsp.Success = ret;
        rsp.FileSize = fileSize;
        memcpy(rsp.CommFile, commFile.c_str(), commFile.length() + 1);
        memcpy(&gSendData[0], &rsp, sizeof(rsp));
        break;
    }
    case PAGED_SOCKET_CLIENT_EXIT:
    {
        mUtil->ExitClient(req->Name);
        PagedSocketResponse rsp = {};
        rsp.Type = reqType;
        rsp.Success = true;
        memcpy(&gSendData[0], &rsp, sizeof(rsp));
        break;
    }
    case PAGED_SOCKET_SUBSCRIBE_TICKER:
    {
        uint8_t source = gRecvData[1];
        bool ret = mUtil->SubTicker(&gRecvData[2], source);
        PagedSocketResponse rsp = {};
        rsp.Type = reqType;
        rsp.Success = ret;
        memcpy(&gSendData[0], &rsp, sizeof(rsp));
        break;
    }
    case PAGED_SOCKET_UNSUBSCRIBE_TICKER:
    {
        uint8_t source = gRecvData[1];
        bool ret = mUtil->UnSubTicker(&gRecvData[2], source);
        PagedSocketResponse rsp = {};
        rsp.Type = reqType;
        rsp.Success = ret;
        memcpy(&gSendData[0], &rsp, sizeof(rsp));
        break;
    }
    case PAGED_SOCKET_SUBSCRIBE_KLINE:
    {
        uint8_t source = gRecvData[1];
        KlineTypeType type = gRecvData[2];
        PagedSocketResponse rsp = {};
        rsp.Type = reqType;
        rsp.Success = mUtil->SubKline(&gRecvData[3], type, source);
        memcpy(&gSendData[0], &rsp, sizeof(rsp));
        break;
    }
    case PAGED_SOCKET_UNSUBSCRIBE_KLINE:
    {
        uint8_t source = gRecvData[1];
        KlineTypeType type = gRecvData[2];
        PagedSocketResponse rsp = {};
        rsp.Type = reqType;
        rsp.Success = mUtil->UnSubKline(&gRecvData[3], type, source);
        memcpy(&gSendData[0], &rsp, sizeof(rsp));
        break;
    }
    case PAGED_SOCKET_SUBSCRIBE_DEPTH:
    {
        uint8_t source = gRecvData[1];
        auto depth = reinterpret_cast<int*>(&gRecvData[2]);
        PagedSocketResponse rsp = {};
        rsp.Type = reqType;
        rsp.Success = mUtil->SubDepth((&gRecvData[2] + sizeof(int)), *depth, source);
        memcpy(&gSendData[0], &rsp, sizeof(rsp));
        break;
    }
    case PAGED_SOCKET_UNSUBSCRIBE_DEPTH:
    {
        uint8_t source = gRecvData[1];
        auto depth = reinterpret_cast<int*>(&gRecvData[2]);
        PagedSocketResponse rsp = {};
        rsp.Type = reqType;
        rsp.Success = mUtil->UnSubDepth(&gRecvData[2] + sizeof(int), *depth, source);
        memcpy(&gSendData[0], &rsp, sizeof(rsp));
        break;
    }
    case PAGED_SOCKET_TD_LOGIN:
    {
        bool ret = mUtil->LoginTd(req->Name, req->Source);
        PagedSocketResponse rsp = {};
        rsp.Type = reqType;
        rsp.Success = ret;
        memcpy(&gSendData[0], &rsp, sizeof(rsp));
    }
    default:
        break;
    }
    write(*gSocket, buffer(gSendData));
}