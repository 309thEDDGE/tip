set(headers
    include/network_packet_parser.h
    include/ethernet_data.h
)

add_library(ch10ethernet
    src/network_packet_parser.cpp
    src/ethernet_data.cpp
)

target_compile_features(ch10ethernet PUBLIC cxx_std_17)

target_include_directories(ch10ethernet
    PUBLIC include
)

set_target_properties(ch10ethernet
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(ch10ethernet
    PUBLIC tins tiputil pcap
)

install(
    TARGETS ch10ethernet
)
