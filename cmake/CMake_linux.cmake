
set(BUILD_SHARED_LIBS OFF)

#########################################################
#                      Env 
#########################################################

add_definitions(
    -DNEWARROW
)

#########################################################
#                      Dependencies 
#########################################################

find_package(yaml-cpp REQUIRED)
find_package(spdlog REQUIRED)
find_package(libtins CONFIG REQUIRED)

# Use as GTest::gtest, GTest::gmock
find_package(GTest REQUIRED)

# Includes libparquet.so
find_package(Arrow CONFIG REQUIRED)

set(GTEST_LIBRARIES GTest::gtest GTest::gmock GTest::gtest_main)

