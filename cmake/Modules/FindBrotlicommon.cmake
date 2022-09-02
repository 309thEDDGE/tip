find_library(Brotlicommon_LIBRARY
    NAMES ${Brotlicommon_lib_names}
    PATHS ${Brotlicommon_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Brotlicommon_LIBRARY: ${Brotlicommon_LIBRARY}")

set(Brotlicommon_LIBRARIES ${Brotlicommon_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Brotlicommon DEFAULT_MSG 
    Brotlicommon_LIBRARY 
)

if(Brotlicommon_FOUND)
    set(Brotlicommon_FOUND TRUE)
    add_library(brotlicommon UNKNOWN IMPORTED)
    set_target_properties(brotlicommon
        PROPERTIES IMPORTED_LOCATION "${Brotlicommon_LIBRARY}"
    )
endif()