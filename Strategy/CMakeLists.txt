CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(dcssstrategy)

set(CMAKE_CXX_STANDARD 11)

#SET(MG_SOURCE_FILES IMGEngine.h IMGEngine.cpp OKMGApi.h ../MD5/MD5.h ../MD5/MD5.cpp main.cpp OKMGApi.cpp)
INCLUDE_DIRECTORIES(../include)
INCLUDE_DIRECTORIES(../UnitEngine/utils)
INCLUDE_DIRECTORIES(../UnitEngine/Unit)
INCLUDE_DIRECTORIES(../UnitEngine/Logger)

LINK_DIRECTORIES(/usr/local/lib /opt/kungfu/toolchain/boost-1.62.0/lib)
#INCLUDE_DIRECTORIES(/usr/local/include ../Logger ../MD5 ../../include /opt/kungfu/toolchain/boost-1.62.0/include/ .. ../Base)
LINK_LIBRARIES(cpprest dcsslog boost_system ssl crypto)
ADD_LIBRARY(${PROJECT_NAME} SHARED IDCSSDataProcessor.h IDCSSStrategy.h IDCSSStrategy.cpp DCSSDataWrapper.h DCSSStrategyUtil.h DCSSStrategyUtil.cpp DCSSDataWrapper.cpp)
TARGET_LINK_LIBRARIES(${PROJECT_NAME})
#ADD_EXECUTABLE(strategy IDCSSDataProcessor.cpp IDCSSDataProcessor.h, IDCSSStrategy.cpp IDCSSStrategy.h)
