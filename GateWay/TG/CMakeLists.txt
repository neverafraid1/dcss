CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(dcsstg)

set(CMAKE_CXX_STANDARD 11)

SET(TD_SOURCE_FILES ITGEngine.h OKTGApi.cpp OKTGApi.h OKAddress.h ITGEngine.cpp ../MD5/MD5.h ../MD5/MD5.cpp)

LINK_DIRECTORIES(/usr/local/lib /opt/kungfu/toolchain/boost-1.62.0/lib)
INCLUDE_DIRECTORIES(/usr/local/include ../Logger ../MD5 ../../include /opt/kungfu/toolchain/boost-1.62.0/include/ .. ../Base)
LINK_LIBRARIES(cpprest dcsslog boost_system ssl crypto)
ADD_EXECUTABLE(dcsstg main.cpp ${TD_SOURCE_FILES})
