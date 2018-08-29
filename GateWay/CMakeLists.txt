cmake_minimum_required(VERSION 3.10)
project(GateWay)

include_directories(../include)
include_directories(../UnitEngine/Logger)
include_directories(../UnitEngine/Paged)
include_directories(../UnitEngine/Unit)
include_directories(../UnitEngine/utils)
include_directories(Dao)
include_directories(Base)

add_subdirectory(TG)
add_subdirectory(MG)