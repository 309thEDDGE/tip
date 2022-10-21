#########################################################
#                       CMake Setup
#########################################################
cmake_minimum_required(VERSION 3.14..3.4)

if(${CMAKE_VERSION} VERSION_LESS 3.17)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

#Hack for Ninja generator on windows.  This has to come before project()
#see https://gitlab.kitware.com/cmake/cmake/-/issues/19401
get_property(multiConfig GLOBAL PROPERTY GENERATOR_IS_MULTICONFIG)
if (NOT multiConfig AND NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")
elseif(NOT DEFINED CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING)
    message(STATUS "Not defined: Defining cmake_build_type: " ${CMAKE_BUILD_TYPE})
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "")
    set(CMAKE_BUILD_TYPE Release CACHE STRING)
    message(STATUS "Empty string: Defining cmake_build_type: " ${CMAKE_BUILD_TYPE})
endif()

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

if(NOT 
    (("${CMAKE_BUILD_TYPE}" STREQUAL "Release") OR 
    ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug") OR 
    ("${CMAKE_BUILD_TYPE}" STREQUAL "Profile")))
    message(FATAL_ERROR "CMAKE_BUILD_TYPE must be one of [Release, Debug, Profile] (Default Release)")
endif()

project(TIP
        VERSION 0.2
        DESCRIPTION "Translate Ingest Parse (TIP) for Ch10 files"
        LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_STANDARD_REQUIRED true)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_PLATFORM_INDEPENDENT_CODE ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
#     message(STATUS "Setting -s flag to strip symbols")
#     set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
# endif()

#########################################################
#                      Build Options
#########################################################

option(USE_DEPS_DIR "Specify dependencies directory for offline build" OFF)
option(USER_INSTALL_DIR "Binary installation directory" OFF)
option(CI_COMMIT_TAG "Manually specify git tag as version (if can't be obtained from git automatically)" OFF)
option(CI_COMMIT_SHORT_SHA "Manually specify git short hash as version (if can't be obtained from git automatically)" OFF)
option(USER_VERSION "Specify version string if commit hash or tag can't be obtained from git" OFF)
option(USE_NEWARROW "Define NEWARROW macro to use modern arrow-cpp API. Not defined automatically when -DUSE_DEPS_DIR is configured" OFF)

if(NOT "${USE_DEPS_DIR}" STREQUAL OFF)
	if(EXISTS "${USE_DEPS_DIR}")
		message(STATUS "Dependencies dir:  ${USE_DEPS_DIR}")
	else()
		message(FATAL_ERROR "Dependencies dir does not exist: ${USE_DEPS_DIR}. Use absolute path.")
	endif()
endif()

#########################################################
#                      Version 
#########################################################

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_version.cmake" include_version_file)
include("${include_version_file}")
message(STATUS "Git version string: " "${GIT_VERSION_STRING}")

#########################################################
#                      Build vars
#########################################################

set(CH10_PARSE_EXE_NAME "tip_parse")
set(TRANSLATE_1553_EXE_NAME "tip_translate_1553")
set(TRANSLATE_429_EXE_NAME "tip_translate_arinc429")

set(include_dir_name "include")
set(source_dir_name "src")
set(main_dir_name "main")
set(bin_dir_name "bin")
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/cpp" build_dir)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/cpp/tests" tests_build_dir)
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${bin_dir_name}" out_bin_dir)
if(NOT "${USER_INSTALL_DIR}" STREQUAL OFF)
    set(out_bin_dir "${USER_INSTALL_DIR}")
endif()
if((NOT "${USER_INSTALL_DIR}" STREQUAL OFF) OR (NOT "${USE_DEPS_DIR}" STREQUAL OFF))
    message(STATUS "Install dir:  ${out_bin_dir}")
endif()

# CMake module dir
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules" CUSTOM_MODULE_PATH)
if(NOT "${USE_DEPS_DIR}" STREQUAL OFF)
    if(NOT DEFINED CMAKE_MODULE_PATH)
        set(CMAKE_MODULE_PATH ${CUSTOM_MODULE_PATH})
    else()
        string(APPEND CMAKE_MODULE_PATH " ${CUSTOM_MODULE_PATH}")
    endif()
    message(STATUS "Module paths: ${CMAKE_MODULE_PATH}")
else()
	set(USE_NEWARROW TRUE)
    set(out_bin_dir "")
endif()

# Common includes all of the tip-relevant directories. Do not include() until
# after system specific config and dependencies are configured.
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_common.cmake" common_include_file)
if(NOT EXISTS "${common_include_file}")
    message(FATAL_ERROR "CMake_common.cmake not found in cmake/")
endif()

# System config includes macros, preprocessor directives, build flags
file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_windows_config.cmake" win_config_include_file)
if(NOT EXISTS "${win_config_include_file}")
    message(FATAL_ERROR "CMake_windows_config.cmake not found in cmake/")
endif()

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_linux_config.cmake" linux_config_include_file)
if(NOT EXISTS "${linux_config_include_file}")
    message(FATAL_ERROR "CMake_linux_config.cmake not found in cmake/")
endif()

file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_darwin_config.cmake" darwin_config_include_file)
if(NOT EXISTS "${darwin_config_include_file}")
    message(FATAL_ERROR "CMake_darwin_config.cmake not found in cmake/")
endif()


#########################################################
#                      System Specific
#########################################################

message(STATUS "System name: " ${CMAKE_SYSTEM_NAME})

if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    include("${win_config_include_file}")    
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    include("${linux_config_include_file}")    
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
    include("${darwin_config_include_file}")    
endif()

if(DEFINED CONDA_PREFIX)

    message(STATUS "Setting conda build options")

    file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_conda.cmake" conda_include_file) 
    if(NOT EXISTS "${conda_include_file}")
        message(FATAL_ERROR "CMake_conda.cmake not found in cmake/")
    endif()
    include("${conda_include_file}") 

elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")

    message(STATUS "Setting system-specific options: " ${CMAKE_SYSTEM_NAME})

	if(NOT "${USE_DEPS_DIR}" STREQUAL OFF)
		file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_windows_depsdir.cmake" deps_include_file)
        if(NOT EXISTS "${deps_include_file}")
            message(FATAL_ERROR "CMake_windows_depsdir.cmake not found in cmake/")
        endif()
	else()
		message(FATAL_ERROR "Windows build for -DUSE_DEPS_DIR=OFF not configured")
	endif()
    include("${deps_include_file}")

elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")

    message(STATUS "Setting system-specific options: " ${CMAKE_SYSTEM_NAME})

	if(NOT "${USE_DEPS_DIR}" STREQUAL OFF)
		file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_linux_depsdir.cmake" include_file)
        if(NOT EXISTS "${include_file}")
            message(FATAL_ERROR "CMake_linux_depsdir.cmake not found in cmake/")
        endif()
	else()
		file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CMake_linux.cmake" include_file)
        if(NOT EXISTS "${include_file}")
            message(FATAL_ERROR "CMake_linux.cmake not found in cmake/")
        endif()
	endif()
    include("${include_file}")

else()
    message(FATAL_ERROR "No system-specific options: " ${CMAKE_SYSTEM_NAME})
endif()

include("${common_include_file}")
