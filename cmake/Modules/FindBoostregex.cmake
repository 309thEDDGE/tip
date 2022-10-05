find_library(Boostregex_LIBRARY
    NAMES ${Boostregex_lib_names}
    PATHS ${Boostregex_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Boostregex_LIBRARY: ${Boostregex_LIBRARY}")

set(Boostregex_LIBRARIES ${Boostregex_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Boostregex DEFAULT_MSG 
    Boostregex_LIBRARY 
)

if(Boostregex_FOUND)
    set(Boostregex_FOUND TRUE)
    add_library(boost_regex UNKNOWN IMPORTED)
    set_target_properties(boost_regex
        PROPERTIES IMPORTED_LOCATION "${Boostregex_LIBRARY}"
    )
endif()