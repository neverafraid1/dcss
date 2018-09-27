cmake_minimum_required(VERSION 3.10)
project(unit)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INSTALL_PREFIX "/opt/dcss/master")
add_compile_options(-g)

include_directories(../utils)
include_directories(../Paged)
include_directories(../Logger)
include_directories(../../include)

link_directories(/usr/local/lib/boost)

add_compile_options(-fPIC)

set(LIB_UTIL_INCLUDE_FILES ../utils/Timer.h ../utils/json.hpp)
set(LIB_UTIL_SOURCE_FILES ../utils/Timer.cpp)

set(LIB_INCLUDE_FILES Frame.hpp FrameHeader.h
        IPageProvider.h IStrategyUtil.h Page.h PageHeader.h PageProvider.h PageUtil.h
        StrategySocketHandler.h StrategyUtil.h Unit.h UnitHandler.h
        UnitReader.h UnitWriter.h ../utils/UnitDeclare.h ../utils/SpinLock.h ../utils/SysMessages.h ../../include/util.h  ../../include/DataStruct.h ../../include/Constants.h)
set(LIB_SOURCE_FILES Page.cpp PageUtil.cpp PageProvider.cpp
        StrategyUtil.cpp Unit.cpp UnitHandler.cpp
        UnitReader.cpp UnitWriter.cpp)

add_library(${PROJECT_NAME} SHARED ${LIB_SOURCE_FILES} ${LIB_INCLUDE_FILES} ${LIB_UTIL_SOURCE_FILES} ${LIB_UTIL_INCLUDE_FILES})

target_link_libraries(${PROJECT_NAME} boost_regex boost_filesystem)

set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION 1.0 VERSION 1.0)
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

install(FILES ${LIB_UTIL_INCLUDE_FILES} ${LIB_INCLUDE_FILES} DESTINATION "include")
install(TARGETS ${PROJECT_NAME} DESTINATION "lib/unit")