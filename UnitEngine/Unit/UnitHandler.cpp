//
// Created by wangzhen on 18-6-19.
//

#include "UnitHandler.h"
#include "PageProvider.h"
#include "Unit.h"
#include "Timer.h"

#include <sstream>

using namespace UNIT;


std::string UnitHandler::GetDefalutName(const std::string& prefix)
{
    std::stringstream ss;
    ss << prefix << "_" << GetNanoTime();
    return ss.str();
}

size_t UnitHandler::AddUnit(const std::string& dir, const std::string& uname)
{

}