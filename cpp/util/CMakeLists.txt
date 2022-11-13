set(headers
    include/argument_validation.h
    include/binbuff.h
    include/data_organization.h
    include/file_reader.h
    include/iterable_tools.h
    include/managed_path.h
    include/parser_config_params.h
    include/parse_text.h
    include/translation_config_params.h
    include/user_input.h
    include/yaml_reader.h
    include/resource_limits.h
    include/version_info.h
    include/uri_percent_encoding.h
    include/stream_buffering.h
    include/sha256_tools.h
    include/provenance_data.h
    include/terminal.h
)

add_library(tiputil 
            src/argument_validation.cpp
            src/binbuff.cpp
            src/data_organization.cpp
            src/file_reader.cpp
            src/iterable_tools.cpp
            src/managed_path.cpp
            src/parse_text.cpp
            src/user_input.cpp
            src/yaml_reader.cpp
            src/resource_limits.cpp
            src/version_info.cpp
            src/uri_percent_encoding.cpp
            src/stream_buffering.cpp
            src/sha256_tools.cpp
            src/provenance_data.cpp
            src/terminal.cpp
)

target_compile_features(tiputil PUBLIC cxx_std_17)

target_include_directories(tiputil PUBLIC include)

set_target_properties(tiputil PROPERTIES PUBLIC_HEADER "${headers}")

target_link_libraries(tiputil
    PRIVATE common spdlog::spdlog yaml-cpp 
)

target_link_libraries(tiputil
    PUBLIC sha256
)

if(BUILD_SHARED_LIBS)
    install(TARGETS tiputil)
endif()
