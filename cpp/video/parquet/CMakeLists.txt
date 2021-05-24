set(headers
    include/parquet_video_extraction.h
)

add_library(lib_video_parquet
    STATIC
    src/parquet_video_extraction.cpp
)

target_compile_features(lib_video_parquet PUBLIC cxx_std_17)

target_include_directories(lib_video_parquet
    PUBLIC include
)

set_target_properties(lib_video_parquet
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(lib_video_parquet
    PUBLIC lib_util
    arrow_shared parquet
)

add_executable(parquet_video_extractor main/parquet_video_extractor.cpp)

target_compile_features(parquet_video_extractor PUBLIC cxx_std_17)

target_link_libraries(parquet_video_extractor lib_video_parquet)

install(
    TARGETS lib_video_parquet
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/video/parquet
)
