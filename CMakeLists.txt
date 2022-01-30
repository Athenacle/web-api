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

option(ENABLE_WEB_API "Build Web-API" ON)
option(ENABLE_APK_MANAGER "Build apk-manager" ON)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)

include_directories(${CMAKE_BINARY_DIR})

add_subdirectory(ostool)

if (ENABLE_WEB_API)
    add_subdirectory(web-api/)
endif ()

if (ENABLE_APK_MANAGER)
    add_subdirectory(apk-manager/)
endif ()
