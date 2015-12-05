cmake_minimum_required(VERSION 3.1)

project(InEfAn)

include(cmake/projecthelper.cmake)
include(cmake/VersionConfGit.cmake)

SET_DESKTOP_TARGET()
DEFINE_DEFAULT_DEFINITIONS()
SETUP_COMPILER_SETTINGS(OFF)

add_subdirectory(src)