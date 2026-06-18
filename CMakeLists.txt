# CMake file for SAD
cmake_minimum_required(VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
) 

project(sad
    VERSION 2.6.0
    DESCRIPTION "SAD - Hex Dump Utility" 
    LANGUAGES C CXX
)

# Set C and C++ standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options for build configuration
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

option(PROJECT_USES_GIT "Project is using git" ON)

set( PROJECT_SRC_DIR ${PROJECT_ROOT}/src )

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Platform detection and configuration
if(WIN32)
    message(STATUS "Configuring for Windows")
    # Check for Watcom compiler
    if(CMAKE_C_COMPILER_ID MATCHES "Watcom" OR DEFINED ENV{WATCOM})
        set(WATCOM TRUE)
        message(STATUS "Watcom compiler detected")
    endif()
elseif(APPLE)
    message(STATUS "Configuring for macOS")
elseif(UNIX)
    message(STATUS "Configuring for Linux/Unix")
endif()

# Configure the version information
include(BuildStats)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sad_version.h.in
    ${CMAKE_BINARY_DIR}/version.h
    @ONLY
)


# Use GNUInstallDirs for standard installation directories
include(GNUInstallDirs)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(
        ${PROJECT_SRC_DIR}
        ${CMAKE_BINARY_DIR}
)


# Add the source directory
add_subdirectory(src)
