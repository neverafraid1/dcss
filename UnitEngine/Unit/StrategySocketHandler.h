//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYSOCKETHANDLER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYSOCKETHANDLER_H

#include "PageProvider.h"
#include "IStrategyUtil.h"
#include "SocketStruct.h"
#include "Declare.h"
#include <array>

typedef std::array<char, MAX_SOCKET_MESSAGE_LENGTH> SocketArray;

void GetSocketRsp(SocketArray& input, SocketArray& output);

void GetSocketRspOnReq(PagedSocketRequest& req, SocketArray& data, const std::string& name);

class StrategySocketHandler : public IStrategyUtil, public ClientPageProvider
{
public:

    StrategySocketHandler(const std::string& strategyName);

    virtual bool RegisterStrategy(int& ridStart, int& ridEnd);

    virtual bool TdConnect(short source);

    virtual bool MdSubscribe(const std::vector<DCSSSymbolField>& tickers, short source);
};

DECLARE_PTR(StrategySocketHandler);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_STRATEGYSOCKETHANDLER_H
