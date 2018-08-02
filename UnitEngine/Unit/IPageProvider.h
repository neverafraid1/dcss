//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_IPAGEPROVIDER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_IPAGEPROVIDER_H

#include "UnitDeclare.h"

UNIT_NAMESPACE_START

PRE_DECLARE_PTR(Page);

class IPageProvider
{
public:
    virtual PagePtr GetPage(const std::string& dir, const std::string& uname, int serviceIdx, short pageNum) = 0;
    /*release page after using*/
    virtual void ReleasePage(void* buffer, size_t size, int serviceIdx) = 0;
    /*return true if this page is using for writing*/
    virtual bool IsWriter() const = 0;

    virtual ~IPageProvider() = default;
};

DECLARE_PTR(IPageProvider);

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_IPAGEPROVIDER_H
