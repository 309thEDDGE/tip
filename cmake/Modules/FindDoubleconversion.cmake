find_library(Doubleconversion_LIBRARY
    NAMES ${Doubleconversion_lib_names}
    PATHS ${Doubleconversion_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Doubleconversion_LIBRARY: ${Doubleconversion_LIBRARY}")

set(Doubleconversion_LIBRARIES ${Doubleconversion_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Doubleconversion DEFAULT_MSG 
    Doubleconversion_LIBRARY 
)

if(Doubleconversion_FOUND)
    set(Doubleconversion_FOUND TRUE)
    add_library(doubleconversion UNKNOWN IMPORTED)
    set_target_properties(doubleconversion
        PROPERTIES IMPORTED_LOCATION "${Doubleconversion_LIBRARY}"
    )
endif()