//
// Created by wangzhen on 18-6-26.
//

#include <sys/stat.h>

#include "PageServiceTask.h"
#include "PageEngine.h"
#include "PageUtil.h"
#include "Timer.h"

USING_UNIT_NAMESPACE

#define TEMP_PAGE DCSS_UNIT_FOLDER "TEMP_PAGE"
const std::string PstTempPage::PageFullPath = TEMP_PAGE;

PstPidCheck::PstPidCheck(PageEngine* pe)
: mEngine(pe)
{ }

void PstPidCheck::Go()
{
    std::vector<std::string> clientToRemove;
    for (const auto& i : mEngine->mPidClientMap)
    {
        struct stat st;
        std::stringstream ss;
        ss << "/proc/" << i.first;
        if (stat(ss.str().c_str(), &st) == -1 && errno == ENOENT)
        {
            for (const auto& name : i.second)
            {
                clientToRemove.emplace_back(name);
            }
        }
    }

    for (const auto& name : clientToRemove)
    {
        mEngine->ExitClient(name);
    }
}

PstTimeTick::PstTimeTick(UnitEngine::PageEngine* pe)
: mEngine(pe)
{ }

void PstTimeTick::Go()
{ }

PstTempPage::PstTempPage(PageEngine* pe)
: mEngine(pe)
{ }

void PstTempPage::Go()
{
    auto& fileAddrs = mEngine->mFildAddrs;
    if (fileAddrs.find(PageFullPath) == fileAddrs.end())
    {
        DCSS_LOG_INFO(mEngine->GetLogger(), "NEW TEMP PAGE: " << PageFullPath);
        void* buffer = PageUtil::LoadPageBuffer(PageFullPath, UNIT_PAGE_SIZE, true, true);
        if (buffer != nullptr)
            fileAddrs[PageFullPath] = buffer;
    }
}
