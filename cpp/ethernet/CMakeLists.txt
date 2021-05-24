set(headers
    include/network_packet_parser.h
    include/ethernet_data.h
)

add_library(lib_ethernet
    STATIC
    src/network_packet_parser.cpp
    src/ethernet_data.cpp
)

target_compile_features(lib_ethernet PUBLIC cxx_std_17)

target_include_directories(lib_ethernet
    PUBLIC include
)

set_target_properties(lib_ethernet
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(lib_ethernet
    PUBLIC tins
)

install(
    TARGETS lib_ethernet
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ethernet
)
