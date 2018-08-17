//
// Created by wangzhen on 18-6-19.
//

#include "UnitHandler.h"
#include "PageProvider.h"
#include "Unit.h"
#include "Timer.h"

#include <sstream>

USING_UNIT_NAMESPACE

UnitHandler::UnitHandler(UnitEngine::PageProviderPtr ptr)
        : mPageProvider(std::move(ptr))
{ }

UnitHandler::~UnitHandler()
{
    mPageProvider->ExitClient();
}

std::string UnitHandler::GetDefaultName(const std::string& prefix)
{
    std::stringstream ss;
    ss << prefix << "_" << GetNanoTime();
    return ss.str();
}

size_t UnitHandler::AddUnit(const std::string& _dir, const std::string& uname)
{
    std::string dir = _dir.back() == '/' ? _dir.substr(0, _dir.length() - 1) : _dir;

    int serviceIdx = mPageProvider->RegisterUnit(dir, uname);
    mUnits.emplace_back(Unit::CreateUnit(dir, uname, serviceIdx, mPageProvider));
    return mUnits.size() - 1;
}