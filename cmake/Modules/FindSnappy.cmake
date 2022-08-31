find_library(Snappy_LIBRARY
    NAMES ${Snappy_lib_names}
    PATHS ${Snappy_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Snappy_LIBRARY: ${Snappy_LIBRARY}")

set(Snappy_LIBRARIES ${Snappy_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Snappy DEFAULT_MSG 
    Snappy_LIBRARY 
)

if(Snappy_FOUND)
    set(Snappy_FOUND TRUE)
    add_library(snappy UNKNOWN IMPORTED)
    set_target_properties(snappy
        PROPERTIES IMPORTED_LOCATION "${Snappy_LIBRARY}"
    )
endif()