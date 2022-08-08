find_library(Glog_LIBRARY
    NAMES ${Glog_lib_names}
    PATHS ${Glog_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Glog_LIBRARY: ${Glog_LIBRARY}")

set(Glog_LIBRARIES ${Glog_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Glog DEFAULT_MSG 
    Glog_LIBRARY 
)

if(Glog_FOUND)
    set(Glog_FOUND TRUE)
    add_library(glog UNKNOWN IMPORTED)
    set_target_properties(glog
        PROPERTIES IMPORTED_LOCATION "${Glog_LIBRARY}"
    )
endif()
