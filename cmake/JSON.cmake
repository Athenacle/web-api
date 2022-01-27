include(ExternalProject)

set(JSON_ROOT ${CMAKE_CURRENT_BINARY_DIR}/3party/json CACHE FILEPATH "")

ExternalProject_Add(
    json URL https://github.com/nlohmann/json/archive/refs/tags/v3.10.5.tar.gz PREFIX ${JSON_ROOT}
    URL_HASH SHA256=5daca6ca216495edf89d167f808d1d03c4a4d929cef7da5e10f135ae1540c7e4 BUILD_COMMAND
             "" LOG_INSTALL OFF INSTALL_COMMAND "")

ExternalProject_Get_Property(json source_dir binary_dir)

file(MAKE_DIRECTORY ${source_dir}/include)
include_directories(${source_dir}/include)
