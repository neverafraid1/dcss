CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(engine)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-g)

LINK_DIRECTORIES(/usr/local/lib/unit)
LINK_DIRECTORIES(/usr/local/lib/boost)

SET(LIB_SOURCE_FILES PageEngine.h PageEngine.cpp PageSocketHandler.h PageSocketHandler.cpp PageServiceTask.h PageServiceTask.cpp PageCommStruct.h PageSocketStruct.h main.cpp)
LINK_LIBRARIES(unit dcsslog boost_system boost_filesystem boost_regex log4cplus pthread)
ADD_EXECUTABLE(${PROJECT_NAME} ${LIB_SOURCE_FILES})