//
// Created by wangzhen on 18-6-19.
//

#include "Page.h"
#include "PageHeader.h"
#include "PageUtil.h"
#include "Timer.h"
#include <sstream>

USING_UNIT_NAMESPACE

#define PAGE_INIT_POSITION sizeof(PageHeader)

Page::Page(void *buffer)
        : mCurrFrame(ADDRESS_ADD(buffer, PAGE_INIT_POSITION)), mBuffer(buffer), mPosition(PAGE_INIT_POSITION), mFrameNum(0), mPageNum(-1)
{}

PagePtr Page::Load(const std::string& dir, const std::string& uname, short pageNum, bool isWriting,
        bool quickMode)
{
    std::string path = PageUtil::GenPageFullPath(dir, uname, pageNum);
    void* buffer = PageUtil::LoadPageBuffer(path, UNIT_PAGE_SIZE, isWriting, quickMode);
    if (buffer == nullptr)
        return PagePtr();

    auto header = static_cast<PageHeader*>(buffer);
    if (header->Status == UNIT_PAGE_STATUS_RAW)
    {
        if (!isWriting)
            return PagePtr();

        memcpy(header->UnitName, uname.c_str(), uname.length() + 1);
        header->StartNano = GetNanoTime();
        header->CloseNano = -1;
        header->PageNum = pageNum;
        header->FrameVersion = __FRAME_VERSION__;
        header->Status = UNIT_PAGE_STATUS_INITED;
    }
    else if (header->FrameVersion > 0 &&
            header->FrameVersion != __FRAME_VERSION__)
    {
        std::stringstream ss;
        ss << "page version mismatch: (program)" << __FRAME_VERSION__ << " (page)" << header->FrameVersion;
        throw std::runtime_error(ss.str().c_str());
    }

    PagePtr pPage(new Page(buffer));
    pPage->mPageNum = pageNum;
    return std::move(pPage);
}

void Page::FinishPage()
{
    auto header = static_cast<PageHeader*>(mBuffer);
    header->CloseNano = GetNanoTime();
    header->FrameNum = mFrameNum;
    header->LastPos = mPosition;
    mCurrFrame.SetStatusPageClosed();
}