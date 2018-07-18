CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(dcsstg)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS -g)

SET(TD_SOURCE_FILES ITGEngine.h ITGEngine.cpp ../Base/IEngine.cpp ../Base/IEngine.h ../Base/Helper.h)

SET(OK_SOURCE_FILES ${TD_SOURCE_FILES} OKTGApi.cpp OKTGApi.h ../MD5/MD5.cpp ../MD5/MD5.h)

LINK_DIRECTORIES(/usr/local/lib/unit)
INCLUDE_DIRECTORIES(../MD5 ../Base ../../include)

LINK_LIBRARIES(cpprest dcsslog boost_system ssl crypto unit log4cplus pthread boost_filesystem boost_regex)

ADD_EXECUTABLE(dcsstg main.cpp ${OK_SOURCE_FILES})
