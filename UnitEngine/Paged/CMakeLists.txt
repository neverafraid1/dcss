cmake_minimum_required(VERSION 3.10)
project(engine)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INSTALL_PREFIX "/opt/dcss/master")
add_compile_options(-g)

link_directories(/opt/dcss/master/lib/unit)
link_directories(/usr/local/lib/boost)

set(LIB_SOURCE_FILES PageEngine.h PageEngine.cpp PageSocketHandler.h PageSocketHandler.cpp PageServiceTask.h PageServiceTask.cpp PageCommStruct.h PageSocketStruct.h main.cpp)
link_libraries(unit dcsslog boost_system boost_filesystem boost_regex log4cplus pthread)
add_executable(${PROJECT_NAME} ${LIB_SOURCE_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION "bin")