//
// Created by wangzhen on 18-6-20.
//

#include "UnitWriter.h"
#include "Unit.h"
#include "PageProvider.h"
#include "Timer.h"

USING_UNIT_NAMESPACE

const std::string UnitWriter::PREFIX = "writer";

void UnitWriter::Init(const std::string& dir, const std::string& uname)
{
    AddUnit(dir, uname);
    mUnit = mUnits[0];
    SeekToEnd();
}

short UnitWriter::GetPageNum() const
{
    return  mUnit->GetCurPageNum();
}

long UnitWriter::WriteFrameFull(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source,
		FH_MSG_TP_TYPE msgType, FH_LASTFG_TYPE lastFlag, FH_REQID_TYPE requestId,
		FH_NANO_TYPE extraNano, FH_ERRORID_TYPE errorId, const char* errorMsg)
{
    void* buffer = mUnit->LocateFrame();

    Frame frame(buffer);
    frame.SetSource(source);
    frame.SetMsgType(msgType);
    frame.SetLastFlag(lastFlag);
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
    PageProviderPtr provider(new ClientPageProvider(writerName, true));

    return Create(dir, uname, provider);
}

UnitWriterPtr UnitWriter::Create(const std::string& dir, const std::string& uname, PageProviderPtr ptr)
{
    UnitWriterPtr uwp = UnitWriterPtr(new UnitWriter(std::move(ptr)));
    uwp->Init(dir, uname);
    return uwp;
}

UnitWriterPtr UnitWriter::Create(const std::string& dir, const std::string& uname)
{
    return std::move(Create(dir, uname, GetDefaultName(PREFIX)));
}

void UnitWriter::SeekToEnd()
{
    mUnit->SeekTime(TIME_TO_LAST);
    mUnit->LocateFrame();
}

#include "SpinLock.h"

SpinLock gSl;

long UnitSafeWriter::WriteFrameFull(const void* data, FH_LENGTH_TYPE length, FH_SOURCE_TYPE source,
        FH_MSG_TP_TYPE msgType, FH_LASTFG_TYPE lastFlag, FH_REQID_TYPE requestId, FH_NANO_TYPE extraNano,
        FH_ERRORID_TYPE errorId, const char* errorMsg)
{
    gSl.Lock();
    long nano = UnitWriter::WriteFrameFull(data, length, source, msgType, lastFlag, requestId, extraNano, errorId, errorMsg);
    gSl.UnLock();
    return nano;
}

UnitWriterPtr UnitSafeWriter::Create(const std::string& dir, const std::string& uname, const std::string& writerName)
{
    PageProviderPtr provider(new ClientPageProvider(writerName, true));
    UnitWriterPtr uwp = UnitWriterPtr(new UnitSafeWriter(provider));
    uwp->Init(dir, uname);
    return uwp;
}
