//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_UNITHANDLER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_UNITHANDLER_H

#include "UnitDeclare.h"

/**
 *                       | - UnitReader
 * UnitHandler          -|
 *       |               | - UnitWriter
 *       |
 *       | - add unit
 *       | - connect to PageEngine
 */

UNIT_NAMESPACE_START

PRE_DECLARE_PTR(PageProvider);
PRE_DECLARE_PTR(Unit);

/**
 * basic class of unit reader / writer
 */
class UnitHandler
{
public:
    UnitHandler(PageProviderPtr ptr)
    : mPageProvider(ptr)
    {}

    virtual ~UnitHandler();
    /*return the unit's index in the vector*/
    virtual size_t AddUnit(const std::string& dir, const std::string& uname);
    /*get default name*/
    static std::string GetDefaultName(const std::string& prefix);

protected:

    PageProviderPtr mPageProvider;
//    UnitPtr mCurUnit;
    /*Units*/
    std::vector<UnitPtr> mUnits;
};

DECLARE_PTR(UnitHandler);

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_UNITHANDLER_H
