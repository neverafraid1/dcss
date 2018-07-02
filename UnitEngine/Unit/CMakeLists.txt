CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(UNIT)

INCLUDE_DIRECTORIES(/opt/kungfu/toolchain/boost-1.62.0/include/)
INCLUDE_DIRECTORIES(../utils)
INCLUDE_DIRECTORIES(../Paged)
INCLUDE_DIRECTORIES(../../include)

SET(LIB_UTIL_INCLUDE_FILES ../utils/Timer.h)
SET(LIB_UTIL_SOURCE_FILES ../utils/Timer.cpp)

SET(LIB_INCLUDE_FILES ../utils/constants.h Frame.h FrameHeader.h
        Page.h PageHeader.h)
SET(LIB_SOURCE_FILES Page.cpp)

ADD_LIBRARY(${PROJECT_NAME} SHARED Page.cpp
        ../utils/constants.h Frame.h FrameHeader.h Page.h PageHeader.h
        ../utils/Timer.cpp
        ../utils/Timer.h ../utils/Declare.h PageUtil.h PageUtil.cpp UnitHandler.h UnitHandler.cpp PageProvider.h Unit.h IStrategyUtil.h StrategySocketHandler.h StrategySocketHandler.cpp SocketStruct.h PageProvider.cpp UnitReader.h UnitReader.cpp IPageProvider.h Unit.cpp UnitWriter.h UnitWriter.cpp ../Paged/PageEngine.h ../Paged/PageSocketHandler.h ../Paged/PageSocketHandler.cpp
        StrategyUtil.cpp StrategyUtil.h)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 1.0 VERSION 1.0)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

#TARGET_LINK_LIBRARIES(${PROJECT_NAME} python ${Boost_LIBRARIES})