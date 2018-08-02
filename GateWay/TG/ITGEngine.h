//
// Created by wangzhen on 18-6-7.
//

#ifndef DCSS_TGENGINE_H
#define DCSS_TGENGINE_H

#include "DataStruct.h"
#include "IEngine.h"

PRE_DECLARE_PTR(ITGApi);

struct TradeAccount
{
    std::string ApiKey;
    std::string SecretKey;
    ITGApiPtr Api;
};

struct ClientInfo
{
    bool IsAlive;
    size_t UnitIndex;
    int RidStart;
    int RidEnd;
    int AccountIndex;
};

/**
 * base class of all trade engine
 *
 * one strategy may connect to several different trade engines at the same time,
 * we use this to wrap te detail
 */
class ITGEngine : public IEngine , public std::enable_shared_from_this<ITGEngine>
{
public:
    ITGEngine();
    ~ITGEngine() override = default;

public:
    void Init() override ;

    void Load(const nlohmann::json& config) override ;

    void SetReaderThread() override ;

    void Connect() override;

    void Disconnect() override;
    /** release api */
    void ReleaseApi() override {};
    /** return true if engine is connected to front */
    bool IsConnected() const  override;

    virtual TradeAccount LoadAccount(int idx, const nlohmann::json& account);

protected:
    /*add strategy, return true if added successfully*/
    bool RegisterClient(const std::string& name, const nlohmann::json& request);
    /*remove strategy*/
    bool RemoveClient(const std::string& name, const nlohmann::json& request);

public:
    void OnRtnOrder(const DCSSOrderField* order, uint8_t source);

    void OnRspQryTicker(const DCSSTickerField* ticker, uint8_t source, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspQryKline(const DCSSKlineHeaderField* header, uint8_t source, const std::vector<DCSSKlineField>& kline,
            int requestId, int errorId = 0, const char* errorMsg = nullptr);

    void OnRspQryUserInfo(const DCSSTradingAccountField* userInfo, uint8_t source, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspOrderInsert(const DCSSRspInsertOrderField* rsp, uint8_t source, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspOrderAction(const DCSSRspCancelOrderField* rsp, uint8_t source, int requestId,
            int errorId = 0, const char* errorMsg = nullptr);

    void OnRspQryOrder(const DCSSRspQryOrderHeaderField* header, uint8_t source, const std::vector<DCSSRspQryOrderField>& order,
            int requestId, int errorId = 0, const char* errorMsg = nullptr);

    void OnRtnBalance(const DCSSBalanceField* balance, uint8_t source);

    void OnRtnTdStatus(const GateWayStatusType& status, uint8_t source);

protected:
    void Listening();

protected:
    ClientInfo mClient;
    /*vec of all accounts*/
//    TradeAccount mAccounts;
    /*current nano time*/
    long mCurTime;
    /*-1 if do not accept unregistered account*/
    int mDefaultAccountIndex;

    std::map<short, ITGApiPtr> mApiMap;
    // client name, load from config
    std::string mName;
    // client folder/ load from config
    std::string mFolder;
};

DECLARE_PTR(ITGEngine);

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

    virtual void Register(ITGEnginePtr spi) = 0;

public:
    virtual void ReqQryTicker(const DCSSReqQryTickerField* req, int requestID) = 0;
    virtual void ReqQryKline(const DCSSReqQryKlineField* req, int requestID) = 0;
    virtual void ReqQryUserInfo(int requestID) = 0;
    virtual void ReqQryOrder(const DCSSReqQryOrderField* req, int requestID) = 0;
    virtual void ReqInsertOrder(const DCSSReqInsertOrderField* req, int requestID) = 0;
    virtual void ReqCancelOrder(const DCSSReqCancelOrderField* req, int requestID) = 0;

protected:
    explicit ITGApi(uint8_t source)
    : mSourceId(source)
    {}
    virtual ~ITGApi() = default;

protected:
    uint8_t mSourceId;
};

#endif //DEMO_TGENGINE_H
