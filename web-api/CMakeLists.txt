include(CPPMHD)

# find_package(cppmhd)
aux_source_directory(v1/ V1_SRC)
add_library(v1 STATIC ${V1_SRC})
target_link_libraries(v1 PUBLIC CPPMHD::CPPMHD spdlog::spdlog OpenSSL::Crypto)
target_include_directories(v1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} app.cc utils.cc serialize.cc logger.cc)
target_link_libraries(${PROJECT_NAME} PUBLIC CPPMHD::CPPMHD v1)
add_dependencies(${PROJECT_NAME} json)
