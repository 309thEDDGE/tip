set(BUILD_SHARED_LIBS OFF)

# NEWARROW is used if the new (~0.17+) version of
# Arrow is used. Currently the version of arrow supplied by the 
# conda build requires NEWARROW to be defined.
if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")

	add_compile_definitions(
		TINS_STATIC
		NEWARROW
      )

   link_directories("${CONDA_PREFIX}\\Library\\lib")

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

   add_definitions(
		-DNEWARROW
		-DTINS_STATIC)

else()
    message(FATAL_ERROR "No system-specific options: " ${CMAKE_SYSTEM_NAME})
endif()

if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")

   add_compile_options(/EHsc)
   if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
      message(FATAL_ERROR "Windows Conda build with build type ${CMAKE_BUILD_TYPE} not implemented")
   elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Profile")
      message(FATAL_ERROR "Windows Conda build with build type ${CMAKE_BUILD_TYPE} not implemented")
   else()
      set(GTEST_LIBRARIES gtest-md gmock_main-md gtest_main-md)
   endif()

else()
   set(GTEST_LIBRARIES gtest gmock gtest_main)
endif()

###########################################
#              Dependencies
###########################################
find_package(Arrow REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(spdlog REQUIRED)

if("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
   find_package(PkgConfig)
   pkg_check_modules(LIBTINS REQUIRED libtins)
   pkg_check_modules(LIBPCAP REQUIRED libpcap)
endif()

