//
// Created by wangzhen on 18-6-15.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H

#include "DCSSLog.h"
#include "DataStruct.h"
#include "IEngine.h"

class IMGApi;
DECLARE_PTR(IMGApi);

class IMGEngine : public IEngine, public std::enable_shared_from_this<IMGEngine>
{
public:
    IMGEngine()
    {}

    virtual ~IMGEngine()
    {}

public:
    void Init(short index);
    void SetReaderThread();


    void OnRtnDepth(const DCSSDepthHeaderField* header, const std::vector<DCSSDepthField>& depthVec);
    void OnRtnTicker(const DCSSTickerField* ticker);
    void OnRtnKline(const DCSSKlineHeaderField* header, const std::vector<DCSSKlineField>& klineVec);

protected:
    void Listening();

protected:
    IMGApiPtr mMGApi;
};

DECLARE_PTR(IMGEngine);

class IMGApi
{
public:
    static IMGApiPtr Create(short index);

    virtual void Register(IMGEnginePtr spi) = 0;

    virtual void Connect() = 0;
    virtual bool IsConnected() const = 0;

public:
    virtual void ReqSubTicker(const char10& Symbol) = 0;
    virtual void ReqSubDepth(const char10& Symbol, int depth) = 0;
    virtual void ReqSubKline(const char10& Symbol, KlineTypeType klineType) = 0;

    virtual void ReqUnSubTicker(const char10& Symbol) = 0;
    virtual void ReqUnSubDepth(const char10& Symbol, int depth) = 0;
    virtual void ReqUnSubKline(const char10& Symbol, KlineTypeType klineType) = 0;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_MGENGINE_H
