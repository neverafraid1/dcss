cmake_minimum_required(VERSION 3.10)
project(dcssmg)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INSTALL_PREFIX "/opt/dcss/master")

add_compile_options(-g)

set(MG_SOURCE_FILES IMGEngine.h IMGEngine.cpp OKEX/OKMGApi.h ../MD5/MD5.h ../MD5/MD5.cpp main.cpp OKEX/OKMGApi.cpp ../Base/IEngine.h ../Base/IEngine.cpp Binance/BinaMGApi.h Binance/BinaMGApi.cpp ../Dao/Dao.hpp ../Dao/SymbolDao.hpp)

include_directories(../MD5 ../../include ../Base ../Dao /usr/include/mysql ../Common)
link_directories(/opt/dcss/master/lib/unit)
link_libraries(cpprest dcsslog boost_system ssl crypto log4cplus unit mysqlpp)
add_executable(dcssmg main.cpp ${MG_SOURCE_FILES})

install(TARGETS ${PROJECT_NAME} DESTINATION "bin")
install(FILES ../../include/EnumClassHash.h DESTINATION "include")
