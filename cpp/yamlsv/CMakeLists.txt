set(headers
    include/yamlsv_log_item.h
    include/yaml_schema_validation.h
)

add_library(lib_yamlsv STATIC
            src/yaml_schema_validation.cpp
)

target_compile_features(lib_yamlsv PUBLIC cxx_std_17)

target_include_directories(lib_yamlsv PUBLIC include)

set_target_properties(lib_yamlsv PROPERTIES PUBLIC_HEADER "${headers}")

target_link_libraries(lib_yamlsv
    PUBLIC lib_util yaml-cpp
)

add_executable(validate_yaml main/validate_yaml.cpp)

target_compile_features(validate_yaml PUBLIC cxx_std_17)

target_link_libraries(validate_yaml lib_yamlsv)

install(
    TARGETS lib_yamlsv validate_yaml
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/yamlsv
)
