set(headers
    include/argument_validation.h
    include/binbuff.h
    include/data_organization.h
    include/file_reader.h
    include/iterable_tools.h
    include/managed_path.h
    include/metadata.h
    include/parse_text.h
    include/tmats_parser.h
    include/user_input.h
    include/yaml_reader.h
)

add_library(lib_util STATIC
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
)

target_compile_features(lib_util PUBLIC cxx_std_17)

target_include_directories(lib_util PUBLIC include)

set_target_properties(lib_util PROPERTIES PUBLIC_HEADER "${headers}")

target_link_libraries(lib_util
    PUBLIC spdlog::spdlog yaml-cpp
)

install(
    TARGETS lib_util
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/utils
)
