set(headers
    include/yamlsv_log_item.h
    include/yamlsv_schema.h
    include/yaml_schema_validation.h
    include/validate_yaml_main.h
)

add_library(yamlsv
    src/yaml_schema_validation.cpp
    src/validate_yaml_main.cpp
)

target_compile_features(yamlsv PUBLIC cxx_std_17)

target_include_directories(yamlsv PUBLIC include)

set_target_properties(yamlsv PROPERTIES PUBLIC_HEADER "${headers}")

target_link_libraries(yamlsv
    PRIVATE common tiputil yaml-cpp spdlog::spdlog cli
)

if(BUILD_SHARED_LIBS)
    install(TARGETS yamlsv)
endif()

