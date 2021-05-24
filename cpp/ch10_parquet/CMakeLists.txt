set(headers
    include/parquet_ethernetf0.h
    include/parquet_milstd1553f1.h
    include/parquet_reader.h
    include/parquet_videodataf0.h
)

add_library(lib_ch10_parquet
    STATIC
    src/parquet_ethernetf0.cpp
    src/parquet_milstd1553f1.cpp
    src/parquet_reader.cpp
    src/parquet_videodataf0.cpp
)

target_compile_features(lib_ch10_parquet PUBLIC cxx_std_17)

target_include_directories(lib_ch10_parquet
    PUBLIC include ../parser_rewrite/include
)

set_target_properties(lib_ch10_parquet
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(lib_ch10_parquet
    PUBLIC lib_parquet_context lib_ethernet lib_util lib_parser_rewrite
    arrow_shared parquet spdlog::spdlog
)

install(
    TARGETS lib_ch10_parquet
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ch10_parquet
)
