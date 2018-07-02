//
// Created by wangzhen on 18-6-20.
//

#include "UnitReader.h"
#include "PageProvider.h"
#include "Timer.h"
#include <sstream>
#include <cassert>

const std::string UnitReader::PREFIX = "reader";

UnitReader::UnitReader(PageProviderPtr ptr)
: UnitHandler(ptr)
{
    mUnitMap.clear();
}

std::string UnitReader::GetFrameName() const
{
    return mCurUnit.get() ==nullptr ? "" : mCurUnit->GetUnitName();
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
    for (auto& unit : mUnitVec)
        unit->SeekTime(startTime);
}

bool UnitReader::ExpireUnit(size_t idx)
{
    if (idx < mUnitVec.size())
    {
        mUnitVec.at(idx)->Expire();
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

FramePtr UnitReader::GetNextFrame()
{
    long minNano = TIME_TO_LAST;
    void* address = nullptr;

    for (auto& unit : mUnitVec)
    {
        FrameHeader* header = reinterpret_cast<FrameHeader*>(unit->LocateFrame());
        if (header != nullptr)
        {
            long nano = header->Nano;
            if (minNano > nano || minNano == TIME_TO_LAST)
            {
                minNano = nano;
                address = header;
                mCurUnit = unit;
            }
        }
    }
    if (address != nullptr)
    {
        mCurUnit->PassFrame();
        return FramePtr(new Frame(address));
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
    ss << readerName << "_read";
    std::string clientName = ss.str();

    PageProviderPtr provider = PageProviderPtr(new ClientPageProvider(clientName, false));
    UnitReaderPtr urp = UnitReaderPtr(new UnitReader(provider));

    for (size_t i = 0; i < dirs.size(); ++i)
        urp->AddUnit(dirs[i], unames[i]);

    urp->JumpToStartTime(startTime);

    return urp;
}

UnitReaderPtr UnitReader::Create(std::string dir, std::string uname, long startTime, const std::string& readerName)
{
    return Create(std::vector<std::string>({dir}), std::vector<std::string>({uname}), startTime, readerName);
}

UnitReaderPtr UnitReader::Create(const std::vector<std::string>& dirs, const std::vector<std::string>& unames,
        long startTime)
{
    return Create(dirs, unames, startTime, GetDefalutName(PREFIX));
}

UnitReaderPtr UnitReader::Create(std::string dir, std::string uname, long startTime)
{
    return Create(dir, uname, startTime, GetDefalutName(PREFIX));
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