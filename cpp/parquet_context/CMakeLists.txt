set(headers
    include/parquet_context.h
    include/column_data.h
)

add_library(parquet_context 
    src/parquet_context.cpp
)

target_compile_features(parquet_context PUBLIC cxx_std_17)

target_include_directories(parquet_context
    PUBLIC include
)

set_target_properties(parquet_context
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(parquet_context
    PUBLIC arrow_shared parquet 
)

target_link_libraries(parquet_context
    PRIVATE spdlog::spdlog
)

if(BUILD_SHARED_LIBS)
    install(TARGETS parquet_context)
endif()
