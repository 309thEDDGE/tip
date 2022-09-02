find_library(Jemalloc_LIBRARY
    NAMES ${Jemalloc_lib_names}
    PATHS ${Jemalloc_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Jemalloc_LIBRARY: ${Jemalloc_LIBRARY}")

set(Jemalloc_LIBRARIES ${Jemalloc_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Jemalloc DEFAULT_MSG 
    Jemalloc_LIBRARY 
)

if(Jemalloc_FOUND)
    set(Jemalloc_FOUND TRUE)
    add_library(jemalloc UNKNOWN IMPORTED)
    set_target_properties(jemalloc
        PROPERTIES IMPORTED_LOCATION "${Jemalloc_LIBRARY}"
    )
endif()
