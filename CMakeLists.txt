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

#########################################################
#                      Build Options
#########################################################

set(linux false)
set(container false)
set(debug 1)
set(libirig106 true)
set(video_data false)
set(ethernet_data true)

if (DEFINED DEBUG)
    message(STATUS "DEBUG defined")
    #set(ignore ${DEBUG})
    set(debug ${DEBUG})
endif()

if(DEFINED CONTAINER)
    message(STATUS "CONTAINER defined")
    set(ignore ${CONTAINER})
    set(container true)
endif()

# Default def video_data = false. Set the VIDEO_DATA preproc 
# def if true. User can set -DVIDEO=1 to set this property.
if(DEFINED VIDEO)
    message(STATUS "VIDEO defined")
    set(ignore ${VIDEO})
    set(video_data true)
endif()

if(DEFINED ETHERNET)
    if("${ETHERNET}" MATCHES "OFF")
        message(STATUS "ETHERNET set to OFF")
        set(ethernet_data false)
    endif()
    set(ignore ${ETHERNET})
endif()
    
if(ethernet_data)
     message(STATUS "ETHERNET set to ON")
endif()

# Default def build_tests = true. If TESTS is defined by the
# user via -DNOTESTS=1 then tests will not be built.
set(build_tests true)
if(DEFINED NOTESTS)
    message(STATUS "Not building tests")
    set(ignore ${NOTESTS})
    set(build_tests false)
endif()

if(DEFINED LIBIRIG106)
    if("${LIBIRIG106}" MATCHES "OFF")
        message(STATUS "LIBIRIG106 set to OFF")
        set(libirig106 false)
    endif()
    set(ignore LIBIRIG106)
endif()

if(libirig106)
    message(STATUS "Using libirig106, defining LIBIRIG106")
endif()

if(ethernet_data AND NOT libirig106)
    message(FATAL_ERROR "LIBIRIG106 (default ON) must be ON if ETHERNET (default ON) is ON")
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
if("${CMAKE_SYSTEM_NAME}" MATCHES "CYGWIN")
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
    add_definitions(-DLOCALDB
                    -DDEBUG=${debug}
                #-D_CRT_SECURE_NO_WARNINGS   
)
else()
    add_compile_definitions(LOCALDB
                        DEBUG=${debug}
                #_CRT_SECURE_NO_WARNINGS   
)
endif()

if(linux)
    add_definitions(-DPARQUET_STATIC
                    -DPARQUET
                    -DARROW_STATIC)
else()
    add_compile_definitions(PARQUET_STATIC
                        PARQUET
                        ARROW_STATIC)
endif()

if(video_data)
    if(linux)
        add_definitions(-DVIDEO_DATA)
    else()
	    add_compile_definitions(VIDEO_DATA)
    endif()
endif()

if(libirig106)
    if(linux)
        add_definitions(-DLIBIRIG106)
    else()
        add_compile_definitions(LIBIRIG106)
    endif()
endif()

if(ethernet_data)
    if(linux)
        add_definitions(-DETHERNET_DATA)
        add_definitions(-DTINS_STATIC)
    else()
        add_compile_definitions(ETHERNET_DATA)
        add_compile_definitions(TINS_STATIC)
    endif()
endif()

# Add source files dir.
add_subdirectory(cpp) 
