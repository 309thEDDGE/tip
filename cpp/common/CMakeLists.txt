set(headers
    include/ch10_1553f1_msg_hdr_format.h
    include/ch10_ethernetf0_msg_hdr_format.h
    include/ch10_header_format.h
    include/ch10_tdpf1_hdr_format.h
    include/ch10_videof0_header_format.h
    include/ch10_arinc429f0_msg_hdr_format.h
    include/ch10_packet_type.h
    include/sysexits.h
)

add_library(common INTERFACE)

target_compile_features(common INTERFACE cxx_std_17)

target_include_directories(common
    INTERFACE include
)

set_target_properties(common
    PROPERTIES PUBLIC_HEADER "${headers}"
)

if(BUILD_SHARED_LIBS)
    install(TARGETS common)
endif()
