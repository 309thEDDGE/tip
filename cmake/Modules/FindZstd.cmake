find_library(Zstd_LIBRARY
    NAMES ${Zstd_lib_names}
    PATHS ${Zstd_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Zstd_LIBRARY: ${Zstd_LIBRARY}")

set(Zstd_LIBRARIES ${Zstd_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Zstd DEFAULT_MSG 
    Zstd_LIBRARY 
)

if(Zstd_FOUND)
    set(Zstd_FOUND TRUE)
    add_library(zstd UNKNOWN IMPORTED)
    set_target_properties(zstd
        PROPERTIES IMPORTED_LOCATION "${Zstd_LIBRARY}"
    )
endif()