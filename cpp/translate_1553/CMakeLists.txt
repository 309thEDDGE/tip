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

add_library(lib_translate_1553
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

target_compile_features(lib_translate_1553 PUBLIC cxx_std_17)

target_include_directories(lib_translate_1553
    PUBLIC include
)

set_target_properties(lib_translate_1553
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(lib_translate_1553
  PUBLIC lib_util lib_parquet_context lib_yamlsv lib_ch10_parquet
  yaml-cpp arrow_shared parquet
)


add_executable(tip_translate main/translate_1553_from_parquet.cpp)

target_link_libraries(tip_translate lib_translate_1553)

target_compile_features(tip_translate PUBLIC cxx_std_17)

install(
    TARGETS lib_translate_1553 tip_translate
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/translate_1553
)
