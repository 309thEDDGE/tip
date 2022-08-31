find_library(Lz4_LIBRARY
    NAMES ${Lz4_lib_names}
    PATHS ${Lz4_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Lz4_LIBRARY: ${Lz4_LIBRARY}")

set(Lz4_LIBRARIES ${Lz4_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Lz4 DEFAULT_MSG 
    Lz4_LIBRARY 
)

if(Lz4_FOUND)
    set(Lz4_FOUND TRUE)
    add_library(lz4 UNKNOWN IMPORTED)
    set_target_properties(lz4
        PROPERTIES IMPORTED_LOCATION "${Lz4_LIBRARY}"
    )
endif()