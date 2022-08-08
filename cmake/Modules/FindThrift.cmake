find_library(Thrift_LIBRARY
    NAMES ${Thrift_lib_names}
    PATHS ${Thrift_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Thrift_LIBRARY: ${Thrift_LIBRARY}")

set(Thrift_LIBRARIES ${Thrift_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Thrift DEFAULT_MSG 
    Thrift_LIBRARY 
)

if(Thrift_FOUND)
    set(Thrift_FOUND TRUE)
    add_library(thrift UNKNOWN IMPORTED)
    set_target_properties(thrift
        PROPERTIES IMPORTED_LOCATION "${Thrift_LIBRARY}"
    )
endif()