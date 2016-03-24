cmake_minimum_required(VERSION 3.1)

project(InEfAn)

set(CMAKE_INSTALL_PREFIX bin)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(BRAND_NAME "${PROJECT_NAME}" CACHE STRING "Utility name")
set(BRAND_COMPANYNAME "Pleasier" CACHE STRING "Company name")
set(BRAND_FULLNAME "${BRAND_COMPANYNAME} ${PROJECT_NAME}" CACHE STRING "Full name of the product")
set(BRAND_DOMAIN "52.29.2.208" CACHE STRING "Company site url")

set(BRAND_ICON_FILE ${CMAKE_SOURCE_DIR}/resources/mainIcon.ico)

include(cmake/projecthelper.cmake)
include(cmake/VersionConfGit.cmake)
include(cmake/integrate-boost.cmake)

SET_DESKTOP_TARGET()
DEFINE_DEFAULT_DEFINITIONS()
SETUP_COMPILER_SETTINGS(OFF)

set(CMAKE_MODULE_PATH 
	${CMAKE_MODULE_PATH} 
	${CMAKE_SOURCE_DIR}/cmake
)


include_directories(src)

INTEGRATE_BOOST()

add_subdirectory(src/input-hooker)
add_subdirectory(src/active-app-tracker)
add_subdirectory(src/logger)
add_subdirectory(src/InEfAn-tray)

add_subdirectory(src/test)

include(cmake/CPackHelper.cmake)
if(WIN32)
	install(FILES resources/mainIcon.ico DESTINATION . COMPONENT mainIcon)
	list(APPEND INSTALLER_COMPONENTS mainIcon)
endif(WIN32)

INIT_INSTALLER("NSIS" "${INSTALLER_COMPONENTS}")

set(CPACK_PACKAGE_INSTALL_DIRECTORY "${BRAND_COMPANYNAME}\\\\${PROJECT_NAME}")
if(WIN32)
#	set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
	set(CPACK_NSIS_DISPLAY_NAME "${BRAND_FULLNAME}")
endif(WIN32)

INCLUDE_CPACK()
