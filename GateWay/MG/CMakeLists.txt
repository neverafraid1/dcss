CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(dcssmg)

set(CMAKE_CXX_STANDARD 11)

add_compile_options(-g)

SET(MG_SOURCE_FILES IMGEngine.h IMGEngine.cpp OKEX/OKMGApi.h ../MD5/MD5.h ../MD5/MD5.cpp main.cpp OKEX/OKMGApi.cpp ../Base/IEngine.h ../Base/IEngine.cpp Binance/BinaMGApi.h Binance/BinaMGApi.cpp ../Dao/Dao.hpp ../Dao/SymbolDao.hpp)

INCLUDE_DIRECTORIES(../MD5 ../../include ../Base ../Dao /usr/include/mysql)
LINK_DIRECTORIES(/usr/local/lib/unit)
LINK_LIBRARIES(cpprest dcsslog boost_system ssl crypto log4cplus unit mysqlpp)
ADD_EXECUTABLE(dcssmg main.cpp ${MG_SOURCE_FILES})
