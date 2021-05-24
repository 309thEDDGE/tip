set(headers
    include/comparator.h
)

add_library(lib_validation
    STATIC
    src/comparator.cpp
)

target_compile_features(lib_validation PUBLIC cxx_std_17)

target_include_directories(lib_validation
    PUBLIC include
)

set_target_properties(lib_validation
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(lib_validation
    PUBLIC lib_util lib_ch10_parquet
    arrow_shared parquet
)

add_executable(pqcompare main/parquet_arrow_validator_main.cpp)

target_link_libraries(pqcompare lib_validation)

target_compile_features(pqcompare PUBLIC cxx_std_17)

add_executable(bincompare main/binary_comparison.cpp)

target_link_libraries(bincompare lib_validation)

install(
    TARGETS lib_validation pqcompare bincompare
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/validation
)
