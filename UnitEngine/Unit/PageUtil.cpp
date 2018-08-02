//
// Created by wangzhen on 18-6-19.
//

#include "PageUtil.h"
#include "PageHeader.h"

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <fcntl.h>
#include <sys/mman.h>
#include <sstream>

USING_UNIT_NAMESPACE

std::string PageUtil::GenPageFileName(const std::string& uname, short pageNum)
{
    std::stringstream ss;
    ss << UNIT_PREFIX << "." << uname << "." << pageNum << "." << UNIT_SUFFIX;
    return ss.str();
}

std::string PageUtil::GenPageFullPath(const std::string& dir, const std::string& uname, short pageNum)
{
    std::stringstream ss;
    ss << dir << "/" << GenPageFileName(uname, pageNum);
    return ss.str();
}

std::string PageUtil::GetPageFileNamePattern(const std::string& uname)
{
    std::stringstream ss;
    ss << UNIT_PREFIX << "\\." << uname << "\\.[0-9]+\\." << UNIT_SUFFIX;
    return ss.str();
}

short PageUtil::ExtractPageNum(const std::string& fileName, const std::string& uname)
{
    return (short)std::stoi(fileName.substr(UNIT_PREFIX.length() + uname.length() + 2, fileName.length() - UNIT_SUFFIX.length()));
}

short PageUtil::GetPageNumWithTime(const std::string& dir, const std::string& uname, long time)
{
    std::vector<short> pageNums = GetPageNums(dir, uname);
    for (auto iter = pageNums.rbegin(); iter != pageNums.rend(); ++iter)
    {
        PageHeader header = GetPageHeader(dir, uname, *iter);
        if (header.StartNano < time)
            return *iter;
    }
    return 1;
}

std::vector<short> PageUtil::GetPageNums(const std::string& dir, const std::string& uname)
{
    std::string namePattern = GetPageFileNamePattern(uname);
    boost::filesystem::path p(dir);
    boost::regex pattern(namePattern);
    std::vector<short> res;
    auto range = boost::filesystem::directory_iterator(p);
    boost::filesystem::directory_iterator end;
    for (auto begin = boost::filesystem::directory_iterator(p);begin != end; ++begin)
    {
        std::string fileName = begin->path().filename().string();
        if (boost::regex_match(fileName.begin(), fileName.end(), pattern))
            res.emplace_back(ExtractPageNum(fileName, uname));
    }
    std::sort(res.begin(), res.end());
    return std::move(res);
}

void* PageUtil::LoadPageBuffer(const std::string& path, size_t size, bool isWriting,
        bool quickMode)
{
    int fd = open(path.c_str(), (isWriting) ? (O_RDWR | O_CREAT) : O_RDONLY,
            (mode_t)0600);
    if (fd < 0)
    {
        if (!isWriting)
            return nullptr;
        perror("Cannot create/write the file");
        exit(EXIT_FAILURE);
    }

    if (isWriting)
    {
        if (lseek(fd, size-1, SEEK_SET) == -1)
        {
            close(fd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }
        if (write(fd, "", 1) == -1)
        {
            close(fd);
            perror("Error writing last byte of the file");
            exit(EXIT_FAILURE);
        }
    }

    void* buffer = mmap(nullptr, size, (isWriting) ? (PROT_READ | PROT_WRITE) : PROT_READ, MAP_SHARED, fd, 0);

    if (buffer == MAP_FAILED)
    {
        close(fd);
        perror("Error mapping file to mBuffer");
        exit(EXIT_FAILURE);
    }

    if (!quickMode && madvise(buffer, size, MADV_RANDOM) != 0 && mlock(buffer, size) != 0)
    {
        munmap(buffer, size);
        close(fd);
        perror("Error in madvise or mlock");
        exit(EXIT_FAILURE);
    }

    close(fd);
    return buffer;
}

void PageUtil::ReleasePageBuffer(void* buffer, size_t size, bool quickMode)
{
    if (!quickMode && munlock(buffer, size) != 0)
    {
        perror("ERROR in munlock");
        exit(EXIT_FAILURE);
    }

    if(munmap(buffer, size)!=0)
    {
        perror("ERROR in munmap");
        exit(EXIT_FAILURE);
    }
}

bool PageUtil::FileExists(const std::string& fileName)
{
    return boost::filesystem::exists(fileName);
}

PageHeader PageUtil::GetPageHeader(const std::string& dir, const std::string& uname, short pageNum)
{
    PageHeader header = {};
    std::string path = GenPageFullPath(dir, uname, pageNum);
    FILE* fp = fopen(path.c_str(), "rb");
    if (fp == nullptr)
        perror("cannot open file in PageUtil::GetPageHeader");

    auto length = fread(&header, 1, sizeof(PageHeader), fp);
    if (length != sizeof(PageHeader))
        perror("cannot get page header in PageUtil::GetPageHeader");
    fclose(fp);
    return header;
}