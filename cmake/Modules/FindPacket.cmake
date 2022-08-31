find_library(Packet_LIBRARY
    NAMES ${packet_lib_names}
    PATHS ${libpcap_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Packet_LIBRARY: ${Packet_LIBRARY}")

set(Packet_LIBRARIES ${Packet_LIBRARY})

if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
    find_file(Packetdll_LIBRARY
        NAMES ${packetdll_lib_names}
        PATHS ${libpcap_lib_paths}
        NO_DEFAULT_PATH
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Packet DEFAULT_MSG 
    Packet_LIBRARY 
)

if(Packet_FOUND)
    set(Packet_FOUND TRUE)
    add_library(packet UNKNOWN IMPORTED)
    set_target_properties(packet
        PROPERTIES IMPORTED_LOCATION "${Packet_LIBRARY}"
    )
endif()