macro(INTEGRATE_BOOST)
	set(BOOST_COMPONENTS ${ARGV})
	if(WIN32) 
		if(NOT BOOST_ROOT AND NOT $ENV{BOOST_ROOT} STREQUAL "")
			FILE( TO_CMAKE_PATH $ENV{BOOST_ROOT} BOOST_ROOT )
			if( NOT EXISTS ${BOOST_ROOT} )
				MESSAGE( STATUS  ${BOOST_ROOT} " does not exist. Checking if BOOST_ROOT was a quoted string.." )
				STRING( REPLACE "\"" "" BOOST_ROOT ${BOOST_ROOT} )
				if( EXISTS ${BOOST_ROOT} )
					MESSAGE( STATUS "After removing the quotes " ${BOOST_ROOT} " was now found by CMake" )
				endif( EXISTS ${BOOST_ROOT})
			endif( NOT EXISTS ${BOOST_ROOT} )

			# Save the BOOST_ROOT in the cache
			if( NOT EXISTS ${BOOST_ROOT} )
				MESSAGE( WARNING ${BOOST_ROOT} " does not exist." )
			else(NOT EXISTS ${BOOST_ROOT})
				SET (BOOST_ROOT ${BOOST_ROOT} CACHE STRING "Set the value of BOOST_ROOT to point to the root folder of your boost install." FORCE)
			endif( NOT EXISTS ${BOOST_ROOT} )

		endif(NOT BOOST_ROOT AND NOT $ENV{BOOST_ROOT} STREQUAL "")

		if(NOT BOOST_ROOT )
			MESSAGE( WARNING "Please set the BOOST_ROOT environment variable." )
		endif(NOT BOOST_ROOT )
	endif(WIN32)
	set(Boost_USE_STATIC_LIBS       ON)
	set(Boost_USE_STATIC_RUNTIME	ON)
	set(Boost_USE_MULTITHREADED     ON)

	add_definitions(-DBOOST_ALL_NO_LIB)

	find_package(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)

	IF( Boost_FOUND )
	STRING (COMPARE LESS "${Boost_VERSION}" 1.55.0 VersionIncompatible)
	IF (VersionIncompatible)
		MESSAGE(FATAL_ERROR "Boost ${Boost_VERSION} not supported. Only 1.55.0 version and higher is supported.")
	ELSE (VersionIncompatible)
		MESSAGE (STATUS "Found Boost ${Boost_VERSION} -- ${Boost_LIBRARIES}")
		INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} )
	ENDIF (VersionIncompatible)
	ENDIF()

endmacro(INTEGRATE_BOOST)

macro(INSTALL_BOOST TARGET_NAME ADDING_LIBS)
	target_link_libraries(${TARGET_NAME} "${ADDING_LIBS}" ${Boost_LIBRARIES})
endmacro(INSTALL_BOOST)
