
find_path(GMock_INCLUDE_DIR 
    NAMES gmock.h 
    PATHS ${GMock_include_paths}
    REQUIRED
    NO_DEFAULT_PATH
)

# Includes for GMock are written as [#include "GMock/GMock.h"]
get_filename_component(GMock_include_dirname ${GMock_INCLUDE_DIR} NAME)
if(${GMock_include_dirname} STREQUAL gmock)
    get_filename_component(GMock_INCLUDE_DIR ${GMock_INCLUDE_DIR} DIRECTORY)
endif()
# message("GMock_INCLUDE_DIR: ${GMock_INCLUDE_DIR}")

find_library(GMock_LIBRARY
    NAMES ${GMock_lib_names}
    PATHS ${GMock_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("GMock_LIBRARY: ${GMock_LIBRARY}")

set(GMock_LIBRARIES ${GMock_LIBRARY})
set(GMock_INCLUDE_DIRS ${GMock_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMock DEFAULT_MSG 
    GMock_LIBRARY GMock_INCLUDE_DIR 
)

if(GMock_FOUND)
    set(GMock_FOUND TRUE)
    add_library(GTest::gmock UNKNOWN IMPORTED)
    set_target_properties(GTest::gmock
        PROPERTIES IMPORTED_LOCATION "${GMock_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GMock_INCLUDE_DIR}")
endif()
