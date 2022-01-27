cmake_minimum_required(VERSION 3.14)

project(web-api)

set(MAJOR_VERSION 0)
set(MINOR_VERSION 0)
set(PATCH_VERSION 1)
set(PROJECT_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)
set(CMAKE_CXX_STANDARD 20)

include(CPPMHD)
include(JSON)
include(Check)

find_package(OpenSSL REQUIRED)

find_package(spdlog)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)

# find_package(cppmhd)
aux_source_directory(src/v1 V1_SRC)
add_library(v1 STATIC ${V1_SRC})
target_link_libraries(v1 PUBLIC CPPMHD::CPPMHD spdlog::spdlog OpenSSL::Crypto)
target_include_directories(v1 PUBLIC src/ PUBLIC ${CMAKE_BINARY_DIR})

add_executable(${PROJECT_NAME} src/app.cc src/utils.cc src/serialize.cc src/logger.cc)
target_link_libraries(${PROJECT_NAME} PUBLIC CPPMHD::CPPMHD v1)
add_dependencies(${PROJECT_NAME} json)
