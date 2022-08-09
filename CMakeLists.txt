# Require the cmake minimum version.
cmake_minimum_required(VERSION 3.16)


# Set project properties.
project(
	EasyConfigLogic
	VERSION 1.9
	DESCRIPTION "A project to easily config logic of MZTTIO"
	LANGUAGES CXX
)


# Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
	message(FATAL_ERROR "You cannot build in a source directory (or any directory with"
						"CmakeLists.txt file). Please make a build subdirectory. Feel free"
						"to remove CMakeCache.txt and CMakeFiles.")
endif()

# Set the build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()


# Only do these if this is the main project, and not if it is included
# through add_subdirectory
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

	# Let's ensure -std=c++xx instead of -std=g++xx
	set(CMAKE_CXX_EXTENSIONS OFF)

	# Set the default cxx standard
	set(CMAKE_CXX_STANDARD 17)

	# Let's nicely support folders in IDE's
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

	# Enable testing if it's in the main app.
	include (CTest)


	# # Docs only available if this is the main app
	# find_package(Doxygen)
	# if(Doxygen_FOUND)
	# 	add_subdirectory(docs)
	# else()
	# 	message(STATUS "Doxygen not found, not building docs")
	# endif()



	include(FetchContent)


endif()


# The complied library code
add_subdirectory(src)

# Standalone executable program.
add_subdirectory(standalone)

# add test
if (BUILD_TESTING)
	set(INSTALL_GTEST OFF)
	add_subdirectory(test)
endif()
