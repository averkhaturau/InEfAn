
macro(TRY_GIT_VERSIONING match_pattern delete_prefix)
	find_package(GIT QUIET)
	if(GIT_FOUND)
		execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --match ${match_pattern} RESULT_VARIABLE GIT_VERSIONING_RESULT OUTPUT_VARIABLE GIT_COM_ID)
		if(${GIT_VERSIONING_RESULT} EQUAL 0)
			# remove prefix if any
			set(delete_prefix "${delete_prefix}")
			if(delete_prefix)
				string(REGEX REPLACE "^${delete_prefix}" "" GIT_COM_ID_2 ${GIT_COM_ID})
			else()
				set(GIT_COM_ID_2 ${GIT_COM_ID})
			endif()
			# prepare to parse into list
			string(REPLACE "\n" "" GIT_COM_ID_3 ${GIT_COM_ID_2})
			string(REPLACE "-" "." GIT_COM_ID_4 ${GIT_COM_ID_3})
			string(REPLACE "." ";" GIT_COM_ID_LIST ${GIT_COM_ID_4})
			list(LENGTH GIT_COM_ID_LIST GIT_COM_ID_LIST_LEN)
			# process version numbers
			if(${GIT_COM_ID_LIST_LEN} GREATER 0)
				list(GET GIT_COM_ID_LIST 0 GIT_VERSIONING_MAJOR_VER)
			else()
				set(GIT_VERSIONING_MAJOR_VER 0)
			endif()
			if(${GIT_COM_ID_LIST_LEN} GREATER 1)
				list(GET GIT_COM_ID_LIST 1 GIT_VERSIONING_MINOR_VER1)
			else()
				set(GIT_VERSIONING_MINOR_VER1 0)
			endif()
			if(${GIT_COM_ID_LIST_LEN} GREATER 2)
				list(GET GIT_COM_ID_LIST 2 GIT_VERSIONING_MINOR_VER2)
			else()
				set(GIT_VERSIONING_MINOR_VER2 0)
			endif()
			if(${GIT_COM_ID_LIST_LEN} GREATER 3)
				list(GET GIT_COM_ID_LIST 3 GIT_VERSIONING_MINOR_VER3)
			else()
				set(GIT_VERSIONING_MINOR_VER3 0)
			endif()
			if(${GIT_COM_ID_LIST_LEN} GREATER 4)
				list(GET GIT_COM_ID_LIST 4 GIT_VERSIONING_MINOR_COMMIT)
			else()
				set(GIT_VERSIONING_MINOR_COMMIT 0)
			endif()
			set(GIT_VERSIONING_FOUND ON)
		endif()
	else(GIT_FOUND)
		set(GIT_VERSIONING_FOUND OFF)
	endif(GIT_FOUND)
endmacro(TRY_GIT_VERSIONING)


macro(VersionConf prjName from_file to_file)

set(COMPANYNAME "\"${BRAND_COMPANYNAME}\"")
set(PRODUCTNAME "\"${BRAND_FULLNAME}\"")

set(VERSION_TAG_MASK_PREFIX v)
set(VERSION_TAG_MASK "${VERSION_TAG_MASK_PREFIX}*.*.*")

TRY_GIT_VERSIONING("${VERSION_TAG_MASK}" "${VERSION_TAG_MASK_PREFIX}") # get from git tag, e.g. tag should be like v1.0.0

if(GIT_VERSIONING_FOUND)
	set(MAJOR_VER  ${GIT_VERSIONING_MAJOR_VER})
	set(MINOR_VER1 ${GIT_VERSIONING_MINOR_VER1})
	set(MINOR_VER2 ${GIT_VERSIONING_MINOR_VER2})
	set(MINOR_VER3 ${GIT_VERSIONING_MINOR_VER3})
	set(getFromGit ON)
else(GIT_VERSIONING_FOUND)	

	string(REPLACE "." ";" versionList ${BRAND_VERSION})
	list(LENGTH versionList versionLength)
	if(${versionLength} LESS 4)
		message("Please set BRAND_VERSION to 4 dot separated words, e.g. 1.0.1.124")
		set(versionList 0;0;0;0)
	endif()
	
	set(getFromGit OFF)

	list(GET versionList 0 MAJOR_VER)
	list(GET versionList 1 MINOR_VER1)
	list(GET versionList 2 MINOR_VER2)
	list(GET versionList 3 MINOR_VER3)

endif(GIT_VERSIONING_FOUND)

set(filecontent "
	include(${CMAKE_SOURCE_DIR}/cmake/VersionConfGit.cmake)
	TRY_GIT_VERSIONING(\"${VERSION_TAG_MASK}\" \"${VERSION_TAG_MASK_PREFIX}\")
	if(GIT_VERSIONING_FOUND)
		set(MAJOR_VER  \${GIT_VERSIONING_MAJOR_VER})
		set(MINOR_VER1 \${GIT_VERSIONING_MINOR_VER1})
		set(MINOR_VER2 \${GIT_VERSIONING_MINOR_VER2})
		set(MINOR_VER3 \${GIT_VERSIONING_MINOR_VER3})
	else(GIT_VERSIONING_FOUND)
		set(MAJOR_VER	${MAJOR_VER} )
		set(MINOR_VER1	${MINOR_VER1})
		set(MINOR_VER2	${MINOR_VER2})
		set(MINOR_VER3	${MINOR_VER3})
	endif(GIT_VERSIONING_FOUND)
	set(PRODUCTNAME ${PRODUCTNAME})
	set(SHORTPRODUCTNAME ${prjName})
	set(COMPANYNAME ${COMPANYNAME})
	set(PRODUCTDOMAIN ${BRAND_DOMAIN})
	set(ICON_FILE     \"${ICON_FILE}\")
	set(ICON_FILE_16bit     \"${ICON_FILE_16bit}\")
	set(BRAND_NAME ${BRAND_NAME})
	set(BRAND_FULLNAME  ${BRAND_FULLNAME})
	configure_file(\"${from_file}\" \"${to_file}\")
	file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/version.hxx\" \"#define BRAND_VERSION \\\"\${MAJOR_VER}.\${MINOR_VER1}.\${MINOR_VER2}.\${MINOR_VER3}\\\"\\n\")
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/versionConfFile.cmake" "${filecontent}")

set(BRAND_VERSION "${MAJOR_VER}.${MINOR_VER1}.${MINOR_VER2}.${MINOR_VER3}" PARENT_SCOPE)

add_custom_target(VersionConfGit_${prjName} ALL DEPENDS ${to_file})

add_custom_command(OUTPUT ${to_file}
	COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_BINARY_DIR}/versionConfFile.cmake)

set_source_files_properties(
	${to_file} "${CMAKE_CURRENT_BINARY_DIR}/version.hxx"
	PROPERTIES GENERATED TRUE
#	HEADER_FILE_ONLY TRUE
)

source_group("main" FILES "${CMAKE_CURRENT_BINARY_DIR}/version.hxx")

add_dependencies(${prjName} VersionConfGit_${prjName})

set_property(TARGET VersionConfGit_${prjName} PROPERTY FOLDER "versioning")

endmacro(VersionConf)
