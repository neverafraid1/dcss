cmake_minimum_required(VERSION 3.10)
project(strategy_demo)

set(CMAKE_CXX_STANDARD 11)
add_compile_options(-g)

set(STRATEGY_SOURCE_FILE strategy_demo.cpp)
include_directories(/opt/dcss/master/include)
link_directories(/opt/dcss/master/lib/unit)
add_executable(${PROJECT_NAME} ${STRATEGY_SOURCE_FILE})
target_link_libraries(${PROJECT_NAME} unit dcsslog log4cplus dcssstrategy boost_filesystem boost_regex boost_system ta_lib)
