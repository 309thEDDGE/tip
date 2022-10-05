
find_path(yaml-cpp_INCLUDE_DIR 
    NAMES yaml.h 
    PATHS ${yaml-cpp_include_paths}
    REQUIRED
    NO_DEFAULT_PATH
)

# Includes for yaml are written as [#include "yaml-cpp/yaml.h"]
get_filename_component(yaml-cpp_include_dirname ${yaml-cpp_INCLUDE_DIR} NAME)
if(${yaml-cpp_include_dirname} STREQUAL yaml-cpp)
    get_filename_component(yaml-cpp_INCLUDE_DIR ${yaml-cpp_INCLUDE_DIR} DIRECTORY)
endif()
# message("yaml-cpp_INCLUDE_DIR: ${yaml-cpp_INCLUDE_DIR}")

find_library(yaml-cpp_LIBRARY
    NAMES "${yaml-cpp_lib_names}"
    PATHS "${yaml-cpp_lib_paths}" 
    REQUIRED
    NO_DEFAULT_PATH
)
# message("yaml-cpp_LIBRARY: ${yaml-cpp_LIBRARY}")

set(yaml-cpp_LIBRARIES ${yaml-cpp_LIBRARY})
set(yaml-cpp_INCLUDE_DIRS ${yaml-cpp_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(yaml-cpp DEFAULT_MSG 
    yaml-cpp_LIBRARY yaml-cpp_INCLUDE_DIR 
)

if(yaml-cpp_FOUND)
    set(yaml-cpp_FOUND TRUE)
    add_library(yaml-cpp UNKNOWN IMPORTED)
    set_target_properties(yaml-cpp
        PROPERTIES IMPORTED_LOCATION "${yaml-cpp_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${yaml-cpp_INCLUDE_DIR}")
endif()