//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_UNITWRITER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_UNITWRITER_H

#include "UnitHandler.h"
#include "FrameHeader.h"

UNIT_NAMESPACE_START

PRE_DECLARE_PTR(UnitWriter);

/**
 * uint writer
 * enable user to write into unit
 * one unit writer can only one unit and
 * meanwhile this unit cannot be used by other writer
 */
class UnitWriter : public UnitHandler
{
public:
    static UnitWriterPtr Create(const std::string& dir, const std::string& uname, const std::string& writerName);
    static UnitWriterPtr Create(const std::string& dir, const std::string& uname, PageProviderPtr ptr);
    static UnitWriterPtr Create(const std::string& dir, const std::string& uname);

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
    /*no need now*/
//    long WriteStr(const std::string& str);
    /** write a frame with full information */
    virtual long WriteFrameFull(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source,
    		FH_MSG_TP_TYPE msgType, FH_LASTFG_TYPE lastFlag, FH_REQID_TYPE requestId,
			FH_NANO_TYPE extraNano, FH_ERRORID_TYPE errorId, const char* errorMsg);
    /*write a basic mCurrFrame*/
    virtual long WriteFrame(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source,
    		FH_MSG_TP_TYPE msgType, FH_LASTFG_TYPE lastFlag, FH_REQID_TYPE requestId)
    {
        return WriteFrameFull(data, length, source, msgType, lastFlag, requestId, 0, 0, nullptr);
    }
    /*write a mCurrFrame with extra nano*/
    virtual long WriteFrameExtra(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source,
    		FH_MSG_TP_TYPE msgType, FH_LASTFG_TYPE lastFlag, FH_REQID_TYPE requestId, FH_NANO_TYPE extraNano)
    {
        return WriteFrameFull(data, length, source, msgType, lastFlag, requestId, extraNano, 0, nullptr);
    }
    /*write an error mCurrFrame*/
    virtual long WriteErrorFrame(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source,
            FH_MSG_TP_TYPE msgType, FH_LASTFG_TYPE lastFlag, FH_REQID_TYPE requestId,
            FH_ERRORID_TYPE errorId, const char* errorMsg)
    {
        return WriteFrameFull(data, length, source, msgType, lastFlag, requestId, 0, errorId, errorMsg);
    }

public:
    static const std::string PREFIX;

protected:
    /*the unit will write in*/
    UnitPtr mUnit;
    /*private constructor, only can get object through create*/
    explicit UnitWriter(PageProviderPtr ptr) : UnitHandler(std::move(ptr)) {}

};

class UnitSafeWriter : public UnitWriter
{
public:
    static UnitWriterPtr Create(const std::string& dir, const std::string& uname, const std::string& writerName);

public:
    long WriteFrameFull(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source, FH_MSG_TP_TYPE msgType, FH_LASTFG_TYPE lastFlag,
            FH_REQID_TYPE requestId, FH_NANO_TYPE extraNano, FH_ERRORID_TYPE errorId, const char* errorMsg) override ;

protected:
    explicit UnitSafeWriter(PageProviderPtr ptr) : UnitWriter(std::move(ptr)) {}
};

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_UNITWRITER_H
