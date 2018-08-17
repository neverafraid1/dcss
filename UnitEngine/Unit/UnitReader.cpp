//
// Created by wangzhen on 18-6-20.
//

#include "UnitReader.h"
#include "PageProvider.h"
#include "Timer.h"
#include <sstream>
#include <cassert>
#include <limits>

USING_UNIT_NAMESPACE

const std::string UnitReader::PREFIX = "reader";

UnitReader::UnitReader(PageProviderPtr ptr)
: UnitHandler(std::move(ptr))
{
    mUnitMap.clear();
}

std::string UnitReader::GetFrameName() const
{
    return mCurUnit.get() == nullptr ? "" : mCurUnit->GetUnitName();
}

size_t UnitReader::AddUnit(const std::string& dir, const std::string& uname)
{
    if (mUnitMap.find(uname) != mUnitMap.end())
    {
        return mUnitMap.at(uname);
    }
    else
    {
        size_t index = UnitHandler::AddUnit(dir, uname);
        mUnitMap[uname] = index;
        return index;
    }
}

void UnitReader::JumpToStartTime(long startTime)
{
    for (auto& unit : mUnits)
        unit->SeekTime(startTime);
}

bool UnitReader::ExpireUnit(size_t idx)
{
    if (idx < mUnits.size())
    {
        mUnits.at(idx)->Expire();
        return true;
    }
    return false;
}

bool UnitReader::ExpireUnitByName(const std::string& name)
{
    if (mUnitMap.find(name) != mUnitMap.end())
    {
        return ExpireUnit(mUnitMap.at(name));
    }
    return false;
}

bool UnitReader::SeekTimeUnit(size_t idx, long nano)
{
    if (idx < mUnits.size())
    {
        mUnits.at(idx)->SeekTime(nano);
        return true;
    }
    return false;
}

bool UnitReader::SeekTimeUnitByName(const std::string& name, long nano)
{
    auto iter = mUnitMap.find(name);
    if (iter == mUnitMap.end())
        return false;
    return SeekTimeUnit(iter->second, nano);
}

FramePtr UnitReader::GetNextFrame()
{
    long minNano = std::numeric_limits<long>::max();
    void* address = nullptr;

    size_t index = 0;
    size_t tmp = 0;

    for (auto& unit : mUnits)
    {
        auto header = static_cast<FrameHeader*>(unit->LocateFrame());
        if (header != nullptr)
        {
            long nano = header->Nano;
            if (nano < minNano)
            {
                minNano = nano;
                address = header;
                index = tmp;
            }
        }
        ++tmp;
    }
    if (address != nullptr)
    {
        mCurUnit = mUnits.at(index);
        mCurUnit->PassFrame();
        return std::make_shared<Frame>(address);
    }
    else
    {
        return FramePtr();
    }
}

UnitReaderPtr UnitReader::Create(const std::vector<std::string>& dirs, const std::vector<std::string>& unames,
        long startTime, const std::string& readerName)
{
    assert(dirs.size() == unames.size());
    std::stringstream ss;
    ss << readerName << "_R";
    std::string clientName = ss.str();

    PageProviderPtr provider(new ClientPageProvider(clientName, false));
    UnitReaderPtr urp(new UnitReader(provider));

    for (size_t i = 0; i < dirs.size(); ++i)
        urp->AddUnit(dirs[i], unames[i]);

    urp->JumpToStartTime(startTime);

    return urp;
}

UnitReaderPtr UnitReader::Create(const std::string& dir, const std::string& uname, long startTime, const std::string& readerName)
{
    return Create(std::vector<std::string>({dir}), std::vector<std::string>({uname}), startTime, readerName);
}

UnitReaderPtr UnitReader::Create(const std::vector<std::string>& dirs, const std::vector<std::string>& unames,
        long startTime)
{
    return Create(dirs, unames, startTime, GetDefaultName(PREFIX));
}

UnitReaderPtr UnitReader::Create(const std::string& dir, const std::string& uname, long startTime)
{
    return Create(dir, uname, startTime, GetDefaultName(PREFIX));
}

UnitReaderPtr UnitReader::CreateReaderWithSys(const std::vector<std::string>& dirs,
        const std::vector<std::string>& unames, long startTime, const std::string& readerName)
{
    std::vector<std::string> tdirs(dirs.begin(), dirs.end());
    std::vector<std::string> tunames(unames.begin(), unames.end());
    tdirs.emplace_back(PAGED_UNIT_FOLDER);
    tunames.emplace_back(PAGED_UNIT_NAME);

    return Create(tdirs, tunames, startTime, readerName);
}

UnitReaderPtr UnitReader::CreateRevisableReader(const std::string& readerName)
{
    std::stringstream ss;
    ss << readerName << "_SR";
    std::string clientName = ss.str();
    PageProviderPtr provider(new ClientPageProvider(clientName, false, true));
    UnitReaderPtr urp =  UnitReaderPtr(new UnitReader(provider));

    urp->AddUnit(PAGED_UNIT_FOLDER, PAGED_UNIT_NAME);
    urp->JumpToStartTime(GetNanoTime());
    return urp;
}