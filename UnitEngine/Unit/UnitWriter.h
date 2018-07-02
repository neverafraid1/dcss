//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_UNITWRITER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_UNITWRITER_H

#include "UnitHandler.h"
#include "FrameHeader.h"


class UnitWriter;
DECLARE_PTR(UnitWriter);


class UnitWriter : public UnitHandler
{
public:
    /** init unit */
    void Init(const std::string& dir, const std::string& uname);
    /** get current page number */
    short GetPageNum() const;
    /**
     * seek to the end of the unit
     * an unit can only be appended in the back
     */
    void SeekToEnd();
    /** write a string into unit */
    long WriteStr(const std::string& str);
    /** write a frame with full information */
    virtual long WriteFrameFull(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source, FH_MSG_TP_TYPE msgType,
            FH_REQID_TYPE requestId, FH_NANO_TYPE extraNano, FH_ERRORID_TYPE errorId, const char* errorMsg);

    virtual long WriteFrame(const void* data, FH_LENGTH_TYPE length,
            FH_SOURCE_TYPE source, FH_MSG_TP_TYPE msgType, FH_REQID_TYPE requestId)
    {
        return WriteFrameFull(data, length, source, msgType, requestId, 0, 0, nullptr);
    }

    virtual long WriteFrameExtra(const void* data, FH_LENGTH_TYPE length,
            FH_SOURCE_TYPE source, FH_MSG_TP_TYPE msgType, FH_REQID_TYPE requestId, FH_NANO_TYPE extraNano)
    {
        return WriteFrameFull(data, length, source, msgType, requestId, extraNano, 0, nullptr);
    }

    virtual long WriteErrorFrame(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source,
            FH_MSG_TP_TYPE msgType, FH_REQID_TYPE requestId,
            FH_ERRORID_TYPE errorId, const char* errorMsg)
    {
        return WriteFrameFull(data, length, source, msgType, requestId, 0, errorId, errorMsg);
    }

public:
    static UnitWriterPtr Create(const std::string& dir, const std::string& uname, const std::string& writerName);
    static UnitWriterPtr Create(const std::string& dir, const std::string& uname, PageProviderPtr ptr);
    static UnitWriterPtr Create(const std::string& dir, const std::string& uname);

    static std::string PREFIX;


protected:
    UnitPtr mUnit;

    UnitWriter(PageProviderPtr ptr) : UnitHandler(ptr) {}

};

class UnitSafeWriter : public UnitWriter
{
public:
    virtual long WriteFrameFull(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source, FH_MSG_TP_TYPE msgType,
            FH_REQID_TYPE requestId, FH_NANO_TYPE extraNano, FH_ERRORID_TYPE errorId, const char* errorMsg);

    static UnitWriterPtr Create(const std::string& dir, const std::string& uname, const std::string& writerName);
protected:
    UnitSafeWriter(PageProviderPtr ptr) : UnitWriter(ptr) {}
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_UNITWRITER_H
