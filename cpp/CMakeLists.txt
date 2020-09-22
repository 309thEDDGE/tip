
# Shared vs. static controlled by BUILD_SHARED_LIBS property.
# Note: lib name intentionally same as the dir name.

# Note: There are sections in many of the library build and 
# link portions below which are commented out. Leave the
# commented sections as reference. They may be useful when
# linking static libraries in Linux. If unnecessary after
# successfully building in Linux, then remote them. 

# Include dirs needed for several libs.
set(ch10_lib_name "ch10")
set(ch10_src_dir ${ch10_lib_name}/${source_dir_name})
set(ch10_inc_dir ${ch10_lib_name}/${include_dir_name})
set(ch10_main_dir ${ch10_lib_name}/${main_dir_name})

#########################################################
#                   Build/link util lib
#########################################################
set(util_lib_name "util")
set(util_src_dir ${util_lib_name}/${source_dir_name}) # util/src
set(util_inc_dir ${util_lib_name}/${include_dir_name})
add_library(${util_lib_name}
            ${util_src_dir}/binbuff.cpp              ${util_inc_dir}/binbuff.h
            ${util_src_dir}/data_organization.cpp    ${util_inc_dir}/data_organization.h
            ${util_src_dir}/file_reader.cpp          ${util_inc_dir}/file_reader.h
            ${util_src_dir}/iterable_tools.cpp       ${util_inc_dir}/iterable_tools.h
            ${util_src_dir}/parse_text.cpp           ${util_inc_dir}/parse_text.h
            ${util_src_dir}/user_input.cpp           ${util_inc_dir}/user_input.h
            ${util_src_dir}/yaml_reader.cpp          ${util_inc_dir}/yaml_reader.h
            ${util_src_dir}/tmats_parser.cpp         ${util_inc_dir}/tmats_parser.h
            ${util_src_dir}/metadata.cpp             ${util_inc_dir}/metadata.h
            ${util_src_dir}/parquet_reader.cpp       ${util_inc_dir}/parquet_reader.h
 )

target_include_directories(${util_lib_name} PUBLIC ${util_inc_dir} ${yaml_inc_dir} ${parquet_inc_dirs})
target_compile_features(${util_lib_name} PUBLIC cxx_std_17)
target_link_libraries(${util_lib_name} PUBLIC ${yaml_lib_name} ${parquet_libs} ${linux_libs})

#########################################################
#                   Build/link tmats lib
#########################################################
set(tmats_lib_name "tmats")
set(tmats_src_dir "${tmats_lib_name}/${source_dir_name}")
set(tmats_inc_dir "${tmats_lib_name}/${include_dir_name}")
add_library(${tmats_lib_name}
            ${tmats_src_dir}/tmats_item.cpp  ${tmats_inc_dir}/tmats_item.h
            ${tmats_src_dir}/tmats.cpp       ${tmats_inc_dir}/tmats.h
 )

#target_include_directories(${tmats_lib_name} PUBLIC ${tmats_inc_dir} ${util_inc_dir} ${yaml_inc_dir})
#target_compile_features(${tmats_lib_name} PUBLIC cxx_std_17)
#target_link_libraries(${tmats_lib_name} PUBLIC ${util_lib_name} ${yaml_lib_name})

target_include_directories(${tmats_lib_name} PUBLIC ${tmats_inc_dir} ${util_inc_dir})
target_compile_features(${tmats_lib_name} PUBLIC cxx_std_17)
target_link_libraries(${tmats_lib_name} PUBLIC ${util_lib_name} ${linux_libs})

#########################################################
#              Build/link parquet_context lib
#########################################################
set(pqctx_lib_name "parquet_context")
message(STATUS "Building lib: " "${pqctx_lib_name}")
set(pqctx_src_dir ${pqctx_lib_name}/${source_dir_name})
set(pqctx_inc_dir ${pqctx_lib_name}/${include_dir_name})
add_library(${pqctx_lib_name}
            ${pqctx_inc_dir}/column_data.h
            ${pqctx_src_dir}/parquet_context.cpp   ${pqctx_inc_dir}/parquet_context.h
    )

target_include_directories(${pqctx_lib_name} PUBLIC ${pqctx_inc_dir} ${parquet_inc_dirs})
target_compile_features(${pqctx_lib_name} PUBLIC cxx_std_17)
target_link_libraries(${pqctx_lib_name} PUBLIC ${parquet_libs} ${linux_libs})

