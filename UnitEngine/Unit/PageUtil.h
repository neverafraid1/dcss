//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGEUTIL_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGEUTIL_H

#include <string>
#include <vector>
#include "UnitDeclare.h"

UNIT_NAMESPACE_START

#define UNIT_PREFIX std::string("unit")
#define UNIT_SUFFIX std::string("")

class PageHeader;

class PageUtil
{
public:
    /**
     * direct memory manipulation without service
     * @param path
     * @param size
     * @param isWriting whether to write
     * @param quickMode true: no locking; false: mlock the memory for performance
     * @return the address of memory
     */
    static void* LoadPageBuffer(const std::string& path, int size, bool isWriting, bool quickMode);

    /**
     * release page bufferm, buffer and size need to be specified
     * @param buffer
     * @param size
     * @param quickMode true: no locking; false: munlock the memory
     */
    static void ReleasePageBuffer(void* buffer, int size, bool quickMode);

    static bool FileExists(const std::string& fileName);


    static std::string GenPageFileName(const std::string& uname, short pageNum);

    static std::string GenPageFullPath(const std::string& dir, const std::string& uname, short pageNum);

    static std::string GetPageFileNamePattern(const std::string& uname);

    /*extract page number from file name*/
    static short ExtractPageNum(const std::string& fileName, const std::string& uname);

    static short GetPageNumWithTime(const std::string& dir, const std::string& uname, long time);

    static std::vector<short> GetPageNums(const std::string& dir, const std::string& uname);

    static PageHeader GetPageHeader(const std::string& dir, const std::string& uname, short pageNum);
};

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGEUTIL_H
