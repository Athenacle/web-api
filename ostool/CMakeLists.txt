find_package(Threads)
aux_source_directory(src/ OSTOOL_SRC)

add_library(ostool STATIC ${OSTOOL_SRC})
target_link_libraries(ostool PUBLIC Threads::Threads)
