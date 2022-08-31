find_library(Boostsystem_LIBRARY
    NAMES ${Boostsystem_lib_names}
    PATHS ${Boostsystem_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Boostsystem_LIBRARY: ${Boostsystem_LIBRARY}")

set(Boostsystem_LIBRARIES ${Boostsystem_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Boostsystem DEFAULT_MSG 
    Boostsystem_LIBRARY 
)

if(Boostsystem_FOUND)
    set(Boostsystem_FOUND TRUE)
    add_library(boost_system UNKNOWN IMPORTED)
    set_target_properties(boost_system
        PROPERTIES IMPORTED_LOCATION "${Boostsystem_LIBRARY}"
    )
endif()