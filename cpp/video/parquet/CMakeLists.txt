set(headers
    include/parquet_video_extraction.h
)

add_library(pq_vid_extract 
    src/parquet_video_extraction.cpp
)

target_compile_features(pq_vid_extract PUBLIC cxx_std_17)

target_include_directories(pq_vid_extract
    PUBLIC include
)

set_target_properties(pq_vid_extract
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(pq_vid_extract
    PRIVATE common spdlog::spdlog yaml-cpp tiputil arrow_shared parquet
)

add_executable(parquet_video_extractor main/parquet_video_extractor.cpp)

target_compile_features(parquet_video_extractor PUBLIC cxx_std_17)

target_link_libraries(parquet_video_extractor pq_vid_extract
    common spdlog::spdlog yaml-cpp tiputil cli arrow_shared parquet
)

install(
    TARGETS parquet_video_extractor
    DESTINATION ${out_bin_dir}
)

if(BUILD_SHARED_LIBS)
    install(TARGETS pq_vid_extract)
endif()
