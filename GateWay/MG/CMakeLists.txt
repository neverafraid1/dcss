CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(dcssmg)

set(CMAKE_CXX_STANDARD 11)

SET(MG_SOURCE_FILES IMGEngine.h IMGEngine.cpp OKMGApi.h ../MD5/MD5.h ../MD5/MD5.cpp main.cpp OKMGApi.cpp)

LINK_DIRECTORIES(/usr/local/lib /opt/kungfu/toolchain/boost-1.62.0/lib)
INCLUDE_DIRECTORIES(/usr/local/include ../Logger ../MD5 ../../include /opt/kungfu/toolchain/boost-1.62.0/include/ .. ../Base)
LINK_LIBRARIES(cpprest dcsslog boost_system ssl crypto)
ADD_EXECUTABLE(dcssmg main.cpp ${MG_SOURCE_FILES})
