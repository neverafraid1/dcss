cmake_minimum_required(VERSION 3.10)
project(DigitalCurrencyStrategySystem)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INSTALL_PREFIX "/opt/dcss/master")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_HASH)

string(TIMESTAMP BUILDTIME "%Y%m%d%H%M%S")

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "DigitalCurrencyStrategySystem")
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "ZWANG@SOFTWIN")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")

set(CPACK_GENERATOR "RPM")
set(CPACK_RPM_PACKAGE_RELEASE "${BUILDTIME}")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_RPM_PACKAGE_GROUP "${PROJECT_NAME}")
set(CPACK_PACKAGE_DEFAULT_LOCATION "/opt/dcss/master")
set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/dcss/master")

find_package(Boost 1.67.0 COMPONENTS locale date_time filesystem system regex REQUIRED)

add_subdirectory(GateWay)
add_subdirectory(UnitEngine)
add_subdirectory(Strategy)

set(INFO_TXT_IN "${CMAKE_CURRENT_SOURCE_DIR}/rpm/info.txt")
set(INFO_TXT "${CMAKE_CURRENT_SOURCE_DIR}/rpm/info.txt")
configure_file(${INFO_TXT_IN} ${INFO_TXT})
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/rpm/info.txt")

set(CPACK_RPM_PRE_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/rpm/scripts/pre_install.sh")
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/rpm/scripts/post_install.sh")
set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/rpm/scripts/pre_uninstall.sh")
set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/rpm/scripts/post_uninstall.sh")

install(DIRECTORY "rpm/bin/" DESTINATION "bin")
install(DIRECTORY "rpm/etc/" DESTINATION "etc")

set(CPACK_RPM_PACKAGE_AUTOREQ "no")
include(CPack)