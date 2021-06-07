set(headers
    include/parquet_ethernetf0.h
    include/parquet_milstd1553f1.h
    include/parquet_reader.h
    include/parquet_videodataf0.h
)

add_library(ch10_parquet
    STATIC
    src/parquet_ethernetf0.cpp
    src/parquet_milstd1553f1.cpp
    src/parquet_reader.cpp
    src/parquet_videodataf0.cpp
)

target_compile_features(ch10_parquet PUBLIC cxx_std_17)

target_include_directories(ch10_parquet
    PUBLIC include
)

set_target_properties(ch10_parquet
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(ch10_parquet
    PUBLIC parquet_context ch10ethernet tiputil parserw
    arrow_shared parquet spdlog::spdlog
)

install(
    TARGETS ch10_parquet
)
