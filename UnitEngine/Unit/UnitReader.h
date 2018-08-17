//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_UNITREADER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_UNITREADER_H

#include "UnitHandler.h"
#include "Frame.hpp"
#include "Unit.h"

UNIT_NAMESPACE_START

PRE_DECLARE_PTR(UnitReader);

/**
 * unit reader
 */
class UnitReader : public UnitHandler
{
public:
    static UnitReaderPtr Create(const std::vector<std::string>& dirs,
            const std::vector<std::string>& unames,
            long startTime,
            const std::string& readerName);

    static UnitReaderPtr Create(const std::string& dir,
            const std::string& uname,
            long startTime,
            const std::string& readerName);

    static UnitReaderPtr Create(const std::vector<std::string>& dirs,
            const std::vector<std::string>& unames,
            long startTime);

    static UnitReaderPtr Create(const std::string& dir,
            const std::string& uname,
            long startTime);

    static UnitReaderPtr CreateReaderWithSys(const std::vector<std::string>& dirs,
            const std::vector<std::string>& unames,
            long startTime,
            const std::string& readerName);

    static UnitReaderPtr CreateRevisableReader(const std::string& readerName);

public:
    /*get Next mCurrFrame*/
    FramePtr GetNextFrame();
    /*keep the last time's GetNextFrame's source*/
    std::string GetFrameName() const;
    /*override UnitHandler's AddUnit*/
    size_t AddUnit(const std::string& dir, const std::string& uname) override ;
    /*all units jump to start time*/
    void JumpToStartTime(long startTime);
    /*expire one unit by index*/
    bool ExpireUnit(size_t idx);
    /*expire one unit by name*/
    bool ExpireUnitByName(const std::string& name);
    /*seek nano-time of a unit specified by index*/
    bool SeekTimeUnit(size_t idx, long nano);
    /*seek nano-time of a unit specified by name*/
    bool SeekTimeUnitByName(const std::string& name, long nano);

public:
    static const std::string PREFIX;

private:
    /*private constructor, only can get object through create*/
    explicit UnitReader(PageProviderPtr ptr);

private:
    /*current unit in use*/
    UnitPtr mCurUnit;
    /*map: <unit short name, idx>*/
    std::map<std::string, size_t> mUnitMap;

};

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_UNITREADER_H
