set(headers
    include/argument_validation.h
    include/binbuff.h
    include/data_organization.h
    include/file_reader.h
    include/iterable_tools.h
    include/managed_path.h
    include/metadata.h
    include/parser_config_params.h
    include/parse_text.h
    include/tmats_parser.h
    include/translation_config_params.h
    include/user_input.h
    include/yaml_reader.h
    include/resource_limits.h
    include/version_info.h
)

add_library(tiputil STATIC
            src/argument_validation.cpp
            src/binbuff.cpp
            src/data_organization.cpp
            src/file_reader.cpp
            src/iterable_tools.cpp
            src/managed_path.cpp
            src/metadata.cpp
            src/parse_text.cpp
            src/tmats_parser.cpp
            src/user_input.cpp
            src/yaml_reader.cpp
            src/resource_limits.cpp
            src/version_info.cpp
)

target_compile_features(tiputil PUBLIC cxx_std_17)

target_include_directories(tiputil PUBLIC include)

set_target_properties(tiputil PROPERTIES PUBLIC_HEADER "${headers}")

target_link_libraries(tiputil
    PUBLIC spdlog::spdlog yaml-cpp
)

install(
    TARGETS tiputil
)
