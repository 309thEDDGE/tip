set(headers
    include/parse_manager.h
    include/ch10_parse_main.h
    include/parse_worker.h
    include/worker_config.h
    include/tmats_parser.h
    include/tmats_data.h
    include/parser_cli_help_strings.h
    include/parser_cli.h
)

add_library(parsech10_lib
    src/parse_manager.cpp
    src/ch10_parse_main.cpp
    src/parse_worker.cpp
    src/tmats_parser.cpp 
    src/tmats_data.cpp
)

target_compile_features(parsech10_lib PUBLIC cxx_std_17)

target_include_directories(parsech10_lib
    PUBLIC include
)

set_target_properties(parsech10_lib
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(parsech10_lib
    PUBLIC tiputil ch10comp yamlsv spdlog::spdlog metadata cli
)

add_executable("${CH10_PARSE_EXE_NAME}" main/ch10parse.cpp)

target_compile_features("${CH10_PARSE_EXE_NAME}" PUBLIC cxx_std_17)

target_link_libraries("${CH10_PARSE_EXE_NAME}" PUBLIC common parsech10_lib)

install(
    TARGETS parsech10_lib "${CH10_PARSE_EXE_NAME}"
)
