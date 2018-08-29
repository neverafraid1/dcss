//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_FRAMEHEADER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_FRAMEHEADER_H

#include <cstdint>
#include <cstring>
#include "constants.h"
#define __FRAME_VERSION__ 1

UNIT_NAMESPACE_START

typedef uint8_t		FH_STATUS_TYPE;
typedef uint8_t		FH_SOURCE_TYPE;
typedef long		FH_NANO_TYPE;
typedef size_t		FH_LENGTH_TYPE;
typedef short		FH_MSG_TP_TYPE;
typedef int		FH_REQID_TYPE;
typedef int		FH_ERRORID_TYPE;
typedef bool		FH_LASTFG_TYPE;

#define MAX_ERROR_MSG_LENGTH 100

//////////////////////////////////////
/// (uint8_t) UnitFrameStatus
//////////////////////////////////////
#define UNIT_FRAME_STATUS_RAW       0
#define UNIT_FRAME_STATUS_WRITTEN   1
#define UNIT_FRAME_STATUS_PAGE_END  2

struct FrameHeader
{
    /*unit mCurrFrame status*/
    volatile FH_STATUS_TYPE  Status;
    /*source of this mCurrFrame*/
    FH_SOURCE_TYPE  Source;
    /*nano time of this mCurrFrame*/
    FH_NANO_TYPE    Nano;
    /*total length (include header and errorMsg(if exist))*/
    FH_LENGTH_TYPE  Length;
    /*msg type of the data in this mCurrFrame*/
    FH_MSG_TP_TYPE  MsgType;
    /*last flag*/
    FH_LASTFG_TYPE	LastFlag;
    /*reqid*/
    FH_REQID_TYPE   ReqId;
    /*extra nano time for usage*/
    FH_NANO_TYPE    ExtraNano;
    /*error id
     * ==0 if there's no error*/
    FH_ERRORID_TYPE ErrorID;

    FrameHeader()
    {
        memset(this, 0, sizeof(FrameHeader));
    }
} __attribute__((packed));

/*length of mCurrFrame header*/
const int BASIC_FRAME_HEADER_LENGTH = sizeof(FrameHeader);
/*length of mCurrFrame header with error msg*/
const int ERROR_FRAME_HEADER_LENGTH = BASIC_FRAME_HEADER_LENGTH + MAX_ERROR_MSG_LENGTH * sizeof(char);

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_FRAMEHEADER_H
