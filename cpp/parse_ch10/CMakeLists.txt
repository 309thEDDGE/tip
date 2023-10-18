set(headers
    include/parse_manager.h
    include/ch10_parse_main.h
    include/parse_worker.h
    include/worker_config.h
    include/tmats_parser.h
    include/tmats_data.h
    include/parser_cli_help_strings.h
    include/parser_cli.h
    include/parser_paths.h
    include/parser_metadata.h
    include/ch10_packet_type_specific_metadata.h
)

add_library(parsech10_lib 
    src/parse_manager.cpp
    src/ch10_parse_main.cpp
    src/parse_worker.cpp
    src/worker_config.cpp
    src/tmats_parser.cpp 
    src/tmats_data.cpp
    src/parser_paths.cpp
    src/parser_metadata.cpp
    src/ch10_packet_type_specific_metadata.cpp
)

target_compile_features(parsech10_lib PUBLIC cxx_std_17)

target_include_directories(parsech10_lib
    PUBLIC include
)

set_target_properties(parsech10_lib
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(parsech10_lib
    PRIVATE common spdlog::spdlog yaml-cpp tiputil ch10comp yamlsv 
    spdlog::spdlog metadata cli parquet_context ch10_parquet ch10ethernet
)

if(BUILD_SHARED_LIBS)
    install(TARGETS parsech10_lib)
endif()
