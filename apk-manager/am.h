
#define GH_REPO "https://github.com/Athenacle/abuild.git"

#define DEFAULT_ORIGIN "origin"

#define DEFAULT_BRANCH "refs/remotes/origin/master"

#define DEFAULT_REMOTE_MASTER_HEAD DEFAULT_BRANCH

#ifndef NDEBUG
#define REPO_ROOT "/tmp/abuild"
#else
#define REPO_ROOT "."
#endif

struct git_repository;

void checkout_origin_master(git_repository *repo);
int fetch_repo(git_repository *repo);
git_repository *open_repo();
void free_remote(git_repository *);

void setup_git();
void shutdown_git();