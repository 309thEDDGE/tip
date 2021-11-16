#########################################################
#                       CMake Setup
#########################################################
cmake_minimum_required(VERSION 3.1...3.17)

if(${CMAKE_VERSION} VERSION_LESS 3.17)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

#Hack for Ninja generator on windows.  This has to come before project()
#see https://gitlab.kitware.com/cmake/cmake/-/issues/19401
get_property(multiConfig GLOBAL PROPERTY GENERATOR_IS_MULTICONFIG)
if (NOT multiConfig AND NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")
endif()


project(TIP
        VERSION 0.1
        DESCRIPTION "Translate Ingest Parse (TIP) for Ch10 files"
        LANGUAGES C CXX
)

set(CMAKE_PLATFORM_INDEPENDENT_CODE ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "Setting -s flag to strip symbols")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()

set(CH10_PARSE_EXE_NAME "tip_parse")
set(TRANSLATE_1553_EXE_NAME "tip_translate_1553")

#########################################################
#                      Version 
#########################################################

# Defines GIT_VERSION_STRING
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_version.txt" include_version_file)
include("${include_version_file}")
message(STATUS "Git version string: " "${GIT_VERSION_STRING}")

#########################################################
#                      Build Options
#########################################################

set(linux false)
set(container false)
set(debug 1)
set(ethernet_data true)

if (DEFINED DEBUG)
    message(STATUS "DEBUG defined")
    set(debug ${DEBUG})
endif()

if(DEFINED CONTAINER)
    message(STATUS "CONTAINER defined")
    set(ignore ${CONTAINER})
    set(container true)
endif()

get_filename_component(deps_dir "../deps" ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})
message(STATUS "Dependencies dir: " ${deps_dir})

if(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "Not defined: Defining cmake_build_type: " ${CMAKE_BUILD_TYPE})
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "Empty string: Defining cmake_build_type: " ${CMAKE_BUILD_TYPE})
endif()
message(STATUS "using cmake build type: ${CMAKE_BUILD_TYPE}")

#########################################################
#                      Build Parameters
#########################################################
set(include_dir_name "include")
set(source_dir_name "src")
set(main_dir_name "main")
set(bin_dir_name "bin")
set(build_dir "${CMAKE_CURRENT_BINARY_DIR}/cpp")
get_filename_component(out_bin_dir ${bin_dir_name} ABSOLUTE)
set(out_lib_dir ${out_bin_dir}/lib)
message(STATUS "Install dir: " ${out_bin_dir})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS OFF)

if(container)
    set(BUILD_SHARED_LIBS true)
else()
    set(BUILD_SHARED_LIBS false)
endif()

#########################################################
#                      System Specific
#########################################################

message(STATUS "System name: " ${CMAKE_SYSTEM_NAME})
if(DEFINED CONDA_PREFIX)

    add_definitions(
        -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
        -DVERSION_STRING="${GIT_VERSION_STRING}"
        -DCH10_PARSE_EXE_NAME="${CH10_PARSE_EXE_NAME}"
        -DTRANSLATE_1553_EXE_NAME="${TRANSLATE_1553_EXE_NAME}")
    message(STATUS "Setting conda build options")
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_conda.txt" include_file)
    include("${include_file}")
    
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "CYGWIN")

    message(STATUS "Setting system-specific options: " ${CMAKE_SYSTEM_NAME})
    set(CMAKE_GENERATOR "Unix Makefiles")
    set(CMAKE_CXX_COMPILER "/usr/bin/g++")
    add_compile_options("-m64")
    add_compile_definitions(__linux__)

elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")

    message(STATUS "Setting system-specific options: " ${CMAKE_SYSTEM_NAME})

    # Comment out the line below to use the default generator.
    # Note: this setting is too late in the cmake process. It must go
    # above somewhere as its not taking effect. However, "cmake -G Ninja .."
    # does create ninja build files.
    set(CMAKE_GENERATOR "Ninja")
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_windows.txt" include_file)
    include("${include_file}")

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

    set(linux true)
    message(STATUS "Setting system-specific options: " ${CMAKE_SYSTEM_NAME})
    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_linux.txt" include_file)
    include("${include_file}")

else()
    message(FATAL_ERROR "No system-specific options: " ${CMAKE_SYSTEM_NAME})
endif()

#########################################################
#                    Preprocessor Defs
#########################################################

if(linux)
    add_definitions(
        -DPARQUET_STATIC
        -DARROW_STATIC
        -DTINS_STATIC
        -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
        -DVERSION_STRING="${GIT_VERSION_STRING}"
        -DCH10_PARSE_EXE_NAME="${CH10_PARSE_EXE_NAME}"
        -DTRANSLATE_1553_EXE_NAME="${TRANSLATE_1553_EXE_NAME}")
else()
    add_compile_definitions(
        PARQUET_STATIC
        ARROW_STATIC
        TINS_STATIC
        SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
        VERSION_STRING="${GIT_VERSION_STRING}"
        CH10_PARSE_EXE_NAME="${CH10_PARSE_EXE_NAME}"
        TRANSLATE_1553_EXE_NAME="${TRANSLATE_1553_EXE_NAME}")
endif()

# Add source files dir.
if(NOT DEFINED CONDA_PREFIX)
    add_subdirectory(cpp)
endif()
