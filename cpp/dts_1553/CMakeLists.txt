set(headers
    include/bus_map.h
    include/dts1553.h
    include/icd_data.h
    include/icd_element.h
    include/icd_translate.h
    include/dts429.h
)

add_library(dts_1553
    src/bus_map.cpp
    src/dts1553.cpp
    src/icd_data.cpp
    src/icd_element.cpp
    src/icd_translate.cpp
)

target_compile_features(dts_1553 PUBLIC cxx_std_17)

target_include_directories(dts_1553
    PUBLIC include
)

set_target_properties(dts_1553
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(dts_1553
  PUBLIC tiputil parquet_context yamlsv ch10_parquet
  yaml-cpp arrow_shared parquet tins
)
