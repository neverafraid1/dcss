CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(dcssstrategy)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-g)

SET(STRATEGY_SOURCE_FILES DCSSDataWrapper.cpp DCSSDataWrapper.h IDCSSStrategy.h IDCSSStrategy.cpp DCSSStrategyUtil.h DCSSStrategyUtil.cpp IDCSSDataProcessor.h ../GateWay/Dao/SymbolDao.hpp ../GateWay/Dao/Dao.hpp ../include/EnumClassHash.h)

include_directories(../include)
LINK_DIRECTORIES(/usr/local/lib/unit/)
ADD_LIBRARY(${PROJECT_NAME} SHARED ${STRATEGY_SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} dcsslog ssl crypto unit boost_system)

INSTALL(FILES IDCSSDataProcessor.h IDCSSStrategy.h DCSSDataWrapper.h DCSSStrategyUtil.h ../include/EnumClassHash.h DESTINATION "include")
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "lib/unit")