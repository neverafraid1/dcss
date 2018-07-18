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
        mPtr = std::shared_ptr<PageSocketHandler>(new PageSocketHandler());
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
    if (gIo.get() != nullptr)
        gIo->stop();
    if (gSocket.get() != nullptr)
        gSocket->close();
    if (gAcceptor.get() != nullptr)
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
        int fileSize;
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
    case PAGED_SOCKET_SUBSCRIBE:
    case PAGED_SOCKET_SUBSCRIBE_TBC:
    {
        short source = gRecvData[1];
        auto size = reinterpret_cast<size_t*>(&gRecvData[2]);
        std::vector<DCSSSymbolField> symbol;
        int pos = 2 + sizeof(size_t);
        size_t num = 0;
        DCSSSymbolField* p = nullptr;
        while (pos < MAX_SOCKET_MESSAGE_LENGTH - 1 && num++ < *size)
        {
            p = reinterpret_cast<DCSSSymbolField*>(&gRecvData[pos]);
            if (p != nullptr)
            {
                symbol.emplace_back(*p);
                pos += sizeof(DCSSSymbolField);
            }
            else
                break;
        }
        bool ret = mUtil->SubTicker(symbol, source, reqType == PAGED_SOCKET_SUBSCRIBE);
        PagedSocketResponse rsp = {};
        rsp.Type = reqType;
        rsp.Success = ret;
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