#########################################################
#      Build/link ethernet lib
#########################################################
set(eth_lib_name "")
set(eth_inc_dir "")
if(libirig106 AND ethernet_data)

    # Build ch10ethernet add_library(${1:target_name} ${2:source_file}).
    set(eth_lib_name "ch10ethernet")
    set(eth_path "ethernet")
    set(eth_inc_dir ${eth_path}/${include_dir_name})
    set(eth_src_dir ${eth_path}/${source_dir_name})
    message(STATUS "Building ethernet lib (using libtins): " "${eth_lib_name}")
    add_library(${eth_lib_name} ${eth_src_dir}/network_packet_parser.cpp ${eth_inc_dir}/network_packet_parser.h
                                ${eth_src_dir}/ethernet_data.cpp ${eth_inc_dir}/ethernet_data.h)
    target_include_directories(${eth_lib_name} PUBLIC ${eth_inc_dir}
                               ${all_tins_incs})
    target_link_libraries(${eth_lib_name} PUBLIC ${all_tins_libs})
    target_compile_features(${eth_lib_name} PUBLIC cxx_std_17)
endif()

#########################################################
#            Build/link ch10_parquet lib
#########################################################
set(ch10pq_lib_name "ch10_parquet")
message(STATUS "Building lib: " "${ch10pq_lib_name}")
set(ch10pq_src_dir ${ch10pq_lib_name}/${source_dir_name})
set(ch10pq_inc_dir ${ch10pq_lib_name}/${include_dir_name})

if(ethernet_data)
add_library(${ch10pq_lib_name}
            ${ch10pq_src_dir}/parquet_milstd1553f1.cpp ${ch10pq_inc_dir}/parquet_milstd1553f1.h
            ${ch10pq_src_dir}/parquet_videodataf0.cpp  ${ch10pq_inc_dir}/parquet_videodataf0.h
            ${ch10pq_src_dir}/parquet_ethernetf0.cpp ${ch10pq_inc_dir}/parquet_ethernetf0.h
    )
else()
add_library(${ch10pq_lib_name}
            ${ch10pq_src_dir}/parquet_milstd1553f1.cpp ${ch10pq_inc_dir}/parquet_milstd1553f1.h
            ${ch10pq_src_dir}/parquet_videodataf0.cpp  ${ch10pq_inc_dir}/parquet_videodataf0.h
    )
endif()

#target_include_directories(${ch10pq_lib_name} PUBLIC 
#                           ${ch10pq_inc_dir}
#                           ${parquet_inc_dirs}
#                           ${pqctx_inc_dir} 
#                           ${ch10_inc_dir}) # Ch10 format headers.
#target_compile_features(${ch10pq_lib_name} PUBLIC cxx_std_17)    
#target_link_libraries(${ch10pq_lib_name} PUBLIC ${pqctx_lib_name} ${parquet_libs})

target_include_directories(${ch10pq_lib_name} PUBLIC 
                            ${ch10pq_inc_dir}
                            ${pqctx_inc_dir} 
                            ${eth_inc_dir}
                            ${ch10_inc_dir}) # Ch10 format headers.
target_compile_features(${ch10pq_lib_name} PUBLIC cxx_std_17)    
target_link_libraries(${ch10pq_lib_name} PUBLIC ${pqctx_lib_name} ${eth_lib_name} ${linux_libs})

#########################################################
#                 Build/link metadata lib
#########################################################
set(metadata_lib_name "metadata")
message(STATUS "Building lib: " "${metadata_lib_name}")
set(metadata_src_dir ${metadata_lib_name}/${source_dir_name})
set(metadata_inc_dir ${metadata_lib_name}/${include_dir_name})
add_library(${metadata_lib_name}
            ${metadata_src_dir}/parser_metadata.cpp ${metadata_inc_dir}/parser_metadata.h
    )

#target_include_directories(${metadata_lib_name} PUBLIC 
#                           ${metadata_inc_dir} 
#                           ${parquet_inc_dirs}
#                           ${ch10pq_inc_dir}
#                           ${pqctx_inc_dir}
#                           ${util_inc_dir}
#                           ${yaml_inc_dir}
#                           ${ch10_inc_dir})
#target_compile_features(${metadata_lib_name} PUBLIC cxx_std_17)
#target_link_libraries(${metadata_lib_name} PUBLIC ${util_lib_name} ${ch10pq_lib_name} ${pqctx_lib_name}
#                      ${yaml_lib_name} ${parquet_libs})

target_include_directories(${metadata_lib_name} PUBLIC 
                            ${metadata_inc_dir} 
                            ${ch10pq_inc_dir}
                            ${util_inc_dir}
                            ${ch10_inc_dir})
