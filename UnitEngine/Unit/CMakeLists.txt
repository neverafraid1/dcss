CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(unit)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-g)

INCLUDE_DIRECTORIES(../utils)
INCLUDE_DIRECTORIES(../Paged)
INCLUDE_DIRECTORIES(../Logger)
INCLUDE_DIRECTORIES(../../include)

LINK_DIRECTORIES(/usr/local/lib/boost)

ADD_COMPILE_OPTIONS(-fPIC)

SET(LIB_UTIL_INCLUDE_FILES ../utils/Timer.h ../utils/constants.h ../utils/json.hpp)
SET(LIB_UTIL_SOURCE_FILES ../utils/Timer.cpp)

SET(LIB_INCLUDE_FILES ${LIB_UTIL_INCLUDE_FILES} Frame.hpp FrameHeader.h
        ./IPageProvider.h ./IStrategyUtil.h Page.h ./PageHeader.h ./PageProvider.h ./PageUtil.h
        ./StrategySocketHandler.h ./StrategyUtil.h ./Unit.h ./UnitHandler.h
        ./UnitReader.h ./UnitWriter.h ../utils/UnitDeclare.h ../utils/SpinLock.h ../utils/SysMessages.h ../../include/util.h ../../include/DataStruct.h ../../include/Constants.h)
SET(LIB_SOURCE_FILES ${LIB_UTIL_SOURCE_FILES} Page.cpp PageUtil.cpp PageProvider.cpp
        StrategyUtil.cpp Unit.cpp UnitHandler.cpp
        UnitReader.cpp UnitWriter.cpp)

ADD_LIBRARY(${PROJECT_NAME} SHARED ${LIB_INCLUDE_FILES} ${LIB_SOURCE_FILES})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} boost_regex boost_filesystem)

SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 1.0 VERSION 1.0)
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

INSTALL(FILES ${LIB_UTIL_INCLUDE_FILES} ${LIB_INCLUDE_FILES} DESTINATION "include")
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "lib/unit")