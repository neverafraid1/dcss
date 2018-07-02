//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGEHEADER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGEHEADER_H

namespace UNIT
{
#include <cstdint>
#include "constants.h"

#define UNIT_PAGE_STATUS_RAW    0
#define UNIT_PAGE_STATUS_INITED 1

struct PageHeader
{
    uint8_t Status;

    char UnitName[MAX_UNIT_NAME_LENGTH];

    short PageNum;

    long StartNano;

    long CloseNano;

    int FrameNum;

    int LastPos;

    short FrameVersion;
} __attribute__((packed));

enum PageStatus
{
    PAGE_RAW = 0,
    PAGE_INITED = 1,
};
}

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGEHEADER_H
