set(headers
    include/parse_manager.h
    include/parser_helper_funcs.h
    include/parse_worker.h
    include/worker_config.h
)

add_library(ch10parse_lib
    STATIC
    src/parse_manager.cpp
    src/parser_helper_funcs.cpp
    src/parse_worker.cpp
)

target_compile_features(ch10parse_lib PUBLIC cxx_std_17)

target_include_directories(ch10parse_lib
    PUBLIC include
)

set_target_properties(ch10parse_lib
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(ch10parse_lib
    PUBLIC tiputil parserw yamlsv spdlog::spdlog
)

add_executable(tip_parse main/ch10parse.cpp)

target_compile_features(tip_parse PUBLIC cxx_std_17)

target_link_libraries(tip_parse PUBLIC ch10parse_lib)

install(
    TARGETS ch10parse_lib tip_parse
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ch10
)
