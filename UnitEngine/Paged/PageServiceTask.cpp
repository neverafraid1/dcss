//
// Created by wangzhen on 18-6-26.
//

#include <sys/stat.h>
#include "PageServiceTask.h"
#include "PageEngine.h"
#include "PageUtil.h"
#include "Timer.h"

#define TEMP_PAGE DCSS_UNIT_FOLDER "TEMP_PAGE"
const std::string PstTempPage::PageFullPath = TEMP_PAGE;

PstPidCheck::PstPidCheck(PageEngine* pe)
: engine(pe)
{ }

void PstPidCheck::Go()
{
    std::vector<std::string> clientToRemove;
    for (const auto& i : engine->mPidClientMap)
    {
        struct stat st;
        std::stringstream ss;
        ss << "/proc/" << i.first;
        if (stat(ss.str().c_str(), &st) == -1 && errno == ENOENT)
        {
            for (const auto& name : i.second)
            {
                clientToRemove.push_back(name);
            }
        }
    }

    for (const auto& name : clientToRemove)
    {
        engine->ExitClient(name);
    }
}

PstTempPage::PstTempPage(PageEngine* pe)
: engine(pe)
{ }

void PstTempPage::Go()
{
    auto& fileAddrs = engine->mFildAddrs;
    if (fileAddrs.find(PageFullPath) == fileAddrs.end())
    {
        void* buffer = PageUtil::LoadPageBuffer(PageFullPath, UNIT_PAGE_SIZE, true, true);
        if (buffer != nullptr)
            fileAddrs[PageFullPath] = buffer;
    }
}