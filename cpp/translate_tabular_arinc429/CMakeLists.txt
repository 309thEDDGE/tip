set(headers
    include/translate_tabular_arinc429_main.h
    include/translate_tabular_context_arinc429.h
    include/translatable_explicit_sign_column_template.h
    include/translatable_arinc429_table.h
    include/translator_cli_help_strings_429.h 
    include/translator_cli_429.h 
    include/dts_arinc429_schema.h 
)

add_library(translate_tabular_arinc429 
            src/translate_tabular_arinc429_main.cpp
            src/translate_tabular_context_arinc429.cpp
            src/translatable_arinc429_table.cpp
)

target_compile_features(translate_tabular_arinc429 PUBLIC cxx_std_17)

target_include_directories(translate_tabular_arinc429
    PUBLIC include
)

set_target_properties(translate_tabular_arinc429
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(translate_tabular_arinc429
  PRIVATE common yaml-cpp spdlog::spdlog tiputil yamlsv sha256 cli dts_429 
  translate_tabular metadata ch10_parquet
)

add_executable("${TRANSLATE_429_EXE_NAME}" main/translate_tabular_arinc429.cpp)

target_link_libraries("${TRANSLATE_429_EXE_NAME}" translate_tabular_arinc429
    common yaml-cpp spdlog::spdlog tiputil yamlsv cli dts_429 
    translate_tabular metadata ch10_parquet
)

target_compile_features("${TRANSLATE_429_EXE_NAME}" PUBLIC cxx_std_17)

install(
    TARGETS "${TRANSLATE_429_EXE_NAME}"
    DESTINATION ${out_bin_dir}
)