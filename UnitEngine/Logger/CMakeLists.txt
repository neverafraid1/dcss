CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(dcsslog)

set(CMAKE_CXX_STANDARD 11)

SET(LIB_SOURCE_FILES DCSSLog.cpp DCSSLog.h ../Paged/PageCommStruct.h ../Paged/PageServiceTask.h ../Paged/PageServiceTask.cpp ../Paged/PageEngine.cpp ../utils/SysMessages.h)

INCLUDE_DIRECTORIES(/usr/local/include)
LINK_DIRECTORIES(/usr/local/lib)
ADD_LIBRARY(${PROJECT_NAME} ${LIB_SOURCE_FILES})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} log4cplus)

#INSTALL(FILES KfLog.h DESTINATION "include")
#INSTALL(TARGETS ${PROJECT_NAME} DESTINATION "lib/yijinjing")