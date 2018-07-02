//
// Created by wangzhen on 18-6-19.
//

#include "Page.h"
#include "PageHeader.h"
#include "PageUtil.h"
#include "Timer.h"

#include <sstream>

using namespace UNIT;

#define PAGE_INIT_POSITION sizeof(PageHeader)

Page::Page(void *buffer)
        : frame(ADDRESS_ADD(buffer, PAGE_INIT_POSITION)), buffer(buffer), position(PAGE_INIT_POSITION), frameNum(0), pageNum(-1)
{}

void Page::FinishPage()
{
    PageHeader* header = reinterpret_cast<PageHeader*>(buffer);
    header->CloseNano = GetNanoTime();
    header->FrameNum = frameNum;
    header->LastPos = position;
    frame.SetStatusPageClosed();
}


PagePtr Page::Load(const std::string& dir, const std::string& uname, short pageNum, bool isWriting,
        bool quickMode)
{
    std::string path;
    void* buffer = PageUtil::LoadPageBuffer(path, UNIT_PAGE_SIZE, isWriting, quickMode);
    if (buffer ==nullptr)
        return PagePtr();

    PageHeader* header = reinterpret_cast<PageHeader*>(buffer);
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

    PagePtr pPage = PagePtr(new Page(buffer));
    pPage->pageNum = pageNum;
    return pPage;
}