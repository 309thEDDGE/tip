set(headers
    include/yamlsv_log_item.h
    include/yamlsv_schema.h
    include/yaml_schema_validation.h
)

add_library(yamlsv
            src/yaml_schema_validation.cpp
)

target_compile_features(yamlsv PUBLIC cxx_std_17)

target_include_directories(yamlsv PUBLIC include)

set_target_properties(yamlsv PROPERTIES PUBLIC_HEADER "${headers}")

target_link_libraries(yamlsv
    PUBLIC tiputil yaml-cpp
)

add_executable(validate_yaml main/validate_yaml.cpp)

target_compile_features(validate_yaml PUBLIC cxx_std_17)

target_link_libraries(validate_yaml yamlsv)

install(
    TARGETS yamlsv validate_yaml
)
