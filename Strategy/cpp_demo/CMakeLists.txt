CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(strategy_demo)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-g)

SET(STRATEGY_SOURCE_FILE strategy_demo.cpp)
LINK_DIRECTORIES(/usr/local/lib/unit)
ADD_EXECUTABLE(${PROJECT_NAME} ${STRATEGY_SOURCE_FILE})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} unit dcsslog log4cplus dcssstrategy boost_filesystem boost_regex boost_system)
