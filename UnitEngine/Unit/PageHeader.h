//
// Created by wangzhen on 18-6-19.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGEHEADER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGEHEADER_H

#include "UnitDeclare.h"

UNIT_NAMESPACE_START

#define UNIT_PAGE_STATUS_RAW    0
#define UNIT_PAGE_STATUS_INITED 1

struct PageHeader
{
    /*page status*/
    uint8_t Status;
    /*unit name*/
    char UnitName[MAX_UNIT_NAME_LENGTH];
    /*number of this page in unit*/
    short PageNum;
    /*nano time of when this page started*/
    long StartNano;
    /*nano time of when this page closed*/
    long CloseNano;
    /*there's how many frame in this page(filled when closed)*/
    int FrameNum;
    /*position of last frame*/
    int LastPos;
    /*version, for check*/
    short FrameVersion;
} __attribute__((packed));

enum PageStatus
{
    PAGE_RAW = 0,
    PAGE_INITED = 1,
};

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGEHEADER_H
