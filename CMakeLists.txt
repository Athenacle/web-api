cmake_minimum_required(VERSION 3.14)

project(web-api)

set(MAJOR_VERSION 0)
set(MINOR_VERSION 0)
set(PATCH_VERSION 1)
set(PROJECT_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)
set(CMAKE_CXX_STANDARD 20)

include(JSON)
include(Check)

find_package(OpenSSL REQUIRED)

find_package(spdlog)

add_subdirectory(ostool)
add_subdirectory(web-api/)
