find_package(Lz4 REQUIRED)
find_package(Zstd REQUIRED)
find_package(Zlib REQUIRED)
find_package(Snappy REQUIRED)
find_package(Thrift REQUIRED)

find_package(Brotlicommon REQUIRED)
find_package(Brotlidec REQUIRED)
find_package(Brotlienc REQUIRED)

find_package(Doubleconversion REQUIRED)
find_package(Flatbuffers REQUIRED)

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    find_package(Glog QUIET)
    find_package(Jemalloc QUIET)
endif()

find_package(Boostfilesystem REQUIRED)
find_package(Boostregex REQUIRED)
find_package(Boostsystem REQUIRED)

find_path(Arrow_INCLUDE_DIR 
    NAMES api.h 
    PATHS ${Arrow_include_paths}
    REQUIRED
    NO_DEFAULT_PATH
)

# Includes for Arrow are written as [#include "arrow/api.h"]
get_filename_component(Arrow_include_dirname ${Arrow_INCLUDE_DIR} NAME)
if(${Arrow_include_dirname} STREQUAL arrow)
    get_filename_component(Arrow_INCLUDE_DIR ${Arrow_INCLUDE_DIR} DIRECTORY)
endif()
# message("Arrow_INCLUDE_DIR: ${Arrow_INCLUDE_DIR}")

find_library(Arrow_LIBRARY
    NAMES ${Arrow_lib_names}
    PATHS ${Arrow_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Arrow_LIBRARY: ${Arrow_LIBRARY}")

set(Arrow_LIBRARIES ${Arrow_LIBRARY})
set(Arrow_INCLUDE_DIRS ${Arrow_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Arrow DEFAULT_MSG 
    Arrow_LIBRARY Arrow_INCLUDE_DIR 
)

if(Arrow_FOUND)
    set(Arrow_FOUND TRUE)
    add_library(arrow_shared UNKNOWN IMPORTED)
    set_target_properties(arrow_shared
        PROPERTIES IMPORTED_LOCATION "${Arrow_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Arrow_INCLUDE_DIR}")
    if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
        target_link_libraries(arrow_shared INTERFACE "${CMAKE_THREAD_LIBS_INIT}")
    endif()    

    target_link_libraries(arrow_shared INTERFACE zlib 
        boost_filesystem boost_system boost_regex lz4 zstd 
        snappy thrift brotlienc brotlidec brotlicommon doubleconversion 
        )

    if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
        target_link_libraries(arrow_shared INTERFACE flatbuffers)
    elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
        target_link_libraries(arrow_shared INTERFACE jemalloc glog)
    endif()    
endif()
