cmake_minimum_required(VERSION 3.10)
project(dcsstg)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INSTALL_PREFIX "/opt/dcss/master")

set(CMAKE_CXX_FLAGS -g)

set(TD_SOURCE_FILES ITGEngine.h ITGEngine.cpp ../Base/IEngine.cpp ../Base/IEngine.h ../Base/Helper.h ITGApi.h ITGApi.cpp)

set(OK_SOURCE_FILES OKEX/OKTGApi.cpp OKEX/OKTGApi.h ../MD5/MD5.cpp ../MD5/MD5.h Binance/BinaTGApi.h Binance/BinaTGApi.cpp)

set(BINA_SOURCE_FILES ../Dao/SymbolDao.hpp ../Dao/Dao.hpp Binance/BinaTGApi.h Binance/BinaTGApi.cpp)

link_directories(/usr/local/lib/unit)
include_directories(../MD5 ../Base ../Dao ../../include /usr/include/mysql)

link_libraries(cpprest dcsslog boost_system ssl crypto unit log4cplus pthread boost_filesystem boost_regex mysqlpp)

add_executable(dcsstg main.cpp ${TD_SOURCE_FILES} ${BINA_SOURCE_FILES} ${OK_SOURCE_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION "bin")
install(FILES ../../include/EnumClassHash.h DESTINATION "include")
