
set(BUILD_SHARED_LIBS OFF)

#########################################################
#                      Dependencies 
#########################################################

set(yaml-cpp_include_paths ${USE_DEPS_DIR}/yaml-cpp/include/yaml-cpp ${USE_DEPS_DIR}/yaml-cpp/include)
set(yaml-cpp_lib_paths "${USE_DEPS_DIR}/yaml-cpp/build/Release")
set(yaml-cpp_lib_names libyaml-cppmd.lib)

set(spdlog_include_paths ${USE_DEPS_DIR}/spdlog-1.8.2/include/spdlog ${USE_DEPS_DIR}/spdlog-1.8.2/include)
set(spdlog_lib_paths "${USE_DEPS_DIR}/spdlog-1.8.2/lib")
set(spdlog_lib_names spdlog.lib)

set(Lz4_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Lz4_lib_names liblz4_static)

set(Zstd_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Zstd_lib_names zstd_static)

set(Zlib_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Zlib_lib_names zlibstatic)

set(Snappy_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Snappy_lib_names snappy)

set(Thrift_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Thrift_lib_names thriftmd)

set(Brotlicommon_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Brotlicommon_lib_names brotlicommon-static)

set(Brotlidec_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Brotlidec_lib_names brotlidec-static)

set(Brotlienc_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Brotlienc_lib_names brotlienc-static)

set(Doubleconversion_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Doubleconversion_lib_names double-conversion)

set(Flatbuffers_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Flatbuffers_lib_names flatbuffers)

set(Boostfilesystem_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Boostfilesystem_lib_names libboost_filesystem-vc140-mt-x64-1_67)

set(Boostregex_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Boostregex_lib_names libboost_regex-vc140-mt-x64-1_67)

set(Boostsystem_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Boostsystem_lib_names libboost_system-vc140-mt-x64-1_67)

set(Arrow_include_paths ${USE_DEPS_DIR}/arrow/include/arrow)
set(Arrow_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Arrow_lib_names arrow_static.lib)

set(Parquet_include_paths ${USE_DEPS_DIR}/arrow/include/parquet/api)
set(Parquet_lib_paths ${Arrow_lib_paths})
set(Parquet_lib_names parquet_static.lib)

set(libpcap_include_paths ${USE_DEPS_DIR}/npcap/include)
set(libpcap_lib_paths ${USE_DEPS_DIR}/npcap/lib/x64)
set(libpcap_lib_names wpcap.lib)
set(libpcapdll_lib_names wpcap.dll)

set(packet_lib_names Packet.lib)
set(packetdll_lib_names Packet.dll)

# Pcap dlls that must be installed next to tip_parse.exe.
set(pcap_dll1 ${libpcap_lib_paths}/${libpcapdll_lib_names})
set(pcap_dll2 ${libpcap_lib_paths}/${packetdll_lib_names})

# Copy pcap dlls to bin dir.
configure_file(${pcap_dll1} ${out_bin_dir} COPYONLY)
configure_file(${pcap_dll2} ${out_bin_dir} COPYONLY)

# Copy dlls to build/cpp dir.
if(NOT EXISTS ${tests_build_dir})
    file(MAKE_DIRECTORY ${tests_build_dir})
endif()
configure_file(${pcap_dll1} ${tests_build_dir} COPYONLY)
configure_file(${pcap_dll2} ${tests_build_dir} COPYONLY)

set(libtins_include_paths ${USE_DEPS_DIR}/tins/include/tins)
set(libtins_lib_paths ${USE_DEPS_DIR}/tins/lib)
set(libtins_lib_names tins.lib)

set(GTest_include_paths ${USE_DEPS_DIR}/googletest/googletest/include/gtest)
set(GTest_lib_paths ${USE_DEPS_DIR}/googletest/googletest/lib)
set(GTest_lib_names gtest)
set(GTest_main_lib_names gtest_main)

set(GMock_include_paths ${USE_DEPS_DIR}/googletest/googlemock/include/gmock)
set(GMock_lib_paths ${USE_DEPS_DIR}/googletest/googlemock/lib)
set(GMock_lib_names gmock)

find_package(yaml-cpp REQUIRED)
find_package(spdlog REQUIRED)
find_package(Parquet REQUIRED)
find_package(libtins REQUIRED)
find_package(GTest REQUIRED)

set(GTEST_LIBRARIES GTest::gtest GTest::gmock GTest::gtest_main)