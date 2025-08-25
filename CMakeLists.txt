cmake_minimum_required(VERSION 3.25.1)
project(meowHttp)
find_package(OpenSSL REQUIRED)
set(CMAKE_CXX_STANDARD 23)
set(meowHttp_version 1.0)
enable_testing()
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
  add_subdirectory(${PROJECT_SOURCE_DIR}/test)
endif()
file(GLOB_RECURSE http-client_src "src/*.cpp")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -Wextra")
add_library(meowHttp ${http-client_src})
set_property(TARGET meowHttp PROPERTY VERSION ${meowHttp_version})
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  target_link_libraries(meowHttp PUBLIC OpenSSL::SSL OpenSSL::Crypto socket)
elseif(WIN32)
  target_link_libraries(meowHttp PUBLIC OpenSSL::SSL OpenSSL::Crypto wsock32 ws2_32)
else()
  target_link_libraries(meowHttp PUBLIC OpenSSL::SSL OpenSSL::Crypto)
endif()
target_include_directories(meowHttp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
  $<INSTALL_INTERFACE:include/meowHttp>)

install(TARGETS meowHttp EXPORT meowHttpTargets DESTINATION lib)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/includes// DESTINATION include/meowHttp)
include(CMakePackageConfigHelpers)
set(config_dir lib/cmake/meowHttp)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/meowHttp/meowHttpConfigVersion.cmake"
  VERSION ${meowHttp_version}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/meowHttp/meowHttpConfig.cmake"
  INSTALL_DESTINATION ${config_dir}
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

export(
  EXPORT meowHttpTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/meowHttp/meowHttpTargets.cmake"
)

install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/meowHttp/meowHttpConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/meowHttp/meowHttpConfigVersion.cmake
  DESTINATION ${config_dir}
)

install(EXPORT meowHttpTargets
  FILE
    meowHttpTargets.cmake
  DESTINATION
    ${config_dir}
)
