//
// Created by wangzhen on 18-6-20.
//

#include "PageProvider.h"
#include "PageCommStruct.h"
#include "PageSocketStruct.h"
#include "PageUtil.h"
#include "Page.h"
#include "StrategySocketHandler.h"

#include <boost/asio.hpp>

USING_UNIT_NAMESPACE

typedef std::array<char, MAX_SOCKET_MESSAGE_LENGTH> SocketArray;

/**
 * get socket response via paged socket
 */
void GetSocketRsp(SocketArray& input, SocketArray& output)
{
    using namespace boost::asio;

    io_service is;
    local::stream_protocol::socket sock(is);
    sock.connect(local::stream_protocol::endpoint(PAGED_SOCKET_FILE));
    boost::system::error_code error;
    write(sock, buffer(input), error);
    sock.read_some(buffer(output), error);
}

/**
 * send req and recv rsp via paged socket
 * @param req
 * @param data
 * @param name
 */
void GetSocketRspOnReq(PagedSocketRequest& req, SocketArray& data, const std::string& name)
{
    memcpy(req.Name, name.c_str(), name.length() + 1);
    SocketArray reqArray;
    memcpy(&reqArray[0], &req, sizeof(req));
    GetSocketRsp(reqArray, data);
}


ClientPageProvider::ClientPageProvider(const std::string& clientName, bool isWriting, bool reviseAllowed)
: mClientName(clientName), mCommBuffer(nullptr)
{
    mIsWriter = isWriting;
    mReviseAllowed = mIsWriter || reviseAllowed;
    RegisterClient();
}

void ClientPageProvider::RegisterClient()
{
    PagedSocketRequest req = {};
    req.Type = mIsWriter ? PAGED_SOCKET_WRITER_REGISTER : PAGED_SOCKET_READER_REGISTER;
    req.Pid = getpid();
    SocketArray rspArray;
    GetSocketRspOnReq(req, rspArray, mClientName);

    auto rsp = reinterpret_cast<PagedSocketRspClient*>(&rspArray[0]);
    if (rsp->Type == req.Type && rsp->Success)
        mCommBuffer = PageUtil::LoadPageBuffer(rsp->CommFile, rsp->FileSize, true, false);
    else
        throw std::runtime_error("cannot register client : " + mClientName);
}

void ClientPageProvider::ExitClient()
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

    auto rsp = reinterpret_cast<PagedSocketRspUnit*>(&rspArray[0]);
    int commIdx(-1);
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
        throw std::runtime_error("server mBuffer is not allocated: " + mClientName);

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

StrategySocketHandler::StrategySocketHandler(const std::string& strategyName)
        : ClientPageProvider(strategyName, true)
{ }

bool StrategySocketHandler::TdConnect(short source)
{
    std::array<char, MAX_SOCKET_MESSAGE_LENGTH> rspArray = {};
    PagedSocketRequest req = {};
    req.Type = PAGED_SOCKET_TD_LOGIN;
    req.Source = source;
    GetSocketRspOnReq(req, rspArray, mClientName);
    auto rsp = reinterpret_cast<PagedSocketResponse*>(&rspArray[0]);
    return rsp->Type == req.Type && rsp->Success;
}

bool StrategySocketHandler::MdSubscribe(const std::vector<DCSSSymbolField>& tickers, short source)
{
    size_t idx = 0;
    SocketArray reqArray = {}, rspArray = {};
    int pos = 2 + sizeof(size_t);

    size_t size = tickers.size();

    size_t tickerLen = sizeof(DCSSSymbolField);

    while (idx < tickers.size())
    {
        if (pos + tickerLen < MAX_SOCKET_MESSAGE_LENGTH - 1)
        {
            memcpy(&reqArray[pos], &tickers[idx], tickerLen);
            pos += tickerLen;
            ++idx;
        }
        else
        {
            reqArray[0] = PAGED_SOCKET_SUBSCRIBE_TBC;
            reqArray[1] = source;
            memcpy(&reqArray[2], &size, sizeof(size_t));
            GetSocketRsp(reqArray, rspArray);
            auto rsp = reinterpret_cast<PagedSocketResponse*>(&rspArray[0]);
            if (rsp->Type != reqArray[0] || !rsp->Success)
                return false;

            memset(&reqArray[0], '\0', MAX_SOCKET_MESSAGE_LENGTH);
            pos = 2 + sizeof(size_t);
        }
    }
    reqArray[0] = PAGED_SOCKET_SUBSCRIBE;
    reqArray[1] = source;
    memcpy(&reqArray[2], &size, sizeof(size_t));
    GetSocketRsp(reqArray, rspArray);
    auto rsp = reinterpret_cast<PagedSocketResponse*>(&rspArray[0]);
    return rsp->Type == reqArray[0] && rsp->Success;
}

bool StrategySocketHandler::MdSubscribeKline(const DCSSSymbolField& symbol, KlineTypeType klineType, short source)
{}

bool StrategySocketHandler::MdSubscribeDepth(const DCSSSymbolField& symbol, int depth, short source)
{}

bool StrategySocketHandler::RegisterStrategy(int& ridStart, int& ridEnd)
{
    PagedSocketRequest req = {};
    req.Type = PAGED_SOCKET_STRATEGY_REGISTER;
    SocketArray rspArray;
    GetSocketRspOnReq(req, rspArray, mClientName);
    auto rsp = reinterpret_cast<PagedSocketRspStrategy*>(&rspArray[0]);
    ridStart = rsp->RidStart;
    ridEnd = rsp->RidEnd;
    return rsp->Type == req.Type && rsp->Success;
}
