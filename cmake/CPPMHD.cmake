include(ExternalProject)

set(GTEST_ROOT ${CMAKE_CURRENT_BINARY_DIR}/3party/cppmhd CACHE FILEPATH "")

ExternalProject_Add(
    cppmhd
    URL https://github.com/Athenacle/cppmhd/archive/refs/tags/v0.0.2.tar.gz
    URL_HASH SHA256=fc14dcf368270591970f254f2552d6846de616d56590212b377190ed143c15be
    DOWNLOAD_NO_PROGRESS ON
    PREFIX ${GTEST_ROOT}
    BUILD_BYPRODUCTS ${GTEST_ROOT}/cppmhd-build/lib/libcppmhd.so
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
               -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} -DBUILD_STATIC=ON
    LOG_INSTALL OFF
    INSTALL_COMMAND "")

ExternalProject_Get_Property(cppmhd source_dir binary_dir)

file(MAKE_DIRECTORY ${source_dir}/lib/include)

find_package(spdlog REQUIRED)

add_library(CPPMHD::CPPMHD STATIC IMPORTED)
set_target_properties(
    CPPMHD::CPPMHD
    PROPERTIES IMPORTED_GLOBAL TRUE INTERFACE_INCLUDE_DIRECTORIES ${source_dir}/lib/include
               INTERFACE_LINK_DIRECTORIES ${binary_dir}/lib/ IMPORTED_LOCATION
                                                             ${binary_dir}/lib/libcppmhd.so)

add_dependencies(CPPMHD::CPPMHD cppmhd)
