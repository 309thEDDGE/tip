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
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    if("${CMAKE_BUILD_TYPE}" STREQUAL Profile)
        add_compile_options(--coverage -O1)
        add_link_options(--coverage)
    elseif("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
        add_compile_options(-g -ggdb -O0)
    else()
        add_compile_options(-s -O3 -Wno-switch)
    endif()

else()
    message(WARN "Build flags for compiler ID ${CMAKE_CXX_COMPILER_ID} not handled.")
endif()

if(("${USER_INSTALL_DIR}" STREQUAL OFF) OR ("${USE_DEPS_DIR}" STREQUAL OFF))
    include(GNUInstallDirs)
endif()
