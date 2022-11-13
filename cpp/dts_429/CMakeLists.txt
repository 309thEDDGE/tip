set(headers
    include/dts429.h
    include/organize_429_icd.h
    include/arinc429_data.h
)

add_library(dts_429 
    src/dts429.cpp
    src/organize_429_icd.cpp
    src/arinc429_data.cpp
)

target_compile_features(dts_429 PUBLIC cxx_std_17)

target_include_directories(dts_429
    PUBLIC include
)

set_target_properties(dts_429
    PROPERTIES PUBLIC_HEADER "${headers}"
)

target_link_libraries(dts_429
  PRIVATE spdlog::spdlog yaml-cpp tiputil yamlsv 
)

target_link_libraries(dts_429
  PUBLIC dts_1553
)

if(BUILD_SHARED_LIBS)
    install(TARGETS dts_429)
endif()

