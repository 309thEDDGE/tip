set(headers
    include/parse_manager.h
    include/parser_helper_funcs.h
    include/parse_worker.h
    include/worker_config.h
)

add_library(lib_ch10
    STATIC
    src/parse_manager.cpp
    src/parser_helper_funcs.cpp
    src/parse_worker.cpp
)

target_compile_features(lib_ch10 PUBLIC cxx_std_17)

target_include_directories(lib_ch10
    PUBLIC include
)

set_target_properties(lib_ch10
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(lib_ch10
    PUBLIC lib_util lib_parser_rewrite lib_yamlsv spdlog::spdlog
)

add_executable(tip_parse main/ch10parse.cpp)

target_compile_features(tip_parse PUBLIC cxx_std_17)

target_link_libraries(tip_parse PUBLIC lib_ch10)

install(
    TARGETS lib_ch10 tip_parse
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ch10
)
