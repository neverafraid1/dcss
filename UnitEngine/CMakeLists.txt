cmake_minimum_required(VERSION 3.10)
project(UnitEngine)

include_directories(../include)
include_directories(Unit)
include_directories(Logger)
include_directories(Paged)
include_directories(utils)

add_subdirectory(Logger)
add_subdirectory(Unit)
add_subdirectory(Paged)