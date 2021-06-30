#########################################################
#                       CMake Setup
#########################################################
cmake_minimum_required(VERSION 3.1...3.17)

if(${CMAKE_VERSION} VERSION_LESS 3.17)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
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
elseif("${CMAKE_BUILD_TYPE}" MATCHES "")
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "Empty string: Defining cmake_build_type: " ${CMAKE_BUILD_TYPE})
endif()

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
if(DEFINED CONDA)
    message(STATUS "Setting conda build options")
    include("CMake_conda.txt")
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

    include("CMake_windows.txt")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(linux true)
    message(STATUS "Setting system-specific options: " ${CMAKE_SYSTEM_NAME})
    include("CMake_linux.txt")
else()
    message(FATAL_ERROR "No system-specific options: " ${CMAKE_SYSTEM_NAME})
endif()

#########################################################
#                    Preprocessor Defs
#########################################################

if(linux)
    add_definitions(-DPARQUET_STATIC
                    -DARROW_STATIC
                    -DTINS_STATIC
                    -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
else()
    add_compile_definitions(PARQUET_STATIC
                            ARROW_STATIC
                            TINS_STATIC
                            SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
endif()

# Add source files dir.
if(NOT DEFINED CONDA)
  add_subdirectory(cpp)
endif()
