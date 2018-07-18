CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
PROJECT(UnitEngine)

INCLUDE_DIRECTORIES(../include)

include_directories(Unit)
include_directories(Logger)
include_directories(Paged)
include_directories(utils)

ADD_SUBDIRECTORY(Logger)
ADD_SUBDIRECTORY(Unit)
ADD_SUBDIRECTORY(Paged)