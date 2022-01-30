#include "am.h"
#include "config.h"

#include <git2/global.h>
#include <git2/repository.h>
#include <spdlog/spdlog.h>


int main()
{
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::trace);
#endif
    setup_git();
    git_repository *repo = open_repo();
    fetch_repo(repo);

    git_repository_free(repo);
    git_libgit2_shutdown();
}