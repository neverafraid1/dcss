//
// Created by wangzhen on 18-6-8.
//

#ifndef DIGITALCURRENCYSTRATEGYSYSTEM_HELPER_H
#define DIGITALCURRENCYSTRATEGYSYSTEM_HELPER_H

#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cmath>

inline void SplitLongTime(const long& t, char* date, char* time, int& milliSec)
{
    milliSec = t%1000;
    time_t tmp = t/1000;
    tm* ptm = localtime(&tmp);

    std::stringstream ssdate, sstime;
    ssdate << (ptm->tm_year + 1900) << std::setw(2) << std::setfill('0') << (ptm->tm_mon + 1) << std::setw(2) << std::setfill('0') << ptm->tm_mday;
    sstime << std::setw(2) << std::setfill('0') << ptm->tm_hour << ":" << std::setw(2) << std::setfill('0') << ptm->tm_min << ":" << std::setw(2) << std::setfill('0') << ptm->tm_sec;

    std::string strdate = ssdate.str();
    std::string strtime = sstime.str();

    memcpy(date, strdate.c_str(), sizeof(strdate.length()));
    memcpy(time, strtime.c_str(), sizeof(strtime.length()));
}

inline void SplitTime(const time_t& t, char* date, char* time)
{
    tm* ptm = localtime(&t);

    std::stringstream ssdate, sstime;
    ssdate << (ptm->tm_year + 1900) << std::setw(2) << std::setfill('0') << (ptm->tm_mon + 1) << std::setw(2) << std::setfill('0') << ptm->tm_mday;
    sstime << std::setw(2) << std::setfill('0') << ptm->tm_hour << ":" << std::setw(2) << std::setfill('0') << ptm->tm_min << ":" << std::setw(2) << std::setfill('0') << ptm->tm_sec;

    std::string strdate = ssdate.str();
    std::string strtime = sstime.str();

    memcpy(date, strdate.c_str(), sizeof(strdate.length()));
    memcpy(time, strtime.c_str(), sizeof(strtime.length()));
}

inline bool IsEqual(const double& d1, const double& d2, double fEpsilon = 0.00001)
{
    double fDelta = std::fabs( d1 - d2 );

    if( fDelta > fEpsilon ) return false;
    else if( fDelta < fEpsilon ) return true;
    else return true;
}

#endif //DIGITALCURRENCYSTRATEGYSYSTEM_HELPER_H
