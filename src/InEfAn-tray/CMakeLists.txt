cmake_minimum_required(VERSION 2.8)

project(InEfAnTray)

include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

set(VERS_CONF ${CMAKE_CURRENT_SOURCE_DIR}/Version.inl.conf)
set(VERS_FILE ${CMAKE_CURRENT_BINARY_DIR}/Version.inl)


set(SRCS
	events-logging.h
	events-logging.cpp
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
;\\\${If} \\\${RunningX64}
	SetRegView 64
	WriteRegStr HKLM \\\"SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Run\\\" \\\"${BRAND_NAME}\\\" \\\"\\\$INSTDIR\\\\${INSTALLER_COMPONENT}.exe\\\"
;\\\${Else}
;	SetRegView 32
;	WriteRegStr HKLM \\\"SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Run\\\" \\\"${BRAND_NAME}\\\" \\\"\\\$INSTDIR\\\\${INSTALLER_COMPONENT}.exe\\\"
;\\\${EndIf}
Exec \\\"\\\$INSTDIR\\\\${INSTALLER_COMPONENT}.exe\\\"
" PARENT_SCOPE)

set(INSTALLER_${INSTALLER_COMPONENT}_EXTRA_UNINSTALL_COMMANDS "
;\\\${If} \\\${RunningX64}
	SetRegView 64
	DeleteRegValue  HKLM \\\"SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Run\\\" \\\"${BRAND_NAME}\\\"
;\\\${Else}
;	SetRegView 32
;	DeleteRegValue  HKLM \\\"SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Run\\\" \\\"${BRAND_NAME}\\\"
;\\\${EndIf}
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
