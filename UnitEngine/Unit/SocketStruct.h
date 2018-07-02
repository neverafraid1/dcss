//
// Created by wangzhen on 18-6-20.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_SOCKETSTRUCT_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_SOCKETSTRUCT_H

#include <cstdint>
#include "../utils/constants.h"

#define MAX_SOCKET_MESSAGE_LENGTH   500
#define MAX_SOCKET_ERROR_LENGTH     100

#define PAGED_SOCKET_TD_LOGIN           1

#define PAGED_SOCKET_CONNECTION_PING    0

#define PAGED_SOCKET_STRATEGY_REGISTER  10  // register strategy
#define PAGED_SOCKET_UNIT_REGISTER      11  // register unit
#define PAGED_SOCKET_READER_REGISTER    12  // register client(reader)
#define PAGED_SOCKET_WRITER_REGISTER    13  // register client(writer)
#define PAGED_SOCKET_CLIENT_EXIT        14

#define PAGED_SOCKET_SUBSCRIBE          20
#define PAGED_SOCKET_SUBSCRIBE_TBC      21  // to be continued...

#define PAGED_SOCKET_FILE "paged.sock"

struct PagedSocketRequest
{
    /*PageSocketType*/
    uint8_t Type;
    /*name utilized for client / unit / strategy*/
    char    Name[MAX_UNIT_NAME_LENGTH];
    /*process id (only utilized when registering client)*/
    int     Pid;
    /*source id (only take effect when login trade engine)*/
    short   Source;
} __attribute__((packed));

struct PagedSocketResponse
{
    /*PageSocketType*/
    uint8_t Type;
    /*return true if success*/
    bool    Success;
    /*error messagem if failure; '\0' if success*/
    char    ErrorMsg[MAX_SOCKET_ERROR_LENGTH];
} __attribute__((packed));

struct PagedSocketRspClient : public PagedSocketResponse
{
    /*comm file*/
    char    CommFile[NAX_UNIT_FOLDER_LENGTH];
    /*size of comm file*/
    int     FileSize;
};

struct PagedSocketRspUnit : public PagedSocketResponse
{
    /*the index in the communicate file*/
    int CommIndex;
};

struct PagedSocketRspStrategy : public PagedSocketResponse
{
    /*start of request id*/
    int RidStart;
    /*end of request id*/
    int RidEnd;
};

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_SOCKETSTRUCT_H
