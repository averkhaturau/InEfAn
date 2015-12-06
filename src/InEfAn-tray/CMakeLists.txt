cmake_minimum_required(VERSION 2.8)

project(InEfAnTray)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(VERS_CONF ${CMAKE_CURRENT_SOURCE_DIR}/Version.inl.conf)
set(VERS_FILE ${CMAKE_CURRENT_BINARY_DIR}/Version.inl)


set(SRCS
	resource-App.h
	single-instance.h
	tray-helpers.cpp
	tray-helpers.h
	Win32TrayApp.cpp
	Win32TrayApp.rc

	${VERS_CONF}
	${VERS_FILE}
)

add_executable(${PROJECT_NAME} WIN32 ${SRCS})

target_link_libraries(${PROJECT_NAME} ${ADDITIONAL_LIBS} logger ActiveAppTracker InputHooker_)

#==== setting version =====

VersionConf(${PROJECT_NAME} ${VERS_CONF} ${VERS_FILE})

# ====== installer settings ======


set(CPACK_NSIS_EXEC_App_AT_FINISH_PAGE "\\\$INSTDIR\\\\${PROJECT_NAME}.exe" PARENT_SCOPE)
set(CPACK_NSIS_EXEC_App_AT_FINISH_PAGE_TEXT "Launch ${BRAND_FULLNAME}" PARENT_SCOPE)
set(CPACK_PACKAGE_App_STARTMENU_FILE "${PROJECT_NAME}" PARENT_SCOPE)
set(CPACK_PACKAGE_App_STARTMENU_NAME "${BRAND_FULLNAME}" PARENT_SCOPE)


install(TARGETS ${PROJECT_NAME} DESTINATION . COMPONENT ${INSTALLER_COMPONENT})

#INSTALL_DEBUG_INFO_FILE()

#SignCodeWithCertificate()
