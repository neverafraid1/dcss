//
// Created by wangzhen on 18-6-25.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGESOCKETHANDLER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGESOCKETHANDLER_H


#include "Declare.h"
#include "DCSSLog.h"
#include "PageSocketStruct.h"
#include "DataStruct.h"

UNIT_NAMESPACE_START

class IPageSocketUtil
{
public:
    /*return logger */
    virtual DCSSLogPtr GetLogger() const = 0;
    /*return unix index in communicate file*/
    virtual int RegUnit(const std::string& clientName) = 0;

    virtual IntPair RegStrategy(const std::string& strategyName) = 0;
    /**/
    virtual bool RegClient(std::string& commFile, int& fileSize, const std::string& clientName, int pid, bool isWriter) = 0;

    /** exit client */
    virtual void ExitClient(const std::string& clientName) = 0;
    /*login trade engine*/
    virtual bool LoginTd(const std::string& clientName, short source) = 0;

    virtual bool SubTicker(const std::vector<DCSSSymbolField>& tickers, short source, bool isLast) = 0;

    virtual void AcquireMutex() const = 0;

    virtual void ReleaseMutex() const = 0;

};

/**
 * socker handler for page engine
 */
class PageSocketHandler : public std::enable_shared_from_this<PageSocketHandler>
{
public:
    static PageSocketHandler* GetInstance();

    void Run(IPageSocketUtil* util);

    void Stop();

    bool IsRunning() const;

private:
    PageSocketHandler();

    void ProcessMsg();

    void HandleAccept();

private:
    /*flag for io running*/
    volatile bool mIoRunning;
    /*logger, from paged*/
    DCSSLogPtr mLogger;
    /*util as page engine*/
    IPageSocketUtil* mUtil;
    /*singleton*/
    static std::shared_ptr<PageSocketHandler> mPtr;
};

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGESOCKETHANDLER_H
