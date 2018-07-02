//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGEPROVIDER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGEPROVIDER_H

#include "Declare.h"
#include "IPageProvider.h"

class Page;

class PageProvider : public IPageProvider
{
public:
    virtual int RegisterUnit(const std::string& dir, const std::string& uname) { return -1; };

    virtual void ClientExit() {};

    virtual bool IsWriter() const { return mIsWriter; }

protected:
    bool mIsWriter;

    bool mReviseAllowed;
};

DECLARE_PTR(PageProvider);

class ClientPageProvider : public PageProvider
{
public:
    ClientPageProvider(const std::string& clientName, bool isWriting, bool reviseAllowed = false);

    virtual int RegisterUnit(const std::string& dir, const std::string& uname);

    virtual void ClientExit();

    virtual PagePtr GetPage(const std::string& dir, const std::string& uname, int serviceIdx, short pageNum);

    virtual void ReleasePage(void* buffer, int size, int serviceIdx);

protected:
    void RegisterClient();

protected:
    std::string mClientName;
    void* mCommBuffer;
};

DECLARE_PTR(ClientPageProvider);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGEPROVIDER_H
