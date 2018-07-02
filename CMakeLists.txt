cmake_minimum_required(VERSION 3.10)
project(DigitalCurrencyStrategySystem)

set(CMAKE_CXX_STANDARD 11)

ADD_SUBDIRECTORY(GateWay)
ADD_SUBDIRECTORY(UnitEngine)
ADD_SUBDIRECTORY(Strategy)

#add_executable(DigitalCurrencyStrategySystem main.cpp include/Constants.h include/DataStruct.h src/TG/ITGEngine.h src/TG/OKTGApi.h src/TG/OKTGApi.cpp src/Logger/DCSSLog.h src/Logger/DCSSLog.cpp src/TG/main.cpp)