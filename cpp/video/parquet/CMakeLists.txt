set(headers
    include/parquet_video_extraction.h
    include/parquet_video_extraction_main.h
)

add_library(pq_vid_extract 
    src/parquet_video_extraction.cpp
    src/parquet_video_extraction_main.cpp
)

target_compile_features(pq_vid_extract PUBLIC cxx_std_17)

target_include_directories(pq_vid_extract
    PUBLIC include
)

set_target_properties(pq_vid_extract
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(pq_vid_extract
    PRIVATE common spdlog::spdlog yaml-cpp tiputil cli arrow_shared parquet
)

if(BUILD_SHARED_LIBS)
    install(TARGETS pq_vid_extract)
endif()
