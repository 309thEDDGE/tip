set(headers
    include/md_category.h
    include/md_category_scalar.h
    include/md_category_sequence.h
    include/md_category_map.h
    include/md_document.h
    include/yaml_md_document.h
    include/tip_md_document.h
)

add_library(metadata
            src/md_category.cpp
            src/md_category_scalar.cpp
            src/md_category_sequence.cpp
            src/md_category_map.cpp
            src/md_document.cpp
            src/yaml_md_document.cpp
            src/tip_md_document.cpp
)

target_compile_features(metadata PUBLIC cxx_std_17)

target_include_directories(metadata PUBLIC include)

set_target_properties(metadata PROPERTIES PUBLIC_HEADER "${headers}")

target_link_libraries(metadata
    PUBLIC yaml-cpp spdlog::spdlog
)

install(
    TARGETS metadata 
)
