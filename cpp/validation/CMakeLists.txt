set(headers
    include/comparator.h
)

add_library(parquet_compare
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
    PRIVATE spdlog::spdlog yaml-cpp tiputil ch10_parquet
)

add_executable(pqcompare main/parquet_arrow_validator_main.cpp)

target_link_libraries(pqcompare parquet_compare
    spdlog::spdlog yaml-cpp tiputil cli ch10_parquet
)
target_compile_features(pqcompare PUBLIC cxx_std_17)

add_executable(bincompare main/binary_comparison.cpp)

target_link_libraries(bincompare spdlog::spdlog yaml-cpp tiputil cli)

target_compile_features(bincompare PUBLIC cxx_std_17)

install(
    TARGETS pqcompare bincompare
    DESTINATION ${out_bin_dir}
)

if(BUILD_SHARED_LIBS)
    install(TARGETS parquet_compare)
endif()
