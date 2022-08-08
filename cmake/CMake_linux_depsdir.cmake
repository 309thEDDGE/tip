set(BUILD_SHARED_LIBS OFF)

# NEWARROW is used if the new (~0.17+) version of 
# Arrow is used. 
if(${USE_NEWARROW})
	add_definitions(-DNEWARROW)
endif()

#########################################################
#                     Linux-specific
#########################################################

add_compile_options(-m64 -pthread)

#########################################################
#                      Dependencies 
#########################################################

set(yaml-cpp_include_paths ${USE_DEPS_DIR}/yaml-cpp/include/yaml-cpp ${USE_DEPS_DIR}/yaml-cpp/include)
set(yaml-cpp_lib_paths "${USE_DEPS_DIR}/yaml-cpp/lib")
set(yaml-cpp_lib_names yaml-cpp)

set(spdlog_include_paths ${USE_DEPS_DIR}/spdlog-1.8.2/include/spdlog ${USE_DEPS_DIR}/spdlog-1.8.2/include)
set(spdlog_lib_paths "${USE_DEPS_DIR}/spdlog-1.8.2/lib")
set(spdlog_lib_names spdlog)

set(Lz4_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Lz4_lib_names lz4)

set(Zstd_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Zstd_lib_names zstd)

set(Zlib_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Zlib_lib_names z)

set(Snappy_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Snappy_lib_names snappy)

set(Thrift_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Thrift_lib_names thrift)

set(Brotlicommon_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Brotlicommon_lib_names brotlicommon-static)

set(Brotlidec_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Brotlidec_lib_names brotlidec-static)

set(Brotlienc_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Brotlienc_lib_names brotlienc-static)

set(Jemalloc_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Jemalloc_lib_names jemalloc)

set(Glog_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Glog_lib_names glog)

set(Doubleconversion_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Doubleconversion_lib_names double-conversion)

set(Flatbuffers_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Flatbuffers_lib_names flatbuffers)

set(Boostfilesystem_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Boostfilesystem_lib_names boost_filesystem)

set(Boostregex_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Boostregex_lib_names boost_regex)

set(Boostsystem_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Boostsystem_lib_names boost_system)

set(Arrow_include_paths ${USE_DEPS_DIR}/arrow/include/arrow)
set(Arrow_lib_paths ${USE_DEPS_DIR}/arrow/lib)
set(Arrow_lib_names arrow)

set(Parquet_include_paths ${USE_DEPS_DIR}/arrow/include/parquet/api)
set(Parquet_lib_paths ${Arrow_lib_paths})
set(Parquet_lib_names parquet)

set(libpcap_include_paths ${USE_DEPS_DIR}/pcap/include)
set(libpcap_lib_paths ${USE_DEPS_DIR}/pcap/lib)
set(libpcap_lib_names pcap)
set(libpcapdll_lib_names wpcap.dll)

set(packet_lib_names Packet)
set(packetdll_lib_names Packet.dll)

set(libtins_include_paths ${USE_DEPS_DIR}/tins/include/tins)
set(libtins_lib_paths ${USE_DEPS_DIR}/tins/lib)
set(libtins_lib_names tins)

set(GTest_include_paths ${USE_DEPS_DIR}/googletest/googletest/include/gtest)
set(GTest_lib_paths ${USE_DEPS_DIR}/googletest/googletest/lib)
set(GTest_lib_names gtest)
set(GTest_main_lib_names gtest_main)

set(GMock_include_paths ${USE_DEPS_DIR}/googletest/googlemock/include/gmock)
set(GMock_lib_paths ${USE_DEPS_DIR}/googletest/googlemock/lib)
set(GMock_lib_names gmock)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(Parquet REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(spdlog REQUIRED)
find_package(libtins REQUIRED)
find_package(GTest REQUIRED)

set(GTEST_LIBRARIES GTest::gtest GTest::gmock GTest::gtest_main)