target_compile_features(${metadata_lib_name} PUBLIC cxx_std_17)
target_link_libraries(${metadata_lib_name} PUBLIC ${util_lib_name} ${ch10pq_lib_name} 
                        ${linux_libs})

#########################################################
#             Build/link translate_1553 lib
#########################################################
set(trans_lib_name "translate_1553")
message(STATUS "Building lib: " "${trans_lib_name}")
set(trans_src_dir ${trans_lib_name}/${source_dir_name})
set(trans_inc_dir ${trans_lib_name}/${include_dir_name})
add_library(${trans_lib_name}
        ${trans_src_dir}/bus_map.cpp                        ${trans_inc_dir}/bus_map.h
        ${trans_src_dir}/icd_data.cpp                       ${trans_inc_dir}/icd_data.h
        ${trans_src_dir}/icd_element.cpp                    ${trans_inc_dir}/icd_element.h
        ${trans_src_dir}/icd_translate.cpp                  ${trans_inc_dir}/icd_translate.h
        ${trans_src_dir}/parquet_translation_manager.cpp    ${trans_inc_dir}/parquet_translation_manager.h
        ${trans_src_dir}/translatable_1553_table.cpp        ${trans_inc_dir}/translatable_1553_table.h
        ${trans_src_dir}/translatable_base.cpp              ${trans_inc_dir}/translatable_base.h
        ${trans_src_dir}/translation_master.cpp             ${trans_inc_dir}/translation_master.h
    )

#target_include_directories(${trans_lib_name} PUBLIC ${trans_inc_dir} ${config_inc_dir} ${tmats_inc_dir} 
#                           ${util_inc_dir} ${ch10pq_inc_dir} ${pqctx_inc_dir} ${metadata_inc_dir})
target_include_directories(${trans_lib_name} PUBLIC ${trans_inc_dir}
                            ${util_inc_dir} ${ch10pq_inc_dir})

target_include_directories(${trans_lib_name} PUBLIC  
                            ${metadata_lib_name})

#target_link_libraries(${trans_lib_name} PUBLIC ${config_lib_name} ${tmats_lib_name}
#                      ${util_lib_name} ${ch10pq_lib_name} ${pqctx_lib_name} ${metadata_lib_name})
target_link_libraries(${trans_lib_name} PUBLIC
                        ${util_lib_name} ${ch10pq_lib_name} ${linux_libs})
target_link_libraries(${trans_lib_name} PUBLIC ${metadata_lib_name})
target_compile_features(${trans_lib_name} PUBLIC cxx_std_17)

#########################################################
#      Build/link Parquet-to-Parquet Comparator Lib
#########################################################
set(comp_lib_name "parquet_compare")
set(comp_path "validation")
set(comp_inc_dir ${comp_path}/${include_dir_name})
set(comp_src_dir ${comp_path}/${source_dir_name})
set(comp_main_dir ${comp_path}/${main_dir_name})
message(STATUS "Building pq-to-pq comparator lib: " "${comp_lib_name}")
add_library(${comp_lib_name} ${comp_src_dir}/comparator.cpp )
target_include_directories(${comp_lib_name} PUBLIC ${comp_inc_dir} ${parquet_inc_dirs} ${util_inc_dir})
target_link_libraries(${comp_lib_name} PUBLIC ${util_lib_name} ${linux_libs})
target_compile_features(${comp_lib_name} PUBLIC cxx_std_17)

#########################################################
#                Build/link ch10parse Lib
#########################################################
set(ch10parse_exe_name "tip_parse")
if(video_data)
    set(ch10parse_exe_name "tip_parse_video")
endif()

set(ch10parse_lib_name "ch10parse_lib")
set(temp_irig_lib_name "")
set(temp_irig_inc_dir "")
set(parse_ethernet_src "")
set(temp_eth_lib_name "")
set(temp_eth_inc_dir "")
set(temp_tins_libs "")
if(libirig106)
    set(temp_irig_lib_name ${irig_lib_name})
    set(temp_irig_inc_dir ${irig_inc_dir})
    if(ethernet_data)
        set(parse_ethernet_src ${ch10_inc_dir}/i106_parse_context.h ${ch10_src_dir}/i106_parse_context.cpp
                         ${ch10_inc_dir}/i106_ch10_ethernetf0.h ${ch10_src_dir}/i106_ch10_ethernetf0.cpp)
        set(temp_eth_lib_name ${eth_lib_name})
        set(temp_eth_inc_dir ${eth_inc_dir})
        set(temp_tins_libs ${all_tins_libs})
    endif()
endif()

