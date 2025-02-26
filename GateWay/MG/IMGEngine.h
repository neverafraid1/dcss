//
// Created by wangzhen on 18-6-15.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H

#include "DCSSLog.h"
#include "DataStruct.h"
#include "IEngine.h"
#include "json.hpp"

PRE_DECLARE_PTR(IMGApi);

UNIT_NAMESPACE_START
PRE_DECLARE_PTR(UnitWriter);
UNIT_NAMESPACE_END

class IMGEngine : public IEngine, public std::enable_shared_from_this<IMGEngine>
{
public:
    IMGEngine() = default;

    ~IMGEngine() override ;

public:
    void SetReaderThread() override ;

    void Load(const nlohmann::json& config);

    void Connect();

    bool IsConnected() const ;

    std::string Name() override {return "MG";}

public:
    void OnRtnDepth(const DCSSDepthField* depth, uint8_t source);

    void OnRtnTicker(const DCSSTickerField* ticker, uint8_t source);

    void OnRtnKline(const DCSSKlineField* kline, uint8_t source);

protected:
    void Listening();

protected:
    IMGApiPtr mMGApi;

    UnitEngine::UnitWriterPtr mWriter;
};

DECLARE_PTR(IMGEngine);

class IMGApi
{
public:
    static IMGApiPtr Create(uint8_t source);

    short GetSource() const ;

    void Register(IMGEnginePtr spi);

    void SetProxy(const std::string& proxy);

    virtual void Connect() = 0;
    virtual bool IsConnected() const = 0;
    virtual std::string Name() const = 0;

public:
    virtual void ReqSubTicker(const std::string& Symbol) = 0;
    virtual void ReqSubDepth(const std::string& Symbol) = 0;
    virtual void ReqSubKline(const std::string& Symbol, KlineType klineType) = 0;

    virtual void ReqUnSubTicker(const std::string& Symbol) = 0;
    virtual void ReqUnSubDepth(const std::string& Symbol) = 0;
    virtual void ReqUnSubKline(const std::string& Symbol, KlineType klineType) = 0;

protected:
    explicit IMGApi(uint8_t source);

    virtual ~IMGApi() = default;

protected:
    uint8_t mSourceId;
    IMGEngine* mSpi;
    DCSSLog* mLogger;
    std::string mProxy;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H
