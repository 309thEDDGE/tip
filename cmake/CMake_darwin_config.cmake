if(${USE_NEWARROW})
	add_definitions(-DNEWARROW)
endif()

add_definitions(
    -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
    -DVERSION_STRING="${GIT_VERSION_STRING}"
    -DCH10_PARSE_EXE_NAME="${CH10_PARSE_EXE_NAME}"
    -DTRANSLATE_1553_EXE_NAME="${TRANSLATE_1553_EXE_NAME}"
    -DTRANSLATE_429_EXE_NAME="${TRANSLATE_429_EXE_NAME}"
    -D__linux__
    -DPARQUET_STATIC
    -DARROW_STATIC
)

message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
# Use MATCHES (regex) because CMAKE_CXX_COMPILER_ID 
# includes version, "Clang 14.xxx"
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang.*")

    if("${CMAKE_BUILD_TYPE}" STREQUAL Profile)
        message(FATAL_ERROR "CMAKE_BUILD_TYPE: Profile not supported for ${CMAKE_CXX_COMPILER_ID}")
    elseif("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
        message(FATAL_ERROR "CMAKE_BUILD_TYPE: Profile not supported for ${CMAKE_CXX_COMPILER_ID}")
    else()
        add_compile_options(-O3)
        link_libraries(-lc++fs)
    endif()

else()
    message(WARN "Build flags for compiler ID ${CMAKE_CXX_COMPILER_ID} not handled.")
endif()

include(GNUInstallDirs)