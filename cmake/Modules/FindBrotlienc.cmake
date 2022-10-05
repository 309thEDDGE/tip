find_library(Brotlienc_LIBRARY
    NAMES ${Brotlienc_lib_names}
    PATHS ${Brotlienc_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Brotlienc_LIBRARY: ${Brotlienc_LIBRARY}")

set(Brotlienc_LIBRARIES ${Brotlienc_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Brotlienc DEFAULT_MSG 
    Brotlienc_LIBRARY 
)

if(Brotlienc_FOUND)
    set(Brotlienc_FOUND TRUE)
    add_library(brotlienc UNKNOWN IMPORTED)
    set_target_properties(brotlienc
        PROPERTIES IMPORTED_LOCATION "${Brotlienc_LIBRARY}"
    )
endif()