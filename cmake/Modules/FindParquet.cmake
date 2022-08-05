find_package(Arrow REQUIRED)

find_path(Parquet_INCLUDE_DIR 
    NAMES reader.h 
    PATHS ${Parquet_include_paths}
    REQUIRED
    NO_DEFAULT_PATH
)

# Includes for Arrow are written as [#include "parquet/api.h"]
get_filename_component(Parquet_include_dirname ${Parquet_INCLUDE_DIR} NAME)
if(${Parquet_include_dirname} STREQUAL api)
    get_filename_component(Parquet_INCLUDE_DIR ${Parquet_INCLUDE_DIR} DIRECTORY)
endif()
if(${Parquet_include_dirname} STREQUAL parquet)
    get_filename_component(Parquet_INCLUDE_DIR ${Parquet_INCLUDE_DIR} DIRECTORY)
endif()
# message("Parquet_INCLUDE_DIR: ${Parquet_INCLUDE_DIR}")

find_library(Parquet_LIBRARY
    NAMES ${Parquet_lib_names}
    PATHS ${Parquet_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Parquet_LIBRARY: ${Parquet_LIBRARY}")

set(Parquet_LIBRARIES ${Parquet_LIBRARY})
set(Parquet_INCLUDE_DIRS ${Parquet_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Parquet DEFAULT_MSG 
    Parquet_LIBRARY Parquet_INCLUDE_DIR 
)

if(Parquet_FOUND)
    set(Parquet_FOUND TRUE)
    add_library(parquet UNKNOWN IMPORTED)
    set_target_properties(parquet
        PROPERTIES IMPORTED_LOCATION "${Parquet_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Parquet_INCLUDE_DIR}")
    target_link_libraries(parquet INTERFACE arrow_shared)
endif()
