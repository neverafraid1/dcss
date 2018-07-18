//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGE_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGE_H

#include "FrameHeader.h"
#include "Frame.hpp"
#include "constants.h"
#include "UnitDeclare.h"

UNIT_NAMESPACE_START

PRE_DECLARE_PTR(Page);

/**
 * page class
 * handle signal page to provide continues unit abstration
 */
class Page
{
public:
    /*load page, should bo caller by PageProvider*/
    static PagePtr Load(const std::string& dir, const std::string& uname, short pageNum, bool isWriting,
            bool quickMode);

public:
    /*get page mBuffer*/
    inline void* GetBuffer() const
    { return mBuffer; }
    /*get current page num*/
    inline short GetPageNum() const
    { return mPageNum; }

    /*get writable mCurrFrame address*/
    void* LocateWritableFrame();
    /*get readable mCurrFrame address*/
    void* LocateReadableFrame();
    /*move to next mCurrFrame*/
    void SkipFrame();
    /*skip all writtern mCurrFrame*/
    void SkipWrittenFrame();
    /*skip to time*/
    void SkipToTime(long time);

    bool IsAtPageEnd() const;

    void FinishPage();

private:
    Page(void* buffer);

    inline FH_STATUS_TYPE GetCurStatus() const
    {
        return mCurrFrame.GetStatus();
    }
private:
    /*address of mmap file associated with this page*/
    void* const mBuffer;
    /*current mPosition in this page*/
    int mPosition;
    /*current mCurrFrame*/
    Frame mCurrFrame;
    /*number index of current fram in the page*/
    int mFrameNum;
    /*number of the page in a unit*/
    short mPageNum;
};

inline bool Page::IsAtPageEnd() const
{
    return GetCurStatus() == UNIT_FRAME_STATUS_PAGE_END;
}

inline void Page::SkipFrame()
{
    mPosition += mCurrFrame.Next();
    mFrameNum += 1;
}

inline void Page::SkipWrittenFrame()
{
    while (GetCurStatus() == UNIT_FRAME_STATUS_WRITTEN)
        SkipFrame();
}

inline void Page::SkipToTime(long time)
{
    while (GetCurStatus() == UNIT_FRAME_STATUS_WRITTEN
            && mCurrFrame.GetNano() < time)
        SkipFrame();
}

inline void* Page::LocateWritableFrame()
{
    SkipWrittenFrame();
    return (GetCurStatus() == UNIT_FRAME_STATUS_RAW
            && (mPosition + PAGE_MIN_HEADROOM < UNIT_PAGE_SIZE) ?
            mCurrFrame.GetAddress() : nullptr);
}

inline void* Page::LocateReadableFrame()
{
    if (GetCurStatus() == UNIT_FRAME_STATUS_WRITTEN)
        return mCurrFrame.GetAddress();
    else
        return nullptr;
//    return (GetCurStatus() == UNIT_FRAME_STATUS_WRITTEN ? mCurrFrame.GetAddress() : nullptr);
}

UNIT_NAMESPACE_END
#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGE_H
