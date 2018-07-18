//
// Created by wangzhen on 18-6-26.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGESERVICETASK_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGESERVICETASK_H

/**
 * Page engine needs several tasks to be done in schedule.
 * here we define tasks which can be implemented in page engine
 */

#include "UnitDeclare.h"

UNIT_NAMESPACE_START

class PageEngine;

class PstBase
{
public:
    virtual void Go(){}
    virtual std::string GetName() const { return "Base";}
    virtual ~PstBase() = default;
};
DECLARE_PTR(PstBase);

/**
 * check pid
 */
class PstPidCheck : public PstBase
{
public:
    explicit PstPidCheck(PageEngine* pe);
    void Go() final ;
    std::string GetName() const final{return "PidCheck";}
private:
    PageEngine* mEngine;
};
DECLARE_PTR(PstPidCheck);

class PstTimeTick: public PstBase
{
public:
    explicit PstTimeTick(PageEngine* pe);
    void Go() final ;
    std::string GetName() const final{ return "TimeTick"; }
private:
    PageEngine* mEngine;
};
DECLARE_PTR(PstTimeTick);

/*
 * load temp page
 */
class PstTempPage : public PstBase
{
public:
    static const std::string PageFullPath;
    explicit PstTempPage(PageEngine* pe);
    void Go() final ;
    std::string GetName() const final {return "TempCheck";}
private:
    PageEngine* mEngine;
};
DECLARE_PTR(PstTempPage);

UNIT_NAMESPACE_END

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGESERVICETASK_H
