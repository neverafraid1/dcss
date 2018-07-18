//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGEPROVIDER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGEPROVIDER_H

#include "IPageProvider.h"

UNIT_NAMESPACE_START

/**
 * PageProvider
 * abstract class, utilized by UnitHandler
 */
class PageProvider : public IPageProvider
{
public:
    /*register unit when added into UnitHandler*/
    virtual int RegisterUnit(const std::string& dir, const std::string& uname) { return -1; };
    /*exit client after UnitHandler is released*/
    virtual void ExitClient() {};

    bool IsWriter() const final { return mIsWriter; }

protected:
    bool mIsWriter;

    bool mReviseAllowed;
};

DECLARE_PTR(PageProvider);

/**
 * client page provider
 * provider page via memory service, socket & comm
 */
class ClientPageProvider : public PageProvider
{
public:
    ClientPageProvider(const std::string& clientName, bool isWriting, bool reviseAllowed = false);

    int RegisterUnit(const std::string& dir, const std::string& uname) override ;

    void ExitClient() override ;

    PagePtr GetPage(const std::string& dir, const std::string& uname, int serviceIdx, short pageNum) override ;

    void ReleasePage(void* buffer, int size, int serviceIdx) override ;

protected:
    /*register to service as a client*/
    void RegisterClient();

protected:
    std::string mClientName;

    void* mCommBuffer;
};

DECLARE_PTR(ClientPageProvider);

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGEPROVIDER_H
