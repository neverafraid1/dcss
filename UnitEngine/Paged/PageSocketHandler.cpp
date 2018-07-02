//
// Created by wangzhen on 18-6-25.
//

#include "PageSocketHandler.h"
#include "SocketStruct.h"
#include "Timer.h"
#include <boost/asio.hpp>

using namespace boost::asio;

std::array<char, MAX_SOCKET_MESSAGE_LENGTH> gRecvData;
std::array<char, MAX_SOCKET_MESSAGE_LENGTH> gSendData;
std::shared_ptr<local::stream_protocol::acceptor> gAcceptor;
std::shared_ptr<local::stream_protocol::socket> gSocket;
std::shared_ptr<io_service> gIo;

PageSocketHandlerPtr PageSocketHandler::mPtr = PageSocketHandlerPtr(nullptr);

PageSocketHandler::PageSocketHandler() : mIoRunning(false)
{ }

PageSocketHandlerPtr PageSocketHandler::GetInstance()
{
    // have problem when multi thread called
    if (mPtr.get() == nullptr)
    {
        mPtr = PageSocketHandlerPtr(new PageSocketHandler());
    }

    return mPtr;
}

bool PageSocketHandler::IsRunning() const
{
    return mIoRunning;
}

void PageSocketHandler::Run(IPageSocketUtilPtr util)
{
    mUtil = util;
    mLogger = mUtil->GetLogger();
    gIo.reset(new io_service());
    gAcceptor.reset(new local::stream_protocol::acceptor(
            *gIo,
            local::stream_protocol::endpoint(PAGED_SOCKET_FILE)
            ));
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
    PagedSocketRequest* req = (PagedSocketRequest*)&gRecvData[0];
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

    }
    }


}