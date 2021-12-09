set(headers
    include/logger_setup.h
)

add_executable(tests
    main/test.cpp
    src/argument_validation_u.cpp
    src/binbuff_u.cpp
    src/bus_map_u.cpp
    src/ch10_1553f1_component_u.cpp
    src/ch10_context_u.cpp
    src/ch10_ethernetf0_component_u.cpp
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
    src/icd_element_u.cpp
    src/icd_data_u.cpp
    src/icd_translate_u.cpp
    src/iterable_tools_u.cpp
    src/logger_setup.cpp
    src/managed_path_u.cpp
    src/network_packet_parser_u.cpp
    src/parquet_arrow_validator_u.cpp
    src/parquet_context_u.cpp
    src/parquet_reader_u.cpp
    src/parse_manager_u.cpp
    src/parser_config_params_u.cpp
    src/parse_text_u.cpp
    src/parse_worker_u.cpp
    src/tmats_parser_u.cpp
    src/translation_config_params_u.cpp
    src/user_input_u.cpp
    src/yaml_reader_u.cpp
    src/yaml_schema_validation_u.cpp
    src/yamlsv_log_item_u.cpp
    src/resource_limits_u.cpp
    src/version_info_u.cpp
    src/uri_percent_encoding_u.cpp
    src/translate_tabular_context_base_u.cpp
    src/translate_tabular_u.cpp
    src/translate_tabular_context_1553_u.cpp
    src/translatable_table_base_u.cpp
    src/translatable_column_base_u.cpp
    src/translatable_column_template_u.cpp
    src/stream_buffering_u.cpp
    src/sha256_tools_u.cpp
    src/ch10_arinc429f0_component_u.cpp
    src/md_category_u.cpp
    src/md_category_scalar_u.cpp
    src/md_category_sequence_u.cpp
    src/md_category_map_u.cpp
    src/md_document_u.cpp
    src/yaml_md_document_u.cpp
    src/tip_md_document_u.cpp
    src/provenance_data_u.cpp
    src/dts429_u.cpp
)

target_compile_features(tests PUBLIC cxx_std_17)

target_include_directories(tests
    PUBLIC include
)

target_link_libraries(tests
    PUBLIC common parsech10_lib ch10_parquet ch10ethernet parquet_context ch10comp
    dts_1553 dts_429 tiputil parquet_compare pq_vid_extract yamlsv
    arrow_shared parquet spdlog::spdlog translate_tabular translate_tabular_1553
    metadata
    PRIVATE ${GTEST_LIBRARIES}
)

install(
    TARGETS tests
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(tests)
