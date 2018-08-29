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
    SocketArray rspArray = {};
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

void ClientPageProvider::ReleasePage(void* buffer, size_t size, int serviceIdx)
{
    PageUtil::ReleasePageBuffer(buffer, size, true);
}

StrategySocketHandler::StrategySocketHandler(const std::string& strategyName)
        : ClientPageProvider(strategyName, true)
{ }

StrategySocketHandler::~StrategySocketHandler()
{
    UnSubAll();
}

bool StrategySocketHandler::TdConnect(const std::string& config)
{
    std::array<char, MAX_SOCKET_MESSAGE_LENGTH> rspArray = {};
    PagedSocketRequest req = {};
    req.Type = PAGED_SOCKET_TD_LOGIN;
    bzero(req.Config, MAX_SOCKET_CONFIG_LENGTH);
    strcpy(req.Config, config.c_str());
//    req.Source = source;
    GetSocketRspOnReq(req, rspArray, mClientName);
    auto rsp = reinterpret_cast<PagedSocketResponse*>(&rspArray[0]);
    return rsp->Type == req.Type && rsp->Success;
}

bool StrategySocketHandler::MdSubscribeTicker(const std::string& tickers, uint8_t source)
{
    SocketArray reqArray = {}, rspArray = {};
    reqArray[0] = PAGED_SOCKET_SUBSCRIBE_TICKER;
    reqArray[1] = source;
    memcpy(&reqArray[2], tickers.c_str(), tickers.length() + 1);
    GetSocketRsp(reqArray, rspArray);
    auto rsp = reinterpret_cast<PagedSocketResponse*>(&rspArray[0]);
    if (rsp->Type == reqArray[0] && rsp->Success)
    {
        mSubedTicker[source].insert(tickers);
        return true;
    }
    else
        return false;
}

bool StrategySocketHandler::MdSubscribeKline(const std::string& symbol, int klineType, uint8_t source)
{
    SocketArray reqArray = {}, rspArray = {};
    reqArray[0] = PAGED_SOCKET_SUBSCRIBE_KLINE;
    reqArray[1] = source;
    memcpy(&reqArray[2], &klineType, sizeof(int));
    memcpy(&reqArray[2] + sizeof(int), symbol.c_str(), symbol.length() + 1);
    GetSocketRsp(reqArray, rspArray);
    auto rsp = reinterpret_cast<PagedSocketResponse*>(&rspArray[0]);
    if (rsp->Type == reqArray[0] && rsp->Success)
    {
        mSubedKline[source][symbol].insert(klineType);
        return true;
    }
    else
        return false;
}

bool StrategySocketHandler::MdSubscribeDepth(const std::string& symbol, int depth, uint8_t source)
{
    SocketArray reqArray = {}, rspArray = {};
    reqArray[0] = PAGED_SOCKET_SUBSCRIBE_DEPTH;
    reqArray[1] = source;
    memcpy(&reqArray[2], &depth, sizeof(depth));
    memcpy(&reqArray[2] + sizeof(int), symbol.c_str(), symbol.length() + 1);
    GetSocketRsp(reqArray, rspArray);
    auto rsp = reinterpret_cast<PagedSocketResponse*>(&rspArray[0]);
    if (rsp->Type == reqArray[0] && rsp->Success)
    {
        mSubedDepth[source][symbol].insert(depth);
        return true;
    }
    else
        return false;
}

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


void StrategySocketHandler::UnSubAll()
{
    for (auto& item : mSubedTicker)
    {
        for (auto& symbol : item.second)
        {
            SocketArray reqArray = {}, rspArray = {};
            reqArray[0] = PAGED_SOCKET_UNSUBSCRIBE_TICKER;
            reqArray[1] = item.first;
            memcpy(&reqArray[2], symbol.c_str(), symbol.length() + 1);
            GetSocketRsp(reqArray, rspArray);
        }
    }

    for (auto& i : mSubedKline)
    {
        uint8_t source = i.first;
        for (auto& j : i.second)
        {
            const std::string& symbol = j.first;
            for (auto& k : j.second)
            {
                SocketArray reqArray = {}, rspArray = {};
                reqArray[0] = PAGED_SOCKET_UNSUBSCRIBE_KLINE;
                reqArray[1] = source;
                memcpy(&reqArray[2], &k, sizeof(k));
                memcpy(&reqArray[2] + sizeof(k), symbol.c_str(), symbol.length() + 1);
                GetSocketRsp(reqArray, rspArray);
            }
        }
    }

    for (auto& i : mSubedDepth)
    {
        uint8_t source = i.first;
        for (auto& j : i.second)
        {
            const std::string& symbol = j.first;
            for (auto&k : j.second)
            {
                SocketArray reqArray = {}, rspArray = {};
                reqArray[0] = PAGED_SOCKET_UNSUBSCRIBE_DEPTH;
                reqArray[1] = source;
                reqArray[2] = k;
                memcpy(&reqArray[2] + sizeof(int), symbol.c_str(), symbol.length() + 1);
                GetSocketRsp(reqArray, rspArray);
            }
        }
    }
}
