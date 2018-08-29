cmake_minimum_required(VERSION 3.10)
project(dcsslog)

set(CMAKE_CXX_STANDARD 11)

set(LIB_SOURCE_FILES DCSSLog.cpp DCSSLog.h)

add_library(${PROJECT_NAME} SHARED ${LIB_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} log4cplus)

install(FILES DCSSLog.h DESTINATION "include")
install(TARGETS ${PROJECT_NAME} DESTINATION "lib/unit")