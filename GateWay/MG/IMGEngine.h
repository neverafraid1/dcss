//
// Created by wangzhen on 18-6-15.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H

#include "DCSSLog.h"
#include "DataStruct.h"
#include "IEngine.h"

PRE_DECLARE_PTR(IMGApi);

class IMGEngine : public IEngine, public std::enable_shared_from_this<IMGEngine>
{
public:
    IMGEngine() = default;

    ~IMGEngine() override = default;

public:
    void Init() override ;

    void SetReaderThread() override ;

    void Load(const nlohmann::json& config) override ;

    void Connect() override {};

    void ReleaseApi() override {};

    bool IsConnected() const override { return true;};

public:
    void OnRtnDepth(const DCSSDepthHeaderField* header, const std::vector<DCSSDepthField>& depthVec, short source);

    void OnRtnTicker(const DCSSTickerField* ticker, short source);

    void OnRtnKline(const DCSSKlineHeaderField* header, const std::vector<DCSSKlineField>& klineVec, short source);

protected:
    void Listening();

protected:
    IMGApiPtr mMGApi;
};

DECLARE_PTR(IMGEngine);

class IMGApi
{
public:
    static IMGApiPtr Create(short source);

    short GetSource() const ;

    virtual void Register(IMGEnginePtr spi) = 0;

    virtual void Connect() = 0;
    virtual bool IsConnected() const = 0;
    virtual std::string Name() const = 0;

public:
    virtual void ReqSubTicker(const DCSSSymbolField& Symbol) = 0;
    virtual void ReqSubDepth(const DCSSSymbolField& Symbol, int depth) = 0;
    virtual void ReqSubKline(const DCSSSymbolField& Symbol, KlineTypeType klineType) = 0;

    virtual void ReqUnSubTicker(const DCSSSymbolField& Symbol) = 0;
    virtual void ReqUnSubDepth(const DCSSSymbolField& Symbol, int depth) = 0;
    virtual void ReqUnSubKline(const DCSSSymbolField& Symbol, KlineTypeType klineType) = 0;

protected:
    IMGApi(short source)
            : mSourceId(source)
    {}
    virtual ~IMGApi() = default;

protected:
    short mSourceId;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H
