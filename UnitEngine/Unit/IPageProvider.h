//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_IPAGEPROVIDER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_IPAGEPROVIDER_H

#include "Declare.h"
#include "Page.h"

class IPageProvider
{
public:
    virtual PagePtr GetPage(const std::string& dir, const std::string& uname, int serviceIdx, short pageNum) = 0;

    virtual void ReleasePage(void* buffer, int size, int serviceIdx) = 0;

    virtual bool IsWriter() const = 0;

    virtual  ~IPageProvider() = default;
};

DECLARE_PTR(IPageProvider);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_IPAGEPROVIDER_H
