//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_FRAME_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_FRAME_H

#include <cstring>
#include "FrameHeader.h"
#include "UnitDeclare.h"

UNIT_NAMESPACE_START

/**
 * Basic memory unit
 * header / data / errorMsg(if needs)
 */
class Frame
{
public:
    explicit Frame(void* address);

    void SetAddress(void* address);

    void* GetAddress() const;

    FH_STATUS_TYPE GetStatus() const ;
    FH_SOURCE_TYPE GetSource() const ;
    FH_NANO_TYPE GetNano() const ;
    FH_LENGTH_TYPE GetFrameLength() const ;
    FH_LENGTH_TYPE GetHeaderLength() const ;
    FH_LENGTH_TYPE GetDataLength() const ;
    FH_MSG_TP_TYPE GetMsgType() const ;
    FH_REQID_TYPE GetRequestID() const ;
    FH_NANO_TYPE GetExtraNano() const ;
    FH_ERRORID_TYPE GetErrorID() const ;
    char* GetErrorMsg() const ;
    void* GetData() const ;

    void SetSource(FH_SOURCE_TYPE source);
    void SetNano(FH_NANO_TYPE nano);
    void SetMsgType(FH_MSG_TP_TYPE type);
    void SetReqID(FH_REQID_TYPE reqid);
    void SetExtraNano(FH_NANO_TYPE extra);
    void SetErrorData(FH_ERRORID_TYPE errorid, const char* errormsg, const void* data, int dataLength);
    void SetData(const void* data, int dataLength);
    void SetStatusWritten();
    void SetStatusPageClosed();

    /*move the mCurrFrame forward by length*/
    FH_LENGTH_TYPE Next();

private:
    /*return the address of next mCurrFrame header*/
    FrameHeader* GetNextEntry() const ;

    void SetStatus(FH_STATUS_TYPE status);

    void SetFrameLength(FH_LENGTH_TYPE length);

private:
    /*address with type*/
    FrameHeader* frame;

};

DECLARE_PTR(Frame);

inline Frame::Frame(void* address) : frame(nullptr)
{
    SetAddress(address);
}

inline FH_LENGTH_TYPE Frame::GetHeaderLength() const
{
    if (GetErrorID() == 0)
        return BASIC_FRAME_HEADER_LENGTH;
    else
        return ERROR_FRAME_HEADER_LENGTH;
}

inline FH_STATUS_TYPE Frame::GetStatus() const
{
    return frame->Status;
}

inline FH_SOURCE_TYPE Frame::GetSource() const
{
    return frame->Source;
}

inline FH_NANO_TYPE Frame::GetNano() const
{
    return frame->Nano;
}

inline FH_LENGTH_TYPE Frame::GetFrameLength() const
{
    return frame->Length;
}

inline FH_MSG_TP_TYPE Frame::GetMsgType() const
{
    return frame->MsgType;
}

inline FH_REQID_TYPE Frame::GetRequestID() const
{
    return frame->ReqId;
}

inline FH_NANO_TYPE Frame::GetExtraNano() const
{
    return frame->ExtraNano;
}

inline FH_ERRORID_TYPE Frame::GetErrorID() const
{
    return frame->ErrorID;
}

inline char* Frame::GetErrorMsg() const
{
    if (GetErrorID() == 0)
        return nullptr;
    else
        return static_cast<char*>(ADDRESS_ADD(frame, BASIC_FRAME_HEADER_LENGTH));
}

inline FH_LENGTH_TYPE Frame::GetDataLength() const
{
    return GetFrameLength() - GetHeaderLength();
}

inline void* Frame::GetData() const
{
    return GetDataLength() > 0 ? ADDRESS_ADD(frame, GetHeaderLength()) : nullptr;
}



inline void Frame::SetAddress(void* address)
{
    frame = static_cast<FrameHeader*>(address);
}

inline void* Frame::GetAddress() const
{
    return frame;
}

inline void Frame::SetStatus(FH_STATUS_TYPE status)
{
    frame->Status = status;
}

inline void Frame::SetSource(FH_SOURCE_TYPE source)
{
    frame->Source = source;
}

inline void Frame::SetNano(FH_NANO_TYPE nano)
{
    frame->Nano = nano;
}

inline void Frame::SetFrameLength(FH_LENGTH_TYPE length)
{
    frame->Length = length;
}

inline void Frame::SetMsgType(FH_MSG_TP_TYPE type)
{
    frame->MsgType = type;
}

inline void Frame::SetReqID(FH_REQID_TYPE reqid)
{
    frame->ReqId = reqid;
}

inline void Frame::SetExtraNano(FH_NANO_TYPE extra)
{
    frame->ExtraNano = extra;
}

inline void Frame::SetErrorData(FH_ERRORID_TYPE errorid, const char* errormsg, const void* data, int dataLength)
{
    if (0 == errorid)
    {
        SetData(data, dataLength);
    }
    else
    {
        frame->ErrorID = errorid;
        if (errormsg != nullptr)
            memcpy(ADDRESS_ADD(frame, BASIC_FRAME_HEADER_LENGTH), errormsg, MAX_ERROR_MSG_LENGTH);
        if (data != nullptr)
            memcpy(ADDRESS_ADD(frame, ERROR_FRAME_HEADER_LENGTH), data, (size_t)dataLength);
        SetFrameLength(ERROR_FRAME_HEADER_LENGTH + dataLength);
    }
}

inline void Frame::SetData(const void* data, int dataLength)
{
    frame->ErrorID = 0;
    memcpy(ADDRESS_ADD(frame, BASIC_FRAME_HEADER_LENGTH), data, (size_t)dataLength);
    SetFrameLength(BASIC_FRAME_HEADER_LENGTH + dataLength);
}

inline void Frame::SetStatusWritten()
{
    GetNextEntry()->Status = UNIT_FRAME_STATUS_RAW;
    SetStatus(UNIT_FRAME_STATUS_WRITTEN);
}

inline void Frame::SetStatusPageClosed()
{
    SetStatus(UNIT_FRAME_STATUS_PAGE_END);
}

inline FH_LENGTH_TYPE Frame::Next()
{
    FH_LENGTH_TYPE len = GetFrameLength();
    frame = GetNextEntry();
    return len;
}

inline FrameHeader* Frame::GetNextEntry() const
{
    return static_cast<FrameHeader*>(ADDRESS_ADD(frame, GetFrameLength()));
}

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_FRAME_H
