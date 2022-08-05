find_library(Zlib_LIBRARY
    NAMES ${Zlib_lib_names}
    PATHS ${Zlib_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Zlib_LIBRARY: ${Zlib_LIBRARY}")

set(Zlib_LIBRARIES ${Zlib_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Zlib DEFAULT_MSG 
    Zlib_LIBRARY 
)

if(Zlib_FOUND)
    set(Zlib_FOUND TRUE)
    add_library(zlib UNKNOWN IMPORTED)
    set_target_properties(zlib
        PROPERTIES IMPORTED_LOCATION "${Zlib_LIBRARY}"
    )
endif()