//
// Created by wangzhen on 18-8-5.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_ITGAPI_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_ITGAPI_H

#include "DCSSLog.h"
#include "json.hpp"
#include "DataStruct.h"
#include "UnitReader.h"
#include "UnitWriter.h"

USING_UNIT_NAMESPACE

PRE_DECLARE_PTR(ITGSpi);
PRE_DECLARE_PTR(ITGApi);

struct ClientInfo
{
    bool IsAlive;
    size_t UnitIndex;
    int RidStart;
    int RidEnd;
    int AccountIndex;
};

class ITGSpi : public std::enable_shared_from_this<ITGSpi>
{
public:
    ITGSpi(const std::string& client, DCSSLogPtr logger, const std::string& proxy);
    ~ITGSpi() = default;

public:
    bool Load(const std::string& config);

    void SetReaderThread();

    void SetSignal(int sig);

    void ForceStop();

//    void Connect();

//    void Disconnect();
    /** release api */
    void Release(){};
    /** return true if engine is connected to front */
//    bool IsConnected() const;

    std::string Name() const { return mClient;}

    DCSSLogPtr GetLogger() { return mLogger;}

    std::string GetProxy() const {return mProxy;}

protected:
    /*add strategy, return true if added successfully*/
    bool RegisterClient(const std::string& name, const nlohmann::json& request);
    /*remove strategy*/
    bool RemoveClient(const std::string& name);

public:
    void OnRtnOrder(const DCSSOrderField* order, uint8_t source);

    void OnRspQryTicker(const DCSSTickerField* ticker, uint8_t source, bool isLast, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspQryKline(const DCSSKlineField* kline, uint8_t source, bool isLast, int requestId,
    		int errorId = 0, const char* errorMsg = nullptr);

    void OnRspQryUserInfo(const DCSSTradingAccountField* userInfo, uint8_t source, bool isLast, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, uint8_t source, bool isLast, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspOrderAction(const DCSSRspCancelOrderField* rsp, uint8_t source, bool isLast, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspQryOrder(const DCSSOrderField* order, uint8_t source, bool isLast, int requestId,
    		int errorId = 0, const char* errorMsg = nullptr);

    void OnRtnBalance(const DCSSBalanceField* balance, uint8_t source);

    void OnRtnTdStatus(const GWStatus& status, uint8_t source);

protected:
    void Listening();

protected:
    /*vec of all accounts*/
//    TradeAccount mAccounts;
    /*current nano time*/
    long mCurTime;
    /*-1 if do not accept unregistered account*/
    int mDefaultAccountIndex;

private:
    //reader
    UnitReaderPtr mReader;
    //writer
    UnitWriterPtr mWriter;
    //api map
    std::unordered_map<uint8_t, ITGApiPtr> mApiMap;
    //client name
    const std::string mClient;
    //logger
    DCSSLogPtr mLogger;
    //read thread
    ThreadPtr mReaderThread;

    volatile int mSignalReceived;

    volatile bool mIsRunning;

    const std::string mProxy;
};


class ITGApi
{
public:
    /*only interface to create a api*/
    static ITGApiPtr CreateTGApi(uint8_t source);

    virtual void LoadAccount(const nlohmann::json& config) = 0;
    virtual void Connect() = 0;
    virtual void Disconnect() = 0;
    virtual void Login() = 0;
    virtual void Logout() = 0;
    virtual bool IsConnected() const = 0;
    virtual bool IsLogged() const = 0;

    void RegisterSpi(ITGSpiPtr spi);

public:
    virtual void ReqQryTicker(const DCSSReqQryTickerField* req, int requestID) = 0;
    virtual void ReqQryKline(const DCSSReqQryKlineField* req, int requestID) = 0;
    virtual void ReqQryUserInfo(int requestID) = 0;
    virtual void ReqQryOrder(const DCSSReqQryOrderField* req, int requestID) = 0;
    virtual void ReqInsertOrder(const DCSSReqInsertOrderField* req, int requestID) = 0;
    virtual void ReqCancelOrder(const DCSSReqCancelOrderField* req, int requestID) = 0;

protected:
    explicit ITGApi(uint8_t source)
            :mSourceId(source)
    {}
    virtual ~ITGApi() = default;

protected:
    const uint8_t mSourceId;
    DCSSLogPtr mLogger;
    std::string mProxy;
    ITGSpiPtr mSpi;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_ITGAPI_H
