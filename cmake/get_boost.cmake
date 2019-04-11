set(BOOST_REQUESTED_VERSION 1.69.0)
set(BoostSHA1 8f32d4617390d1c2d16f26a27ab60d97807b35440d45891fa340fc2648b04406)

set(Boost_FIND_COMPONENTS program_options uuid)

if(NOT Boost_FIND_COMPONENTS)
	message(FATAL_ERROR "No COMPONENTS specified for Boost")
endif()

set(BOOST_USE_STATIC_LIBS false)

# Set the library prefix and library suffix properly.
if(BOOST_USE_STATIC_LIBS)
	set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_STATIC_LIBRARY_PREFIX})
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
	set(LIBRARY_PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
	set(LIBRARY_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
else()
	set(CMAKE_FIND_LIBRARY_PREFIXES ${CMAKE_SHARED_LIBRARY_PREFIX})
	set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_SHARED_LIBRARY_SUFFIX})
	set(LIBRARY_PREFIX ${CMAKE_SHARED_LIBRARY_PREFIX})
	set(LIBRARY_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

# Create a list(string) for the build command (e.g. --with-program_options;--with-system)
# and assigns it to BOOST_COMPONENTS_FOR_BUILD
foreach(component ${Boost_FIND_COMPONENTS})
	list(APPEND BOOST_COMPONENTS_FOR_BUILD --with-${component})
endforeach()

# Create a string for the first component (e.g. boost_program_options)
# and assigns it to Boost_FIND_COMPONENTS
list(GET Boost_FIND_COMPONENTS 0 BOOST_FIRST_COMPONENT)
set(BOOST_FIRST_COMPONENT "boost_${BOOST_FIRST_COMPONENT}")

include(FindPackageHandleStandardArgs)

macro(DO_FIND_BOOST_SYSTEM)
	find_path(BOOST_INCLUDE_DIR boost/config.hpp
		PATHS /usr/local/include /usr/include
		)
	find_library(BOOST_LIBRARY
		NAMES ${BOOST_FIRST_COMPONENT}
		PATHS /usr/local/lib /usr/lib
		)
	FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost DEFAULT_MSG
		BOOST_INCLUDE_DIR BOOST_LIBRARY
		)
	set(BOOST_LIBRARIES ${BOOST_LIBRARY})
	set(BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIR})
	mark_as_advanced(BOOST_LIBRARIES BOOST_INCLUDE_DIRS)
endmacro()

macro(DO_FIND_BOOST_ROOT)
	if(NOT BOOST_ROOT_DIR)
		message(STATUS "BOOST_ROOT_DIR is not defined, using binary directory.")
		set(BOOST_ROOT_DIR ${CMAKE_BINARY_DIR}/boost)
	endif()

	find_path(BOOST_INCLUDE_DIR boost/config.hpp ${BOOST_ROOT_DIR}/include)
	find_library(BOOST_LIBRARY ${BOOST_FIRST_COMPONENT} HINTS ${BOOST_ROOT_DIR}/lib)
	FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost DEFAULT_MSG
		BOOST_INCLUDE_DIR BOOST_LIBRARY
		)
	set(BOOST_LIBRARIES ${BOOST_LIBRARY})
	set(BOOST_INCLUDE_DIRS ${BOOST_INCLUDE_DIR})
	mark_as_advanced(BOOST_LIBRARIES BOOST_INCLUDE_DIRS)
endmacro()

macro(DO_FIND_BOOST_DOWNLOAD)
	if(NOT BOOST_REQUESTED_VERSION)
		message(FATAL_ERROR "BOOST_REQUESTED_VERSION is not defined.")
	endif()

	string(REPLACE "." "_" BOOST_REQUESTED_VERSION_UNDERSCORE ${BOOST_REQUESTED_VERSION})

	set(BOOST_MAYBE_STATIC)
	if(BOOST_USE_STATIC_LIBS)
		set(BOOST_MAYBE_STATIC "link=static")
	endif()

	set(BOOST_ZIP_PATH "${BOOST_ROOT_DIR}/boost_${BOOST_REQUESTED_VERSION_UNDERSCORE}.tar.bz2")
	if(NOT EXISTS ${BOOST_ZIP_PATH})
		message(STATUS "Downloading boost ${BOOST_REQUESTED_VERSION} to ${BOOST_ROOT_DIR}")
	endif()

	message(STATUS "Boost root dir: ${BOOST_ROOT_DIR}")

	file(DOWNLOAD https://dl.bintray.com/boostorg/release/${BOOST_REQUESTED_VERSION}/source/boost_${BOOST_REQUESTED_VERSION_UNDERSCORE}.tar.bz2
			${BOOST_ZIP_PATH}
			STATUS Status
			SHOW_PROGRESS
			EXPECTED_HASH SHA1=${BoostSHA1}
	)

	execute_process(COMMAND ${CMAKE_COMMAND} -E tar xfz ${BOOST_ZIP_PATH}
			WORKING_DIRECTORY ${BoostExtractFolder}
			RESULT_VARIABLE Result
			)
	if(NOT Result EQUAL "0")
		message(FATAL_ERROR "Failed extracting boost ${BoostVersion} to ${BoostExtractFolder}")
	endif()

	#ExternalProject_Get_Property(Boost install_dir)
	#set(BOOST_INCLUDE_DIRS ${install_dir}/include)

	macro(libraries_to_fullpath varname)
		set(${varname})
		foreach(component ${Boost_FIND_COMPONENTS})
			list(APPEND ${varname} ${BOOST_ROOT_DIR}/lib/${LIBRARY_PREFIX}boost_${component}${LIBRARY_SUFFIX})
		endforeach()
	endmacro()
	libraries_to_fullpath(BOOST_LIBRARIES)

	FIND_PACKAGE_HANDLE_STANDARD_ARGS(Boost DEFAULT_MSG
		BOOST_INCLUDE_DIRS BOOST_LIBRARIES
		)
	mark_as_advanced(BOOST_LIBRARIES BOOST_INCLUDE_DIRS)
endmacro()

if(NOT BOOST_FOUND)
	DO_FIND_BOOST_ROOT()
endif()

if(NOT BOOST_FOUND)
	DO_FIND_BOOST_SYSTEM()
endif()

if(NOT BOOST_FOUND)
	DO_FIND_BOOST_DOWNLOAD()
endif()
