find_package(Packet)

find_path(libpcap_INCLUDE_DIR 
    NAMES pcap.h 
    PATHS ${libpcap_include_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("libpcap_INCLUDE_DIR: ${libpcap_INCLUDE_DIR}")

find_library(libpcap_LIBRARY
    NAMES ${libpcap_lib_names}
    PATHS ${libpcap_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("libpcap_LIBRARY: ${libpcap_LIBRARY}")

if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    find_file(libpcapdll_LIBRARY
        NAMES ${libpcapdll_lib_names}
        PATHS ${libpcap_lib_paths}
        NO_DEFAULT_PATH
    )
endif()

set(libpcap_LIBRARIES ${libpcap_LIBRARY})
set(libpcap_INCLUDE_DIRS ${libpcap_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libpcap DEFAULT_MSG 
    libpcap_LIBRARY libpcap_INCLUDE_DIR 
)

if(libpcap_FOUND)
    set(libpcap_FOUND TRUE)
    add_library(pcap UNKNOWN IMPORTED)
    set_target_properties(pcap
        PROPERTIES IMPORTED_LOCATION "${libpcap_LIBRARY}"         
        INTERFACE_INCLUDE_DIRECTORIES "${libpcap_INCLUDE_DIR}"
    )
    if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
        target_link_libraries(pcap INTERFACE packet)
    endif()
endif()
