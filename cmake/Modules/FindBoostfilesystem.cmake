find_library(Boostfilesystem_LIBRARY
    NAMES ${Boostfilesystem_lib_names}
    PATHS ${Boostfilesystem_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Boostfilesystem_LIBRARY: ${Boostfilesystem_LIBRARY}")

set(Boostfilesystem_LIBRARIES ${Boostfilesystem_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Boostfilesystem DEFAULT_MSG 
    Boostfilesystem_LIBRARY 
)

if(Boostfilesystem_FOUND)
    set(Boostfilesystem_FOUND TRUE)
    add_library(boost_filesystem UNKNOWN IMPORTED)
    set_target_properties(boost_filesystem
        PROPERTIES IMPORTED_LOCATION "${Boostfilesystem_LIBRARY}"
    )
endif()