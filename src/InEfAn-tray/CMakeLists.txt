cmake_minimum_required(VERSION 2.8)

project(InEfAnTray)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(VERS_CONF ${CMAKE_CURRENT_SOURCE_DIR}/Version.inl.conf)
set(VERS_FILE ${CMAKE_CURRENT_BINARY_DIR}/Version.inl)


set(SRCS
	app-id.h
	events-logging.h
	events-logging.cpp
	backend-bridge.h
	guid.h
	resource-App.h
	single-instance.h
	tray-helpers.cpp
	tray-helpers.h
	win-firewall.cpp
	win-firewall.h
	win-reg.cpp
	win-reg.h

	${VERS_CONF}
	${VERS_FILE}
)

add_library(libInEfAn ${SRCS})
add_executable(${PROJECT_NAME} WIN32 Win32TrayApp.cpp Win32TrayApp.rc)
set_property(TARGET libInEfAn PROPERTY FOLDER "main")
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "main")

target_link_libraries(${PROJECT_NAME} libInEfAn ${ADDITIONAL_LIBS} logger ActiveAppTracker InputHooker_ Rpcrt4.lib)

VersionConf(${PROJECT_NAME} ${VERS_CONF} ${VERS_FILE})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND 
		mt -manifest "${CMAKE_CURRENT_SOURCE_DIR}/win.manifest" -outputresource:"$<TARGET_FILE:${PROJECT_NAME}>"
)

# ====== installer settings ======


set(INSTALLER_COMPONENT ${PROJECT_NAME})
list(APPEND INSTALLER_COMPONENTS ${INSTALLER_COMPONENT})
set(INSTALLER_COMPONENTS ${INSTALLER_COMPONENTS} PARENT_SCOPE)

set(INSTALLER_${INSTALLER_COMPONENT}_EXTRA_INSTALL_COMMANDS "
WriteRegStr HKCU \\\"SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Run\\\" \\\"${BRAND_NAME}\\\" \\\"\\\$INSTDIR\\\\${INSTALLER_COMPONENT}.exe\\\"
Exec \\\"\\\$INSTDIR\\\\${INSTALLER_COMPONENT}.exe\\\"
" PARENT_SCOPE)

#set(CPACK_NSIS_MUI_FINISHPAGE_RUN \\\"\\\$INSTDIR\\\\${INSTALLER_COMPONENT}.exe\\\")

set(INSTALLER_${INSTALLER_COMPONENT}_EXTRA_UNINSTALL_COMMANDS "
DeleteRegValue  HKCU \\\"SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Run\\\" \\\"${BRAND_NAME}\\\"
;Push \\\"${BRAND_NAME}TRAYAGENT\\\"
;Call un.CloseProgramGracefully
ExecWait \\\"taskkill /S localhost /IM ${INSTALLER_COMPONENT}.exe\\\"
" PARENT_SCOPE)

set(CPACK_NSIS_EXEC_${INSTALLER_COMPONENT}_AT_FINISH_PAGE "\\\$INSTDIR\\\\${PROJECT_NAME}.exe" PARENT_SCOPE)
set(CPACK_NSIS_EXEC_${INSTALLER_COMPONENT}_AT_FINISH_PAGE_TEXT "Launch ${BRAND_FULLNAME}" PARENT_SCOPE)
set(CPACK_PACKAGE_${INSTALLER_COMPONENT}_STARTMENU_FILE "${PROJECT_NAME}" PARENT_SCOPE)
set(CPACK_PACKAGE_${INSTALLER_COMPONENT}_STARTMENU_NAME "${BRAND_FULLNAME}" PARENT_SCOPE)



install(TARGETS ${PROJECT_NAME} DESTINATION . COMPONENT ${INSTALLER_COMPONENT})

#INSTALL_DEBUG_INFO_FILE()

#SignCodeWithCertificate()
