
if(${USE_NEWARROW})
	add_compile_definitions(NEWARROW)
endif()

add_compile_definitions(
    PARQUET_STATIC
    ARROW_STATIC
    SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
    VERSION_STRING="${GIT_VERSION_STRING}"
    CH10_PARSE_EXE_NAME="${CH10_PARSE_EXE_NAME}"
    TRANSLATE_1553_EXE_NAME="${TRANSLATE_1553_EXE_NAME}"
    TRANSLATE_429_EXE_NAME="${TRANSLATE_429_EXE_NAME}"
    TINS_STATIC
    _CRT_SECURE_NO_WARNINGS
    _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
    __WIN64
)

message(STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

    add_compile_options(/wd4101 /wd4146)

    if("${CMAKE_BUILD_TYPE}" STREQUAL Profile)
        message(WARNING "${CMAKE_CXX_COMPILER_ID} compiler flags not configured for build type ${CMAKE_BUILD_TYPE}")
    elseif("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
        add_compile_options(/Od /Zi)
    else()
        add_compile_options(/wd4244)
        add_compile_options(/O2)
    endif()

else()
    message(WARN "Build flags for compiler ID ${CMAKE_CXX_COMPILER_ID} not configured.")
endif()




