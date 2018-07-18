//
// Created by wangzhen on 18-6-7.
//

#ifndef DEMO_DCSSLOG_H
#define DEMO_DCSSLOG_H

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include "UnitDeclare.h"

UNIT_NAMESPACE_START

#define DCSS_LOG_FATAL(DCSSlogger, content) LOG4CPLUS_FATAL(DCSSlogger->getLogger(), content)
#define DCSS_LOG_ERROR(DCSSlogger, content) LOG4CPLUS_ERROR(DCSSlogger->getLogger(), content)
#define DCSS_LOG_INFO(DCSSlogger, content) LOG4CPLUS_INFO(DCSSlogger->getLogger(), content)
#define DCSS_LOG_DEBUG(DCSSlogger, content) LOG4CPLUS_DEBUG(DCSSlogger->getLogger(), content)

PRE_DECLARE_PTR(DCSSLog);

class DCSSLog
{
protected:
    DCSSLog() {};
    DCSSLog(std::string name);

public:
    inline log4cplus::Logger& getLogger()
    {
        return logger;
    }

    inline void Fatal(const char* content)
    {
        LOG4CPLUS_FATAL(logger, content);
    }

    inline void Error(const char* content)
    {
        LOG4CPLUS_ERROR(logger, content);
    }

    inline void Info(const char* content)
    {
        LOG4CPLUS_INFO(logger, content);
    }

    inline void Debug(const char* content)
    {
        LOG4CPLUS_DEBUG(logger, content);
    }

    static bool DoConfigure(std::string ConfigureName);

    static DCSSLogPtr GetLogger(std::string name);

    static DCSSLogPtr GetStrategyLogger(std::string name, std::string logFileName);

protected:
    log4cplus::Logger logger;
};

class DCSSStrategyLog : public DCSSLog
{
public:
    DCSSStrategyLog(std::string name, std::string logFileName);
};

DECLARE_PTR(DCSSStrategyLog);

UNIT_NAMESPACE_END
#endif //DEMO_DCSSLOG_H
