add_library(lib_parquet_context
    STATIC src/parquet_context.cpp
)

target_compile_features(lib_parquet_context PUBLIC cxx_std_17)

target_include_directories(lib_parquet_context
    PUBLIC include
)

set_target_properties(lib_parquet_context
    PROPERTIES PUBLIC_HEADER "include/parquet_context.h"
)

target_link_libraries(lib_parquet_context
    PUBLIC arrow_shared parquet spdlog::spdlog
)

install(
    TARGETS lib_parquet_context
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/parquet_context
)
