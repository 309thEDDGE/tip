set(headers
    include/ch10_packet_type.h
)

add_library(common INTERFACE)

target_compile_features(common INTERFACE cxx_std_17)

target_include_directories(common
    INTERFACE include
)

set_target_properties(common
    PROPERTIES PUBLIC_HEADER "${headers}"
)

install(
    TARGETS common
)
