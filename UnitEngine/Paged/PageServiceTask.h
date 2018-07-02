//
// Created by wangzhen on 18-6-26.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_PAGESERVICETASK_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_PAGESERVICETASK_H

/**
 * Page engine needs several tasks to be done in schedule.
 * here we define tasks which can be implemented in page engine
 */

#include "Declare.h"

class PageEngine;

class PstBase
{
public:
    virtual void Go(){}
    virtual std::string GetName() const { return "Base";}
    virtual ~PstBase(){}
};
DECLARE_PTR(PstBase);

/**
 * check pid
 */
class PstPidCheck : public PstBase
{
public:
    PstPidCheck(PageEngine* pe);
    void Go();
    std::string GetName()const {return "PidCheck";}
private:
    PageEngine* engine;
};
DECLARE_PTR(PstPidCheck);

class PstTimeTick: public PstBase
{
public:
    PstTimeTick(PageEngine* pe);
    void Go();
    std::string GetName() const { return "TimeTick"; }
private:
    PageEngine* engine;
};
DECLARE_PTR(PstTimeTick);

class PstTempPage : public PstBase
{
public:
    static const std::string PageFullPath;
    PstTempPage(PageEngine* pe);
    void Go();
    std::string GetName() const {return "TempCheck";}
private:
    PageEngine* engine;
};
DECLARE_PTR(PstTempPage);

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_PAGESERVICETASK_H
