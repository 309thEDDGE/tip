set(headers
    include/spdlog_setup_helper_funcs.h
)

add_executable(tests
    STATIC
    main/test.cpp
    src/argument_validation_u.cpp
    src/binbuff_u.cpp
    src/bus_map_u.cpp
    src/ch10_1553f1_component_u.cpp
    src/ch10_context_u.cpp
    src/ch10_packet_component_u.cpp
    src/ch10_packet_element_u.cpp
    src/ch10_packet_header_component_u.cpp
    src/ch10_packet_u.cpp
    src/ch10_tdp_component_u.cpp
    src/ch10_time_u.cpp
    src/ch10_tmats_component_u.cpp
    src/ch10_videof0_component_u.cpp
    src/dts1553_u.cpp
    src/file_reader_u.cpp
    src/icd_data_u.cpp
    src/icd_translate_u.cpp
    src/iterable_tools_u.cpp
    src/managed_path_u.cpp
    src/metadata_u.cpp
    src/parquet_arrow_validator_u.cpp
    src/parquet_context_u.cpp
    src/parquet_reader_u.cpp
    src/parse_manager_u.cpp
    src/parser_config_params_u.cpp
    src/parse_text_u.cpp
    src/parse_worker_u.cpp
    src/spdlog_setup_helper_funcs.h
    src/tmats_parser_u.cpp
    src/translation_config_params_u.cpp
    src/user_input_u.cpp
    src/yaml_reader_u.cpp
    src/yaml_schema_validation_u.cpp
)

target_compile_features(tests PUBLIC cxx_std_17)

target_include_directories(tests
    PUBLIC include
)

target_link_libraries(tests
    PUBLIC lib_ch10 lib_ch10_parquet lib_ethernet lib_parquet_context lib_parser_rewrite
    spdlog::spdlog
)

install(
    TARGETS tests
)

# #########################################################
# #                 Build/link tests Exe
# #########################################################

# set(test_exe_name "tests")
# message(STATUS "Building lib: " "${test_exe_name}")
# set(test_inc_dir ${test_exe_name}/${include_dir_name})
# set(test_src_dir ${test_exe_name}/${source_dir_name})
# set(test_main_dir ${test_exe_name}/${main_dir_name})
# add_executable(${test_exe_name}
#                 ${test_main_dir}/test.cpp
#                 ${test_src_dir}/bus_map_u.cpp ${test_src_dir}/file_reader_u.cpp
#                 ${test_src_dir}/icd_data_u.cpp ${test_src_dir}/icd_translate_u.cpp
#                 ${test_src_dir}/iterable_tools_u.cpp ${test_src_dir}/parse_text_u.cpp
#                 ${test_src_dir}/parser_config_params_u.cpp ${test_src_dir}/translation_config_params_u.cpp
#                 ${test_src_dir}/user_input_u.cpp ${test_src_dir}/yaml_reader_u.cpp
#                 ${test_src_dir}/binbuff_u.cpp
#                 ${test_src_dir}/parquet_arrow_validator_u.cpp ${test_src_dir}/parquet_context_u.cpp
#                 ${test_src_dir}/tmats_parser_u.cpp ${test_src_dir}/metadata_u.cpp
#                 ${test_src_dir}/parse_manager_u.cpp ${test_src_dir}/parquet_reader_u.cpp
#                 ${test_src_dir}/dts1553_u.cpp
#                 ${test_src_dir}/managed_path_u.cpp
#                 ${test_inc_dir}/spdlog_setup_helper_funcs.h
#                 ${test_src_dir}/ch10_packet_element_u.cpp
#                 ${test_src_dir}/ch10_packet_component_u.cpp
#                 ${test_src_dir}/ch10_context_u.cpp
#                 ${test_src_dir}/ch10_packet_header_component_u.cpp
#                 ${test_src_dir}/ch10_packet_u.cpp
#                 ${test_src_dir}/ch10_tmats_component_u.cpp
#                 ${test_src_dir}/ch10_tdp_component_u.cpp
#                 ${test_src_dir}/ch10_1553f1_component_u.cpp
#                 ${test_src_dir}/ch10_videof0_component_u.cpp
#                 ${test_src_dir}/ch10_time_u.cpp
#                 ${test_src_dir}/yaml_schema_validation_u.cpp
#                 ${test_src_dir}/argument_validation_u.cpp
#                 ${test_src_dir}/parse_worker_u.cpp)

# # set_property(TARGET ${test_exe_name} PROPERTY POSITION_INDEPENDENT_CODE ON)
# target_include_directories(${test_exe_name} PUBLIC ${test_inc_dir} ${gsuite_inc_dirs} ${util_inc_dir}
#                             ${parquet_inc_dirs} ${trans_inc_dir} ${comp_inc_dir} ${pqctx_inc_dir}
#                             ${tmats_inc_dir} ${temp_irig_inc_dir} ${ch10_inc_dir} ${parserw_inc_dir}
#                             ${spdlog_inc_dir} ${yamlsv_inc_dir})

# target_compile_features(${test_exe_name} PUBLIC cxx_std_17)
# target_link_libraries(${test_exe_name}
#                         ${gsuite_libs}
#                         ${util_lib_name}
#                         ${trans_lib_name}
#                         ${pqctx_lib_name}
#                         ${comp_lib_name}
#                         ${ch10parse_lib_name}
#                         ${parquet_libs}
#                         ${parserw_lib_name}
#                         ${spdlog_lib_name}
#                         ${yamlsv_lib_name}
#                         ${linux_libs})

# # For Ctest
# #include(CTest)
# enable_testing()

# # For gtest_discover_tests
# include(GoogleTest)

# # Create ctest executable
# gtest_discover_tests(${test_exe_name})
