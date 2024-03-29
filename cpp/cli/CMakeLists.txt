set(headers
    include/cli_conf.h
    include/cli_arg.h
    include/cli_positional_arg.h
    include/cli_optional_arg.h
    include/cli_flag.h 
    include/cli.h
    include/cli_group.h
    include/arg_special_config.h
    include/default_special_config.h 
    include/validate_special_config.h 
    include/container_arg.h 
)
add_library(cli 
    src/cli_conf.cpp
    src/cli_arg.cpp
    src/cli_flag.cpp
    src/cli.cpp
    src/cli_group.cpp
    src/container_arg.cpp
)

target_compile_features(cli PUBLIC cxx_std_17)

target_include_directories(cli
    PUBLIC include
)

set_target_properties(cli
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(cli 
    PRIVATE common tiputil yaml-cpp spdlog::spdlog)

if(BUILD_SHARED_LIBS)
    install(TARGETS cli)
endif()
