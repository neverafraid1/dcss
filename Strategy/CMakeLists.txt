cmake_minimum_required(VERSION 3.10)
project(dcssstrategy)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INSTALL_PREFIX "/opt/dcss/master")
add_compile_options(-g)

set(STRATEGY_SOURCE_FILES DCSSDataWrapper.cpp DCSSDataWrapper.h IDCSSStrategy.h IDCSSStrategy.cpp DCSSStrategyUtil.h DCSSStrategyUtil.cpp IDCSSDataProcessor.h ../GateWay/Dao/SymbolDao.hpp ../GateWay/Dao/Dao.hpp ../include/EnumClassHash.h)

include_directories(../include ../UnitEngine/utils ../UnitEngine/Unit ../UnitEngine/Logger)
link_directories(/usr/local/lib/unit/)
add_library(${PROJECT_NAME} SHARED ${STRATEGY_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} dcsslog ssl crypto unit boost_system)

install(FILES IDCSSDataProcessor.h IDCSSStrategy.h DCSSDataWrapper.h DCSSStrategyUtil.h ../include/EnumClassHash.h DESTINATION "include")
install(TARGETS ${PROJECT_NAME} DESTINATION "lib/unit")