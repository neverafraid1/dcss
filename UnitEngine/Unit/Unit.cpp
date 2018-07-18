//
// Created by wangzhen on 18-6-20.
//

#include "Unit.h"
#include "PageUtil.h"
#include "IPageProvider.h"

USING_UNIT_NAMESPACE

UnitPtr Unit::CreateUnit(const std::string& dir, const std::string& uname, int serviceIdx, IPageProviderPtr providerPtr)
{
    UnitPtr p(new Unit());
    p->mPageProviderPtr = providerPtr;
    p->mDirectory = dir;
    p->mUnitName = uname;
    p->mServiceIdx = serviceIdx;
    p->mIsWriting = providerPtr->IsWriter();
    return p;
}

void Unit::Expire()
{
    if (!mExpired)
    {
        mExpired = true;
        if (mCurPage.get() != nullptr)
        {
            mPageProviderPtr->ReleasePage(mCurPage->GetBuffer(), UNIT_PAGE_SIZE, mServiceIdx);
            mCurPage.reset();
        }
        mPageProviderPtr->GetPage(mDirectory, mUnitName, mServiceIdx, -1);
    }
}

void Unit::SeekTime(long time)
{
    if (mCurPage.get() != nullptr)
        mPageProviderPtr->ReleasePage(mCurPage->GetBuffer(), UNIT_PAGE_SIZE, mServiceIdx);

    if (time == TIME_TO_LAST)
    {
        std::vector<short> pageNums = PageUtil::GetPageNums(mDirectory, mUnitName);
        mCurPage = mPageProviderPtr->GetPage(mDirectory, mUnitName, mServiceIdx, (pageNums.size() > 0) ? pageNums.back() : 1);
        if (mCurPage.get() !=nullptr)
            mCurPage->SkipWrittenFrame();
    }
    else if (time == TIME_FROM_FIRST)
    {
        std::vector<short> pageNums = PageUtil::GetPageNums(mDirectory, mUnitName);
        mCurPage = mPageProviderPtr->GetPage(mDirectory, mUnitName, mServiceIdx, (pageNums.size() > 0) ? pageNums.front() : 1);
    }
    else
    {
        short pageNum = PageUtil::GetPageNumWithTime(mDirectory, mUnitName, time);
        mCurPage = mPageProviderPtr->GetPage(mDirectory, mUnitName, mServiceIdx, pageNum);
        if (mCurPage.get() !=nullptr)
            mCurPage->SkipWrittenFrame();
    }

    mExpired = (mCurPage.get() == nullptr);
}

void Unit::LoadNextPage()
{
    if (mExpired)
        return;

    if (mCurPage.get() == nullptr)
    {
        mCurPage = mPageProviderPtr->GetPage(mDirectory, mUnitName, mServiceIdx, 1);
    }
    else
    {
        PagePtr newPage = mPageProviderPtr->GetPage(mDirectory, mUnitName, mServiceIdx, mCurPage->GetPageNum() + 1);
        if (mIsWriting)
        {
            mCurPage->FinishPage();
        }
        mPageProviderPtr->ReleasePage(mCurPage->GetBuffer(), UNIT_PAGE_SIZE, mServiceIdx);
        mCurPage = newPage;
    }
}

void* Unit::LocateFrame()
{
    if (mExpired)
        return nullptr;

    // if writing, we need an empty mCurrFrame
    if (mIsWriting)
    {
        void* frame = mCurPage->LocateWritableFrame();
        while (frame ==nullptr)
        {
            LoadNextPage();
            frame = mCurPage->LocateReadableFrame();
        }
        return frame;
    }
    // if reading, we need an written mCurrFrame
    else
    {
        if (mCurPage.get() == nullptr || mCurPage->IsAtPageEnd())
            LoadNextPage();
        if (mCurPage.get() !=nullptr)
            return mCurPage->LocateReadableFrame();
    }
    return nullptr;
}

void Unit::PassFrame()
{
    mCurPage->SkipFrame();
}

short Unit::GetCurPageNum() const
{
    return mCurPage->GetPageNum();
}

std::string Unit::GetUnitName() const
{
    return mUnitName;
}