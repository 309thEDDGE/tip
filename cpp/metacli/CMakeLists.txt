# set(headers
#     include/meta_cli_config_params.h
#     include/meta_cli_help_strings.h
#     include/meta_cli.h
#     include/meta_main.h
# )


add_executable(tip main/main.cpp)

target_include_directories(tip PRIVATE include)

target_compile_features(tip PRIVATE cxx_std_17)
target_sources(tip PRIVATE src/meta_main.cpp)

target_link_libraries(tip PUBLIC parsech10_lib
    common spdlog::spdlog yaml-cpp tiputil ch10comp yamlsv 
    spdlog::spdlog metadata cli parquet_context ch10_parquet ch10ethernet
)

install(
    TARGETS tip
    DESTINATION ${out_bin_dir}
)
