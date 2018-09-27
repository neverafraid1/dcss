//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_UNIT_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_UNIT_H

#include "Page.h"

UNIT_NAMESPACE_START

PRE_DECLARE_PTR(Unit);
PRE_DECLARE_PTR(IPageProvider);

/**
 * the abstraction of continuous memory access
 */
class Unit
{
public:
    /** create a unit, only entrance */
    static UnitPtr CreateUnit(const std::string& dir, const std::string& uname, int serviceIdx, IPageProviderPtr providerPtr);

public:
    /** expire this unit provisionally, until reset by SeekTime */
    void Expire();
    /*seek to time*/
    void SeekTime(long time);
    /** get frame address, return nullptr if thers's no available frame */
    void* LocateFrame();
    /** move forward to next frame */
    void PassFrame();
    /** load next page */
    void LoadNextPage();
    /** get current page number */
    short GetCurPageNum() const;
    /** get unit name */
    std::string GetUnitName() const;

private:
    /** page provider for page acquire / release */
    IPageProviderPtr mPageProviderPtr;
    /** directory path */
    std::string mDirectory;
    /** unit name */
    std::string mUnitName;
    /** determine whether read its frame */
    bool mExpired;
    /** current page in use */
    PagePtr mCurPage;
    /** service index */
    int mServiceIdx;

    bool mIsWriting;

    Unit() : mExpired(false), mServiceIdx(-1), mIsWriting(false) {}
};

UNIT_NAMESPACE_END
#endif //DIGITALCURRENCYSTRATEGYSYSTEM_UNIT_H
