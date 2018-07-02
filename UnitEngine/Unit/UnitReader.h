//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_UNITREADER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_UNITREADER_H

#include <map>
#include "UnitHandler.h"
#include "Frame.h"
#include "Unit.h"
#include "Declare.h"

class UnitReader;
DECLARE_PTR(UnitReader);

class UnitReader : public UnitHandler
{
public:
    FramePtr GetNextFrame();

    std::string GetFrameName() const;

    virtual size_t AddUnit(const std::string& dir, const std::string& uname);

    void JumpToStartTime(long startTime);

    bool ExpireUnit(size_t idx);

    bool ExpireUnitByName(const std::string& name);

public:
    static UnitReaderPtr Create(const std::vector<std::string>& dirs,
            const std::vector<std::string>& unames,
            long startTime,
            const std::string& readerName);

    static UnitReaderPtr Create(std::string dir, std::string uname, long startTime, const std::string& readerName);

    static UnitReaderPtr Create(const std::vector<std::string>& dirs,
            const std::vector<std::string>& unames,
            long startTime);

    static UnitReaderPtr Create(std::string dir, std::string uname, long startTime);

    static UnitReaderPtr CreateReaderWithSys(const std::vector<std::string>& dirs,
            const std::vector<std::string>& unames,
            long startTime,
            const std::string& readerName);

    static std::string PREFIX;

private:
    UnitReader(PageProviderPtr ptr);
private:
    /*current unit in use*/
    UnitPtr mCurUnit;
    /*map: <unit short name, idx>*/
    std::map<std::string, size_t> mUnitMap;

};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_UNITREADER_H
