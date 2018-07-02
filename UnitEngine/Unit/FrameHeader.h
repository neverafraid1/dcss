//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_FRAMEHEADER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_FRAMEHEADER_H

#include <cstdint>
#define __FRAME_VERSION__ 1

typedef uint8_t     FH_STATUS_TYPE;
typedef short       FH_SOURCE_TYPE;
typedef uint8_t     FH_LASTFLAG_TYPE;
typedef long        FH_NANO_TYPE;
typedef int         FH_LENGTH_TYPE;
typedef uint32_t    FH_HASH_TYPE;
typedef short       FH_MSG_TP_TYPE;
typedef int         FH_REQID_TYPE;
typedef int         FH_ERRORID_TYPE;

#define MAX_ERROR_MSG_LENGTH 100

#define UNIT_FRAME_STATUS_RAW       0
#define UNIT_FRAME_STATUS_WRITTEN   1
#define UNIT_FRAME_STATUS_PAGE_END  2

struct FrameHeader
{
    FH_STATUS_TYPE  Status;

    FH_SOURCE_TYPE  Source;

    FH_NANO_TYPE    Nano;

    FH_LENGTH_TYPE  Length;

    FH_HASH_TYPE    Hash;

    FH_MSG_TP_TYPE  MsgType;

    FH_REQID_TYPE   ReqId;

    FH_NANO_TYPE    ExtraNano;

    FH_ERRORID_TYPE ErrorID;
} __attribute__((packed));

const int BASIC_FRAME_HEADER_LENGTH = sizeof(FrameHeader);

const int ERROR_FRAME_HEADER_LENGTH = BASIC_FRAME_HEADER_LENGTH + MAX_ERROR_MSG_LENGTH * sizeof(char);



#endif //DIGITALCURRENCYSTRATEGYSYSTEM_FRAMEHEADER_H
