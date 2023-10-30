add_executable(tip main/main.cpp)

target_include_directories(tip PRIVATE include)

target_compile_features(tip PRIVATE cxx_std_17)
target_sources(tip PRIVATE src/meta_main.cpp)

target_link_libraries(tip PUBLIC 
    common
    spdlog::spdlog
    yaml-cpp
    tiputil
    ch10comp 
    yamlsv 
    metadata 
    cli 
    parquet_context 
    ch10_parquet 
    ch10ethernet
    parsech10_lib
    bin_compare
    parquet_compare
    pq_vid_extract
    dts_1553 
    dts_429 
    translate_tabular 
    translate_tabular_1553
    translate_tabular_arinc429 
)

install(
    TARGETS tip
    DESTINATION ${out_bin_dir}
)
