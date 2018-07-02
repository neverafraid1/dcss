//
// Created by wangzhen on 18-6-7.
//

#include "DCSSLog.h"

#include <log4cplus/initializer.h>
#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>
#include <log4cplus/loglevel.h>

const std::string LOG_CONFIGURATION_FILE = "/opt/DCSS/etc/log4cplus/default.properties";
const std::string STRATEGY_LOG_FLODER = "/opt/DCSS/log/strategy/";
const std::string LOG_CONFIGURATION_STRATEGY_FILE = "/opt/DCSS/etc/log4cplus/strategy.pattern";
static bool IsConfigured = false;
const int STRATEGY_LOG_MAX_SIZE = 10 * 1024 * 1024;
const int STRATEGY_LOG_MAX_BACKUP_INDEX = 10;

bool DCSSLog::DoConfigure(std::string ConfigureName)
{
    if (!IsConfigured)
    {
        log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(ConfigureName));
        IsConfigured = true;
        return true;
    }
    else
        return false;
}

DCSSLog::DCSSLog(std::string name)
{
    DoConfigure(LOG_CONFIGURATION_FILE);
    logger = log4cplus::Logger::getInstance(name);
}

std::shared_ptr<DCSSLog> DCSSLog::GetLogger(std::string name)
{
    return std::shared_ptr<DCSSLog>(new DCSSLog(name));
}

std::shared_ptr<DCSSLog> DCSSLog::GetStrategyLogger(std::string name, std::string logFileName)
{
    return std::shared_ptr<DCSSLog>(new DCSSStrategyLog(name, logFileName));
}

DCSSStrategyLog::DCSSStrategyLog(std::string name, std::string logFileName)
{
    if (IsConfigured)
        throw std::runtime_error("DCSSStrategyLog error : duplicate configuration!");

    std::string file = STRATEGY_LOG_FLODER + logFileName;

    std::string pattern;
    std::ifstream fin(LOG_CONFIGURATION_STRATEGY_FILE);
    std::getline(fin, pattern);
    fin.close();

    log4cplus::SharedAppenderPtr fileAppender(
        new log4cplus::RollingFileAppender(file, STRATEGY_LOG_MAX_SIZE, STRATEGY_LOG_MAX_BACKUP_INDEX)
        );
    fileAppender->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(pattern)));
    log4cplus::Logger::getRoot().addAppender(fileAppender);

    log4cplus::SharedAppenderPtr consoleAppender(new log4cplus::ConsoleAppender());
    consoleAppender->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(pattern)));
    log4cplus::Logger::getRoot().addAppender(consoleAppender);

    logger = log4cplus::Logger::getInstance(name);
    logger.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
    IsConfigured = true;
}