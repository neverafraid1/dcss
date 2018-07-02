//
// Created by wangzhen on 18-6-26.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGECOMMSTRUCT_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGECOMMSTRUCT_H

#include <cstdint>
#include <cstring>
#include "constants.h"

// Status in process 0 ~ 9
#define PAGED_COMM_RAW          0   /**< this msg block is not allocated (default) */
#define PAGED_COMM_OCCUPIED     1   /**< comm msg idx occupied (by server) */
#define PAGED_COMM_HOLDING      2   /**< folder / name ready (by client) */
#define PAGED_COMM_REQUESTING   3   /**< page number specified (by client) */
#define PAGED_COMM_ALLOCATED    4   /**< finish allocated, user may getPage (by server) */
// failures 10 ~ 19
#define PAGED_COMM_NON_EXIST    11  /**< default position */
#define PAGED_COMM_MEM_OVERFLOW 12  /**< default position */
#define PAGED_COMM_MORE_THAN_ONE_WRITER     13  /**< default position */
#define PAGED_COMM_CANNOT_RENAME_FROM_TEMP  14  /**< default position */

struct PageCommMsg
{
    /** PagedCommTypeConstants (by both server and client) */
    volatile uint8_t    Status;
    /** journal folder (by client) */
    char    Folder[NAX_UNIT_FOLDER_LENGTH];
    /** journal name (by client) */
    char    Name[MAX_UNIT_NAME_LENGTH];
    /** return true if the client is writer (by client) */
    bool    IsWriter;
    /** page number to request (by client) */
    short   PageNum;
    /** page number requested (by server) */
    short   LastPageNum;

    // operators for map key
    bool const operator == (const PageCommMsg &p) const
    {
        return PageNum == p.PageNum && strcmp(Folder, p.Folder) == 0 && strcmp(Name, p.Name) == 0;
    }

    bool const operator < (const PageCommMsg &p) const
    {
        return (strcmp(Folder, p.Folder) != 0) ? strcmp(Folder, p.Folder) < 0
                : (strcmp(Name, p.Name) != 0) ? strcmp(Name, p.Name) < 0
                        : PageNum < p.PageNum;
    }
} __attribute__((packed));

/** max number of communication users in the same time */
#define MAX_COMM_USER_NUMBER 1000
/** REQUEST_ID_RANGE * MAX_COMM_USER_NUMBER < 2147483647(max num of int) */
#define REQUEST_ID_RANGE 1000000
/** based on the max number, the comm file size is determined */
const int COMM_SIZE = MAX_COMM_USER_NUMBER * sizeof(PageCommMsg) + 1024;
/** fast type convert */
#define GET_COMM_MSG(buffer, idx) ((PageCommMsg*)(ADDRESS_ADD(buffer, idx * sizeof(PageCommMsg))))


#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGECOMMSTRUCT_H
