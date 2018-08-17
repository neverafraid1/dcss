//
// Created by wangzhen on 18-6-15.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H

#include "DCSSLog.h"
#include "DataStruct.h"
#include "IEngine.h"
#include "json.hpp"
#include "UnitWriter.h"

PRE_DECLARE_PTR(IMGApi);

class IMGEngine : public IEngine, public std::enable_shared_from_this<IMGEngine>
{
public:
    IMGEngine() = default;

    ~IMGEngine() override = default;

public:
    void SetReaderThread() override ;

    void Load(const std::string& str);

    void Connect();

    bool IsConnected() const ;

    std::string Name() override {return "MG";}

public:
    void OnRtnDepth(const DCSSDepthHeaderField* header, const std::vector<DCSSDepthField>& depthVec, uint8_t source);

    void OnRtnTicker(const DCSSTickerField* ticker, uint8_t source);

    void OnRtnKline(const DCSSKlineHeaderField* header, const std::vector<DCSSKlineField>& klineVec, uint8_t source);

protected:
    void Listening();

protected:
    IMGApiPtr mMGApi;

    UnitWriterPtr mWriter;
};

DECLARE_PTR(IMGEngine);

class IMGApi
{
public:
    static IMGApiPtr Create(uint8_t source);

    short GetSource() const ;

    void Register(IMGEnginePtr spi);

    virtual void Connect() = 0;
    virtual bool IsConnected() const = 0;
    virtual std::string Name() const = 0;

public:
    virtual void ReqSubTicker(const std::string& Symbol) = 0;
    virtual void ReqSubDepth(const std::string& Symbol, int depth) = 0;
    virtual void ReqSubKline(const std::string& Symbol, KlineType klineType) = 0;

    virtual void ReqUnSubTicker(const std::string& Symbol) = 0;
    virtual void ReqUnSubDepth(const std::string& Symbol, int depth) = 0;
    virtual void ReqUnSubKline(const std::string& Symbol, KlineType klineType) = 0;

protected:
    explicit IMGApi(uint8_t source)
            : mSourceId(source)
    {}
    virtual ~IMGApi() = default;

protected:
    uint8_t mSourceId;
    IMGEnginePtr mSpi;
    DCSSLogPtr mLogger;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H
