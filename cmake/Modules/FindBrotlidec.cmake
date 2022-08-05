find_library(Brotlidec_LIBRARY
    NAMES ${Brotlidec_lib_names}
    PATHS ${Brotlidec_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Brotlidec_LIBRARY: ${Brotlidec_LIBRARY}")

set(Brotlidec_LIBRARIES ${Brotlidec_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Brotlidec DEFAULT_MSG 
    Brotlidec_LIBRARY 
)

if(Brotlidec_FOUND)
    set(Brotlidec_FOUND TRUE)
    add_library(brotlidec UNKNOWN IMPORTED)
    set_target_properties(brotlidec
        PROPERTIES IMPORTED_LOCATION "${Brotlidec_LIBRARY}"
    )
endif()