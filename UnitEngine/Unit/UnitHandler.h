//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_UNITHANDLER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_UNITHANDLER_H

#include <memory>
#include <vector>
#include "Declare.h"
#include "PageProvider.h"
#include "Unit.h"

/**
 *                       | - UnitReader
 * UnitHandler          -|
 *       |               | - UnitWriter
 *       |
 *       | - add unit
 *       | - connect to PageEngine
 */

class PageProvider;
class Unit;
class UnitHandler
{
public:
    UnitHandler(PageProviderPtr ptr)
    : mPageProvider(ptr)
    {}

    virtual ~UnitHandler();

    virtual size_t AddUnit(const std::string& dir, const std::string& uname);

    static std::string GetDefalutName(const std::string& prefix);

protected:
    PageProviderPtr mPageProvider;
    UnitPtr mCurUnit;
    std::vector<UnitPtr> mUnitVec;
};

DECLARE_PTR(UnitHandler);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_UNITHANDLER_H
