set(headers
    include/comparator.h
)

add_library(parquet_compare
    STATIC
    src/comparator.cpp
)

target_compile_features(parquet_compare PUBLIC cxx_std_17)

target_include_directories(parquet_compare
    PUBLIC include
)

set_target_properties(parquet_compare
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(parquet_compare
    PUBLIC tiputil ch10_parquet
    arrow_shared parquet
)

add_executable(pqcompare main/parquet_arrow_validator_main.cpp)

target_link_libraries(pqcompare parquet_compare)

target_compile_features(pqcompare PUBLIC cxx_std_17)

add_executable(bincompare main/binary_comparison.cpp)

target_link_libraries(bincompare parquet_compare)

target_compile_features(bincompare PUBLIC cxx_std_17)

install(
    TARGETS parquet_compare pqcompare bincompare
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/validation
)
