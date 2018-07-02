//
// Created by wangzhen on 18-6-20.
//

#include "StrategySocketHandler.h"
#include <boost/asio.hpp>

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

void GetSocketRspOnReq(PagedSocketRequest& req, SocketArray& data, const std::string& name)
{
    memcpy(req.Name, name.c_str(), name.length() + 1);
    SocketArray reqArray;
    memcpy(&reqArray[0], &req, sizeof(req));
    GetSocketRsp(reqArray, data);
}


StrategySocketHandler::StrategySocketHandler(const std::string& strategyName)
: ClientPageProvider(strategyName, true)
{ }

bool StrategySocketHandler::TdConnect(short source)
{
    std::array<char, MAX_SOCKET_MESSAGE_LENGTH> rspArray;
    PagedSocketRequest req = {};
    req.Type = PAGED_SOCKET_TD_LOGIN;
    req.Source = source;
    GetSocketRspOnReq(req, rspArray, mClientName);
    PagedSocketResponse* rsp = reinterpret_cast<PagedSocketResponse*>(&rspArray[0]);
    return rsp->Type == req.Type && rsp->Success;
}

bool StrategySocketHandler::MdSubscribe(const std::vector<DCSSSymbolField>& tickers, short source)
{
    size_t idx = 0;
    SocketArray reqArray, rspArray;
    memset(&reqArray[0], 0, MAX_SOCKET_MESSAGE_LENGTH);
    int pos = 3;

    int tickerLen = sizeof(DCSSSymbolField);

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
//            reqArray[2] = msgT
            GetSocketRsp(reqArray, rspArray);
            auto* rsp = reinterpret_cast<PagedSocketResponse*>(&rspArray[0]);
            if (rsp->Type != reqArray[0] || !rsp->Success)
                return false;

            memset(&reqArray[0], '\0', MAX_SOCKET_MESSAGE_LENGTH);
            pos = 3;
        }
    }
    reqArray[0] = PAGED_SOCKET_SUBSCRIBE;
    reqArray[1] = source;
    GetSocketRsp(reqArray, rspArray);
    auto* rsp = reinterpret_cast<PagedSocketResponse*>(&rspArray[0]);
    return rsp->Type == reqArray[0] && rsp->Success;
}

bool StrategySocketHandler::RegisterStrategy(int& ridStart, int& ridEnd)
{
    PagedSocketRequest req = {};
    req.Type = PAGED_SOCKET_STRATEGY_REGISTER;
    SocketArray rspArray;
    GetSocketRspOnReq(req, rspArray, mClientName);
    auto* rsp = reinterpret_cast<PagedSocketRspStrategy*>(&rspArray[0]);
    ridStart = rsp->RidStart;
    ridEnd = rsp->RidEnd;
    return rsp->Type == req.Type && rsp->Success;
}