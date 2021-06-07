set(headers
    include/bus_map.h
    include/dts1553.h
    include/icd_data.h
    include/icd_element.h
    include/icd_translate.h
    include/parquet_translation_manager.h
    include/translatable_1553_table.h
    include/translatable_base.h
    include/translatable_column.h
    include/translation_master.h
    include/translator_helper_funcs.h
)

add_library(translate_1553
    STATIC
    src/bus_map.cpp
    src/dts1553.cpp
    src/icd_data.cpp
    src/icd_element.cpp
    src/icd_translate.cpp
    src/parquet_translation_manager.cpp
    src/translatable_1553_table.cpp
    src/translatable_base.cpp
    src/translation_master.cpp
    src/translator_helper_funcs.cpp
)

target_compile_features(translate_1553 PUBLIC cxx_std_17)

target_include_directories(translate_1553
    PUBLIC include
)

set_target_properties(translate_1553
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(translate_1553
  PUBLIC tiputil parquet_context yamlsv ch10_parquet
  yaml-cpp arrow_shared parquet tins
)

add_executable(tip_translate main/translate_1553_from_parquet.cpp)

target_link_libraries(tip_translate translate_1553)

target_compile_features(tip_translate PUBLIC cxx_std_17)

install(
    TARGETS translate_1553 tip_translate
)
