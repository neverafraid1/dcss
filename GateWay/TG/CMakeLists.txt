CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(dcsstg)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS -g)

SET(TD_SOURCE_FILES ITGEngine.h ITGEngine.cpp ../Base/IEngine.cpp ../Base/IEngine.h ../Base/Helper.h ITGApi.h ITGApi.cpp)

#SET(DAO_SOURCE_FILES ../Dao/Dao.hpp ../Dao/SymbolDao.hpp)

SET(OK_SOURCE_FILES OKEX/OKTGApi.cpp OKEX/OKTGApi.h ../MD5/MD5.cpp ../MD5/MD5.h Binance/BinaTGApi.h Binance/BinaTGApi.cpp)

SET(BINA_SOURCE_FILES ../Dao/SymbolDao.hpp ../Dao/Dao.hpp Binance/BinaTGApi.h Binance/BinaTGApi.cpp)

LINK_DIRECTORIES(/usr/local/lib/unit)
INCLUDE_DIRECTORIES(../MD5 ../Base ../Dao ../../include /usr/include/mysql)

LINK_LIBRARIES(cpprest dcsslog boost_system ssl crypto unit log4cplus pthread boost_filesystem boost_regex mysqlpp)

ADD_EXECUTABLE(dcsstg main.cpp ${TD_SOURCE_FILES} ${BINA_SOURCE_FILES} ${OK_SOURCE_FILES})
