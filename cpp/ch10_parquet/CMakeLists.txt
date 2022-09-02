set(headers
    include/parquet_ethernetf0.h
    include/parquet_milstd1553f1.h
    include/parquet_reader.h
    include/parquet_videodataf0.h
    include/parquet_arinc429f0.h
    include/parquet_tdpf1.h
)

add_library(ch10_parquet 
    src/parquet_ethernetf0.cpp
    src/parquet_milstd1553f1.cpp
    src/parquet_reader.cpp
    src/parquet_videodataf0.cpp
    src/parquet_arinc429f0.cpp
    src/parquet_tdpf1.cpp
)

target_compile_features(ch10_parquet PUBLIC cxx_std_17)

target_include_directories(ch10_parquet
    PUBLIC include
)

set_target_properties(ch10_parquet
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(ch10_parquet
    PRIVATE common yaml-cpp ch10ethernet tiputil 
    spdlog::spdlog
)

target_link_libraries(ch10_parquet
    PUBLIC parquet_context 
)
