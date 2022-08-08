find_library(Flatbuffers_LIBRARY
    NAMES ${Flatbuffers_lib_names}
    PATHS ${Flatbuffers_lib_paths}
    REQUIRED
    NO_DEFAULT_PATH
)
# message("Flatbuffers_LIBRARY: ${Flatbuffers_LIBRARY}")

set(Flatbuffers_LIBRARIES ${Flatbuffers_LIBRARY})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Flatbuffers DEFAULT_MSG 
    Flatbuffers_LIBRARY 
)

if(Flatbuffers_FOUND)
    set(Flatbuffers_FOUND TRUE)
    add_library(flatbuffers UNKNOWN IMPORTED)
    set_target_properties(flatbuffers
        PROPERTIES IMPORTED_LOCATION "${Flatbuffers_LIBRARY}"
    )
endif()