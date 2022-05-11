set(headers
    include/translate_tabular_1553_main.h
    include/translate_tabular_context_1553.h
    include/dts_1553_schema.h
    include/translator_cli_help_strings_1553.h 
    include/translator_cli_1553.h
)

add_library(translate_tabular_1553
            src/translate_tabular_1553_main.cpp
            src/translate_tabular_context_1553.cpp
)

target_compile_features(translate_tabular_1553 PUBLIC cxx_std_17)

target_include_directories(translate_tabular_1553
    PUBLIC include
)

set_target_properties(translate_tabular_1553
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(translate_tabular_1553
  PUBLIC translate_tabular tiputil ch10_parquet dts_1553 yamlsv metadata 
  spdlog::spdlog cli
)

add_executable("${TRANSLATE_1553_EXE_NAME}" main/translate_tabular_1553.cpp)

target_link_libraries("${TRANSLATE_1553_EXE_NAME}" translate_tabular_1553)

target_compile_features("${TRANSLATE_1553_EXE_NAME}" PUBLIC cxx_std_17)

install(
    TARGETS translate_tabular_1553 "${TRANSLATE_1553_EXE_NAME}"
)