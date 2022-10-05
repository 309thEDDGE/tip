
find_package(GMock)

find_path(GTest_INCLUDE_DIR 
    NAMES gtest.h 
    PATHS ${GTest_include_paths}
    REQUIRED
    NO_DEFAULT_PATH
)

# Includes for GTest are written as [#include "GTest/GTest.h"]
get_filename_component(GTest_include_dirname ${GTest_INCLUDE_DIR} NAME)
if(${GTest_include_dirname} STREQUAL gtest)
    get_filename_component(GTest_INCLUDE_DIR ${GTest_INCLUDE_DIR} DIRECTORY)
endif()
# message("GTest_INCLUDE_DIR: ${GTest_INCLUDE_DIR}")

find_library(GTest_LIBRARY
    NAMES ${GTest_lib_names}
    PATHS ${GTest_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("GTest_LIBRARY: ${GTest_LIBRARY}")

set(GTest_LIBRARIES ${GTest_LIBRARY})
set(GTest_INCLUDE_DIRS ${GTest_INCLUDE_DIR})

find_library(GTest_main_LIBRARY
    NAMES ${GTest_main_lib_names}
    PATHS ${GTest_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("GTest_main_LIBRARY: ${GTest_main_LIBRARY}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GTest DEFAULT_MSG 
    GTest_LIBRARY GTest_INCLUDE_DIR 
)

if(GTest_FOUND)
    set(GTest_FOUND TRUE)
    add_library(GTest::gtest UNKNOWN IMPORTED)
    set_target_properties(GTest::gtest
        PROPERTIES IMPORTED_LOCATION "${GTest_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GTest_INCLUDE_DIR}")
endif()

if(GTest_FOUND)
    set(GTest_FOUND TRUE)
    add_library(GTest::gtest_main UNKNOWN IMPORTED)
    set_target_properties(GTest::gtest_main
        PROPERTIES IMPORTED_LOCATION "${GTest_main_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GTest_INCLUDE_DIR}")
endif()