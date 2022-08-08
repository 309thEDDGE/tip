find_package(libpcap)

find_path(libtins_INCLUDE_DIR 
    NAMES tins.h 
    PATHS ${libtins_include_paths}
    REQUIRED
    NO_DEFAULT_PATH
)

# Includes for tins are written as [#include "tins/tins.h"]
get_filename_component(libtins_include_dirname ${libtins_INCLUDE_DIR} NAME)
if(${libtins_include_dirname} STREQUAL tins)
    get_filename_component(libtins_INCLUDE_DIR ${libtins_INCLUDE_DIR} DIRECTORY)
endif()
#message("libtins_INCLUDE_DIR: ${libtins_INCLUDE_DIR}")

find_library(libtins_LIBRARY
    NAMES ${libtins_lib_names}
    PATHS ${libtins_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
#message("libtins_LIBRARY: ${libtins_LIBRARY}")

set(libtins_LIBRARIES ${libtins_LIBRARY})
set(libtins_INCLUDE_DIRS ${libtins_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libtins DEFAULT_MSG 
    libtins_LIBRARY libtins_INCLUDE_DIR 
)

if(libtins_FOUND)
    set(libtins_FOUND TRUE)
    add_library(tins UNKNOWN IMPORTED)
    set_target_properties(tins
        PROPERTIES IMPORTED_LOCATION "${libtins_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${libtins_INCLUDE_DIR}")
    target_link_libraries(tins INTERFACE pcap)    
    if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
        target_link_libraries(tins INTERFACE Ws2_32.lib Iphlpapi.lib)
    endif()
endif()
