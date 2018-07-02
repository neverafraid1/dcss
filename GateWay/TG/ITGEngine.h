//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_TGENGINE_H
#define DCSS_TGENGINE_H

#include "DCSSLog.h"
#include "DataStruct.h"
#include "IEngine.h"

#include <memory>

class ITGApi;
DECLARE_PTR(ITGApi);

class ITGEngine : public IEngine , public std::enable_shared_from_this<ITGEngine>
{
public:
    ITGEngine()
    { };
    virtual ~ITGEngine()
    { };

public:
    void Init(const std::vector<short>& index);

    void SetReaderThread();

public:
    void OnRtnOrder(const DCSSOrderField* order);

    void OnRspQryTicker(const DCSSTickerField* ticker, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspQryKline(const DCSSKlineHeaderField* header, const std::vector<DCSSKlineField>& kline,
            int requestId, int errorId = 0, const char* errorMsg = nullptr);

    void OnRspQryUserInfo(const DCSSUserInfoField* userInfo, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspOrderAction(const DCSSRspCancelOrderField* rsp, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspQryOrder(const DCSSRspQryOrderHeaderField* header, const std::vector<DCSSRspQryOrderField>& order,
            int requestId, int errorId = 0, const char* errorMsg = nullptr);

protected:
    void Listening();

protected:
    std::map<short, ITGApiPtr> mApiMap;

};

DECLARE_PTR(ITGEngine);

class ITGApi
{
public:

    static ITGApiPtr CreateTGApi(short index);

    virtual void Connect() = 0;
    virtual void Login() = 0;
    virtual void Logout() = 0;
    virtual bool IsConnected() const = 0;
    virtual bool IsLogged() = 0;

    virtual void Register(ITGEnginePtr spi) = 0;

public:
    virtual void ReqQryTicker(const DCSSReqQryTickerField* req, int requestID) = 0;
    virtual void ReqQryKline(const DCSSReqQryKlineField* req, int requestID) = 0;
    virtual void ReqQryUserInfo(int requestID) = 0;
    virtual void ReqQryOrder(const DCSSReqQryOrderField* req, int requestID) = 0;
    virtual void ReqInsertOrder(const DCSSReqInsertOrderField* req, int requestID) = 0;
    virtual void ReqCancelOrder(const DCSSReqCancelOrderField* req, int requestID) = 0;
protected:
    ITGApi(){};
    virtual ~ITGApi(){};
};

#endif //DEMO_TGENGINE_H
