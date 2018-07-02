//
// Created by wangzhen on 18-6-20.
//

#include "UnitWriter.h"
#include "Unit.h"
#include "PageProvider.h"
#include "Timer.h"

const std::string UnitWriter::PREFIX = "writer";

void UnitWriter::Init(const std::string& dir, const std::string& uname)
{
    AddUnit(dir, uname);
    mUnit = mUnitVec[0];
    SeekToEnd();
}

short UnitWriter::GetPageNum() const
{
    return  mUnit->GetCurPageNum();
}

long UnitWriter::WriteFrameFull(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source, FH_MSG_TP_TYPE msgType,
        FH_REQID_TYPE requestId, FH_NANO_TYPE extraNano, FH_ERRORID_TYPE errorId, const char* errorMsg)
{
    void* buffer = mUnit->LocateFrame();

    Frame frame(buffer);
    frame.SetSource(source);
    frame.SetMsgType(msgType);
    frame.SetReqID(requestId);
    frame.SetErrorData(errorId, errorMsg, data, length);
    frame.SetExtraNano(extraNano);
    long nano = GetNanoTime();
    frame.SetNano(nano);
    frame.SetStatusWritten();
    mUnit->PassFrame();
    return nano;
}

UnitWriterPtr UnitWriter::Create(const std::string& dir, const std::string& uname, const std::string& writerName)
{
    PageProviderPtr provider = PageProviderPtr(new ClientPageProvider(writerName, true));

    return Create(dir, uname, provider);
}

UnitWriterPtr UnitWriter::Create(const std::string& dir, const std::string& uname, PageProviderPtr ptr)
{
    UnitWriterPtr uwp = UnitWriterPtr(new UnitWriter(ptr));
    uwp->Init(dir, uname);
    return uwp;
}

UnitWriterPtr UnitWriter::Create(const std::string& dir, const std::string& uname)
{
    return Create(dir, uname, GetDefalutName(PREFIX));
}

void UnitWriter::SeekToEnd()
{
    mUnit->SeekTime(TIME_TO_LAST);
    mUnit->LocateFrame();
}

#include <mutex>

std::mutex gSafeWriterMutex;

long UnitSafeWriter::WriteFrameFull(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source,
        FH_MSG_TP_TYPE msgType, FH_REQID_TYPE requestId, FH_NANO_TYPE extraNano,
        FH_ERRORID_TYPE errorId, const char* errorMsg)
{
    std::lock_guard<std::mutex> lck(gSafeWriterMutex);
    return UnitWriter::WriteFrameFull(data, length, source, msgType, requestId, extraNano, errorId, errorMsg);
}

UnitWriterPtr UnitSafeWriter::Create(const std::string& dir, const std::string& uname, const std::string& writerName)
{
    PageProviderPtr provider = PageProviderPtr(new ClientPageProvider(writerName, true));
    UnitWriterPtr uwp = UnitWriterPtr(new UnitSafeWriter(provider));
    uwp->Init(dir, uname);
    return uwp;
}