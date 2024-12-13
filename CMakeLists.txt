# Require the cmake minimum version.
cmake_minimum_required(VERSION 3.16)


# Set project properties.
project(
	EasyConfigLogic
	VERSION 2.2.0
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
	set(CMAKE_CXX_STANDARD_REQUIRED ON)

	# Set default compile options
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wall -Wextra")

	# Let's nicely support folders in IDE's
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

	# Enable testing if it's in the main app.
	include (CTest)

	include(FetchContent)
endif()

option(BUILD_GRPC_SERVER "Build server (depend on gRPC)" ON)
option(BUILD_TESTING "Build test" ON)

if (BUILD_GRPC_SERVER)
	# search for grpc
	option(protobuf_MODULE_COMPATIABLE TRUE)
	find_package(Protobuf CONFIG REQUIRED)
	# set alias
	set(_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
	set(_REFLECTION gRPC::grpc++_reflection)
	if(CMAKE_CROSSCOMPILING)
		find_program(_PROTOBUF_PROTOC protoc)
	else()
		set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)
	endif()
	# Find gRPC installation
	# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
	find_package(gRPC CONFIG REQUIRED)
	# message(STATUS "Using gRPC ${gRPC_VERSION}")
	# set alias
	set(_GRPC_GRPCPP gRPC::grpc++)
	if(CMAKE_CROSSCOMPILING)
		find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
	else()
		set(_GRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
	endif()

endif()

# set install path
if (${CMAKE_INSTALL_PREFIX} STREQUAL "/usr/local")
	set(ECL_INSTALL_PATH "${CMAKE_BINARY_DIR}")
else()
	set(ECL_INSTALL_PATH "${CMAKE_INSTALL_PREFIX}")
endif()

# show properties
MESSAGE("Build Properties")
MESSAGE("  cross................: " ${CMAKE_CROSSCOMPILING})
MESSAGE("  target system........: " ${CMAKE_SYSTEM_NAME})
MESSAGE("  c++ compiler.........: " ${CMAKE_CXX_COMPILER})
MESSAGE("  c++ standard.........: " ${CMAKE_CXX_STANDARD})
MESSAGE("  cmake................: " ${CMAKE_VERSION})
MESSAGE("  build type...........: " ${CMAKE_BUILD_TYPE})
MESSAGE("  generator............: " ${CMAKE_GENERATOR})
MESSAGE("  root path............: " ${CMAKE_FIND_ROOT_PATH})
MESSAGE("  build server.........: " ${BUILD_GRPC_SERVER})
MESSAGE("  build test...........: " ${BUILD_TESTING})
if (BUILD_GRPC_SERVER)
	MESSAGE("  protobuf.............: " ${Protobuf_VERSION})
	MESSAGE("  grpc.................: " ${gRPC_VERSION})
endif()
MESSAGE("  install at...........: " ${ECL_INSTALL_PATH})

if (BUILD_GRPC_SERVER)
	# The protobuf files
	add_subdirectory(proto)
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