add_library(${ch10parse_lib_name} 
    ${ch10_inc_dir}/ch10.h ${ch10_inc_dir}/ch10_milstd1553f1.h ${ch10_inc_dir}/ch10_milstd1553f1stats.h
    ${ch10_inc_dir}/ch10_packet_header.h ${ch10_inc_dir}/ch10_packet_header_format.h
    ${ch10_inc_dir}/ch10_packet_stats.h ${ch10_inc_dir}/ch10_tdf1.h ${ch10_inc_dir}/ch10_videodataf0.h
    ${ch10_inc_dir}/MilStd1553F1Format.h ${ch10_inc_dir}/parse_context.h ${ch10_inc_dir}/parse_manager.h
    ${ch10_inc_dir}/parse_util.h ${ch10_inc_dir}/parse_worker.h
    ${ch10_inc_dir}/TDF1Format.h ${ch10_inc_dir}/VideoDataF0Format.h
    ${ch10_src_dir}/ch10_milstd1553f1.cpp ${ch10_src_dir}/ch10_milstd1553f1stats.cpp
    ${ch10_src_dir}/ch10_packet_header.cpp ${ch10_src_dir}/ch10_tdf1.cpp 
    ${ch10_src_dir}/ch10_videodataf0.cpp ${ch10_src_dir}/parse_manager.cpp ${ch10_src_dir}/parse_util.cpp
    ${ch10_src_dir}/parse_worker.cpp
    ${parse_ethernet_src}
    )
target_include_directories(${ch10parse_lib_name} PUBLIC ${ch10_inc_dir} ${yaml_inc_dir} ${util_inc_dir}
                            ${pqctx_inc_dir} ${ch10pq_inc_dir} ${parquet_inc_dirs} 
                            ${metadata_inc_dir} ${tmats_inc_dir}
                            ${temp_irig_inc_dir} ${temp_eth_inc_dir}
                            )
target_compile_features(${ch10parse_lib_name} PUBLIC cxx_std_17)

target_link_libraries(${ch10parse_lib_name} PUBLIC
                        #${util_lib_name}
                        ${tmats_lib_name}
                        #${pqctx_lib_name}
                        ${ch10pq_lib_name}
                        ${metadata_lib_name}
                        #${parquet_libs}
                        ${linux_libs}
                        ${temp_irig_lib_name}
                        ${temp_tins_libs}
                        ${temp_eth_lib_name}
    )

#########################################################
#                   Install Libs
#########################################################
install(TARGETS ${util_lib_name} 
                ${tmats_lib_name}
                ${pqctx_lib_name} 
                ${ch10pq_lib_name} 
                ${trans_lib_name}
                ${metadata_lib_name} 
                ${comp_lib_name}
                ${eth_lib_name}
                ${ch10parse_lib_name}
        DESTINATION ${out_lib_dir}
)


#########################################################
#                Build/link ch10parse exe
#########################################################
set(ch10parse_exe_name "tip_parse")
if(video_data)
    set(ch10parse_exe_name "tip_parse_video")
endif()

message(STATUS "Building exe: " "${ch10parse_exe_name}")
add_executable(${ch10parse_exe_name} ${ch10_main_dir}/ch10parse.cpp
    )
target_include_directories(${ch10parse_exe_name} PUBLIC ${ch10_inc_dir} ${yaml_inc_dir} ${util_inc_dir}
                            ${pqctx_inc_dir} ${ch10pq_inc_dir} ${parquet_inc_dirs} 
                            ${metadata_inc_dir} ${tmats_inc_dir}
                            ${temp_irig_inc_dir} ${temp_eth_inc_dir}
                            )
target_compile_features(${ch10parse_exe_name} PUBLIC cxx_std_17)

target_link_libraries(${ch10parse_exe_name} ${ch10parse_lib_name})

#########################################################
#               Build/link translator Exe
#########################################################

set(trans_exe_name "tip_translate")
message(STATUS "Building exe: " "${trans_exe_name}")
set(trans_main_dir ${trans_lib_name}/${main_dir_name})
add_executable(${trans_exe_name} ${trans_main_dir}/translate_1553_from_parquet.cpp)
target_include_directories(${trans_exe_name} PUBLIC 
                        ${trans_inc_dir} 
                        ${ch10pq_inc_dir} 
                        ${config_inc_dir}
                        ${metadata_inc_dir})
target_link_libraries(${trans_exe_name} ${trans_lib_name} 
                    ${ch10pq_lib_name} 
                    ${config_lib_name}
                    ${metadata_lib_name}
        ${linux_libs})

