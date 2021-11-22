set(headers
    include/parse_manager.h
    include/parser_helper_funcs.h
    include/parse_worker.h
    include/worker_config.h
)

add_library(parsech10_lib
    src/parse_manager.cpp
    src/parser_helper_funcs.cpp
    src/parse_worker.cpp
)

target_compile_features(parsech10_lib PUBLIC cxx_std_17)

target_include_directories(parsech10_lib
    PUBLIC include
)

set_target_properties(parsech10_lib
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(parsech10_lib
    PUBLIC tiputil parserw yamlsv spdlog::spdlog
)

add_executable("${CH10_PARSE_EXE_NAME}" main/ch10parse.cpp)

target_compile_features("${CH10_PARSE_EXE_NAME}" PUBLIC cxx_std_17)

target_link_libraries("${CH10_PARSE_EXE_NAME}" PUBLIC parsech10_lib)

install(
    TARGETS parsech10_lib "${CH10_PARSE_EXE_NAME}"
)