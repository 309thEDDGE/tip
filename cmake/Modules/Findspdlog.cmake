
find_path(spdlog_INCLUDE_DIR 
    NAMES spdlog.h 
    PATHS ${spdlog_include_paths}
    REQUIRED
    NO_DEFAULT_PATH
)

# Includes for spdlog are written as [#include "spdlog/spdlog.h"]
get_filename_component(spdlog_include_dirname ${spdlog_INCLUDE_DIR} NAME)
if(${spdlog_include_dirname} STREQUAL spdlog)
    get_filename_component(spdlog_INCLUDE_DIR ${spdlog_INCLUDE_DIR} DIRECTORY)
endif()

# message("spdlog_INCLUDE_DIR: ${spdlog_INCLUDE_DIR}")

find_library(spdlog_LIBRARY
    NAMES ${spdlog_lib_names}
    PATHS ${spdlog_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("spdlog_LIBRARY: ${spdlog_LIBRARY}")

set(spdlog_LIBRARIES ${spdlog_LIBRARY})
set(spdlog_INCLUDE_DIRS ${spdlog_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(spdlog DEFAULT_MSG 
    spdlog_LIBRARY spdlog_INCLUDE_DIR 
)

if(spdlog_FOUND)
    set(spdlog_FOUND TRUE)
    add_library(spdlog::spdlog UNKNOWN IMPORTED)
    set_target_properties(spdlog::spdlog
        PROPERTIES IMPORTED_LOCATION "${spdlog_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${spdlog_INCLUDE_DIR}")
endif()