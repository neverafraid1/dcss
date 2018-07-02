//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGESOCKETHANDLER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGESOCKETHANDLER_H


#include "Declare.h"
#include "DCSSLog.h"
#include "SocketStruct.h"

class IPageSocketUtil
{
public:
    /** return logger */
    virtual DCSSLogPtr GetLogger() const = 0;

    virtual int RegUnit(const std::string& clientName) = 0;

    virtual IntPair RegStrategy(const std::string& strategyName) = 0;

    virtual bool RegClient() = 0;

    /** exit client */
    virtual void ExitClient(const std::string& clientName) = 0;

    virtual bool LoginTd(const std::string& clientName, short source) = 0;

    virtual bool SubTicker(const std::vector<std::string>& tickers, short source, short msgType, bool isLast) = 0;

    virtual void AcquireMutex() const = 0;

    virtual void ReleaseMutex() const = 0;

};

DECLARE_PTR(IPageSocketUtil);

class PageSocketHandler;
DECLARE_PTR(PageSocketHandler);

class PageSocketHandler : public std::enable_shared_from_this<PageSocketHandler>
{
public:
    static PageSocketHandlerPtr GetInstance();

    void Run(IPageSocketUtilPtr util);

    void Stop();

    bool IsRunning() const;

private:
    PageSocketHandler();

    void ProcessMsg();

    void HandleAccept();

private:
    bool mIoRunning;

    DCSSLogPtr mLogger;

    IPageSocketUtilPtr mUtil;

    static PageSocketHandlerPtr mPtr;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGESOCKETHANDLER_H
