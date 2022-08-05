set(headers
    include/sha256.h
)

add_library(sha256 
    src/sha256.cpp
)

target_compile_features(sha256 PUBLIC cxx_std_17)

target_include_directories(sha256
    PUBLIC include
)

set_target_properties(sha256
    PROPERTIES PUBLIC_HEADER "${headers}"
)
