set(headers 
    include/translatable_column_base.h
    include/translatable_column_template.h
    include/translatable_table_base.h
    include/translate_status.h
    include/translate_tabular_context_base.h
    include/translate_tabular.h
    include/translation_manager.h
)

add_library(translate_tabular
            src/translatable_column_base.cpp
            src/translatable_table_base.cpp
            src/translate_tabular_context_base.cpp
            src/translate_tabular.cpp
            src/translation_manager.cpp
)

target_compile_features(translate_tabular PUBLIC cxx_std_17)

target_include_directories(translate_tabular PUBLIC include)

set_target_properties(translate_tabular PROPERTIES PUBLIC_HEADER "${headers}")

target_link_libraries(translate_tabular
    PUBLIC tiputil ch10_parquet dts_1553 spdlog::spdlog
)

install(
    TARGETS translate_tabular 
)