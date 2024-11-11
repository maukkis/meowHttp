cmake_minimum_required(VERSION 3.25.1)

project(http-client
DESCRIPTION "woof")
find_package(OpenSSL REQUIRED)
set(CMAKE_CXX_STANDARD 23)
if(NOT CMAKE_BUILD_TYPE)
  set(DCKMAKE_BUILD_TYPE Release)
endif()
file(GLOB_RECURSE http-client_src "src/*.cpp"
"src/*.h")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-03 -Wall -Wextra")
add_executable(nya ${http-client_src})

target_link_libraries(nya PRIVATE OpenSSL::SSL OpenSSL::Crypto)