#########################################################
#                 Build/link tests Exe
#########################################################
if(build_tests)
    set(test_exe_name "tests")
    message(STATUS "Building lib: " "${test_exe_name}")
    set(test_src_dir ${test_exe_name}/${source_dir_name})
    set(test_main_dir ${test_exe_name}/${main_dir_name})
    add_executable(${test_exe_name}
                   ${test_main_dir}/test.cpp
                   ${test_src_dir}/bus_map_u.cpp ${test_src_dir}/file_reader_u.cpp 
                   ${test_src_dir}/icd_data_u.cpp ${test_src_dir}/icd_translate_u.cpp
                   ${test_src_dir}/iterable_tools_u.cpp ${test_src_dir}/parse_text_u.cpp
                   ${test_src_dir}/parser_config_params_u.cpp ${test_src_dir}/translation_config_params_u.cpp
                   ${test_src_dir}/user_input_u.cpp ${test_src_dir}/yaml_reader_u.cpp
                   ${test_src_dir}/binbuff_u.cpp ${test_src_dir}/parse_context_u.cpp
                   ${test_src_dir}/parquet_arrow_validator_u.cpp ${test_src_dir}/parquet_context_u.cpp
                   ${test_src_dir}/tmats_parser_u.cpp ${test_src_dir}/metadata_u.cpp 
                   ${test_src_dir}/parse_manager_u.cpp ${test_src_dir}/parquet_reader_u.cpp)
    target_include_directories(${test_exe_name} PUBLIC ${gsuite_inc_dirs} ${util_inc_dir} #${yaml_inc_dir} 
                               ${parquet_inc_dirs} ${trans_inc_dir} ${comp_inc_dir} ${pqctx_inc_dir} 
                               ${tmats_inc_dir} ${temp_irig_inc_dir} ${ch10_inc_dir} )
    target_compile_features(${test_exe_name} PUBLIC cxx_std_17)
    target_link_libraries(${test_exe_name}
                          ${gsuite_libs}
                          ${util_lib_name}
                          ${trans_lib_name}
                          ${pqctx_lib_name}
                          ${comp_lib_name}
                          ${ch10parse_lib_name}
                          ${parquet_libs}
              ${linux_libs})
endif()

#########################################################
#                 Build/link video extractor Exe
#########################################################

if(video_data)
    set(video_exe_name "parquet_video_extractor")
    set(video_path "video/parquet")
    set(video_inc_dir ${video_path}/${include_dir_name})
    set(video_src_dir ${video_path}/${source_dir_name})
    set(video_main_dir ${video_path}/${main_dir_name})
    message(STATUS "Building video extraction exe: " "${video_exe_name}")
    add_executable(${video_exe_name} ${video_main_dir}/parquet_video_extractor.cpp
                   ${video_inc_dir}/parquet_video_extraction.h ${video_src_dir}/parquet_video_extraction.cpp)
    target_include_directories(${video_exe_name} PUBLIC ${video_inc_dir}
                               ${parquet_inc_dirs})
    target_compile_features(${video_exe_name} PUBLIC cxx_std_17)
    target_link_libraries(${video_exe_name} ${parquet_libs} ${linux_libs})
endif()

#########################################################
#      Build/link Parquet-to-Parquet Comparator Exe
#########################################################
set(comp_exe_name "pqcompare")
message(STATUS "Building pq-to-pq comparator exe: " "${comp_exe_name}")
add_executable(${comp_exe_name} ${comp_main_dir}/parquet_arrow_validator_main.cpp)
target_include_directories(${comp_exe_name} PUBLIC ${comp_inc_dir} ${parquet_inc_dirs})
target_compile_features(${comp_exe_name} PUBLIC cxx_std_17)
target_link_libraries(${comp_exe_name} ${comp_lib_name} ${parquet_libs})

#########################################################
#      Build/link Binary Comparator Exe
#########################################################
set(bincomp_exe_name "bincompare")
message(STATUS "Building binary comparator exe: " "${bincomp_exe_name}")
add_executable(${bincomp_exe_name} ${comp_main_dir}/binary_comparison.cpp)

#########################################################
#                   Install Exes
#########################################################
install(TARGETS ${ch10parse_exe_name}
                ${trans_exe_name}
                ${comp_exe_name}
                ${bincomp_exe_name}
        DESTINATION ${out_bin_dir}
        )

if(build_tests)
    install(TARGETS ${test_exe_name}
            DESTINATION ${out_bin_dir})
endif()

if(video_data)
    install(TARGETS ${video_exe_name}
            DESTINATION ${out_bin_dir})
endif()
