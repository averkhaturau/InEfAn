cmake_minimum_required(VERSION 3.1)

project(InEfAn)

set(CMAKE_INSTALL_PREFIX bin)

set(BRAND_NAME "${PROJECT_NAME}" CACHE STRING "Utility name")
set(BRAND_FULLNAME "Plezir ${PROJECT_NAME}" CACHE STRING "Full name of the product")
set(BRAND_DOMAIN "plezir.by" CACHE STRING "Company site url")
set(BRAND_COMPANYNAME "ZAO Plezir" CACHE STRING "Company name")


include(cmake/projecthelper.cmake)
include(cmake/VersionConfGit.cmake)

SET_DESKTOP_TARGET()
DEFINE_DEFAULT_DEFINITIONS()
SETUP_COMPILER_SETTINGS(OFF)

add_subdirectory(src)