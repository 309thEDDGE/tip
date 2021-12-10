set(dts_1553_inc_dir ../../dts_1553/include)

set(headers
    include/${dts_1553_inc_dir}/bus_map.h
    include/${dts_1553_inc_dir}/icd_data.h
    include/${dts_1553_inc_dir}/icd_element.h
    include/${dts_1553_inc_dir}/icd_translate.h
    include/dts429.h
)

add_library(dts_429
    ../dts_1553/src/bus_map.cpp
    ../dts_1553/src/icd_data.cpp
    ../dts_1553/src/icd_element.cpp
    ../dts_1553/src/icd_translate.cpp
    src/dts429.cpp
)

target_compile_features(dts_429 PUBLIC cxx_std_17)

target_include_directories(dts_429
    PUBLIC include
)

set_target_properties(dts_429
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(dts_429
  PUBLIC tiputil parquet_context yamlsv ch10_parquet
  yaml-cpp arrow_shared parquet tins
)
