find_package(CURL)
find_package(PkgConfig QUIET)

pkg_check_modules(GIT2 libgit2 REQUIRED)

add_executable(apk-manager apk-manager.cc git.cc)
target_include_directories(apk-manager PRIVATE ${GIT2_INC})

target_link_libraries(apk-manager PRIVATE ${GIT2_LIBRARIES} spdlog::spdlog ostool)
