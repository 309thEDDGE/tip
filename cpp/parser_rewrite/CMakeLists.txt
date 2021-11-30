set(headers
    include/ch10_1553f1_component.h
    include/ch10_1553f1_msg_hdr_format.h
    include/ch10_context.h
    include/ch10_ethernetf0_component.h
    include/ch10_ethernetf0_msg_hdr_format.h
    include/ch10_header_format.h
    include/ch10_packet.h
    include/ch10_packet_component.h
    include/ch10_packet_element.h
    include/ch10_packet_element_base.h
    include/ch10_packet_header_component.h
    include/ch10_packet_type.h
    include/ch10_status.h
    include/ch10_tdp_component.h
    include/ch10_time.h
    include/ch10_tmats_component.h
    include/ch10_videof0_component.h
    include/ch10_videof0_header_format.h
    include/ch10_arinc429f0_component.h
    include/ch10_arinc429f0_msg_hdr_format.h
)

add_library(parserw
    STATIC
    src/ch10_1553f1_component.cpp
    src/ch10_context.cpp
    src/ch10_ethernetf0_component.cpp
    src/ch10_packet_component.cpp
    src/ch10_packet.cpp
    src/ch10_packet_header_component.cpp
    src/ch10_status.cpp
    src/ch10_tdp_component.cpp
    src/ch10_time.cpp
    src/ch10_tmats_component.cpp
    src/ch10_videof0_component.cpp
    src/ch10_arinc429f0_component.cpp
)

target_compile_features(parserw PUBLIC cxx_std_17)

target_include_directories(parserw
    PUBLIC include
)

set_target_properties(parserw
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(parserw
    PUBLIC tiputil ch10_parquet spdlog::spdlog
)

install(
    TARGETS parserw
)
