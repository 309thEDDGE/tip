
set(BUILD_SHARED_LIBS OFF)

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

# The logic to define NEWARROW21 below will only work after find_package(Arrow...) is called.
if(${Arrow_VERSION} VERSION_GREATER_EQUAL "21.0")
	add_definitions(-DNEWARROW21)
	message(STATUS "Define macro NEWARROW21: arrow version >= 21.0")
else()
	add_definitions(-DNEWARROW)
endif()

set(GTEST_LIBRARIES GTest::gtest GTest::gmock GTest::gtest_main)

