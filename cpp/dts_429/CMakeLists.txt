set(headers
    include/dts429.h
    include/organize_429_icd.h
)

add_library(dts_429
    src/dts429.cpp
    src/organize_429_icd.cpp
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
  yaml-cpp arrow_shared parquet tins dts_1553
)
