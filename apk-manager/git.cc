#include "am.h"

#include <git2/annotated_commit.h>
#include <git2/checkout.h>
#include <git2/commit.h>
#include <git2/errors.h>
#include <git2/global.h>
#include <git2/remote.h>
#include <git2/repository.h>
#include <spdlog/spdlog.h>

namespace fmt
{
template <>
struct formatter<git_oid> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const git_oid &oid, FormatContext &ctx)
    {
        char buf[GIT_OID_HEXSZ + 1];
        git_oid_fmt(buf, &oid);
        buf[GIT_OID_HEXSZ] = '\0';

        return format_to(ctx.out(), "{}", buf);
    }
};

template <>
struct formatter<git_error> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const git_error &err, FormatContext &ctx)
    {
        return format_to(ctx.out(), "{}({})", err.message, err.klass);
    }
};

}  // namespace fmt


namespace
{

struct GitStatus {
    double received;
    git_oid remote_master;

    GitStatus()
    {
        memset(&remote_master, 0, sizeof(remote_master));
        received = 0;
    }
};

std::shared_ptr<spdlog::logger> gl;



static int fetch_progress_cb(const char *str, int len, void *)
{
    static std::string msg;
    msg.append(str, len);
    while (true) {
        auto r = msg.find_first_of('\r');
        auto n = msg.find_first_of('\n');
        auto m = std::min(r, n);
        if (m == msg.npos) {
            break;
        }

        std::string now = msg.substr(0, m);
        gl->info("fetch: remote: {}", now);
        msg = msg.substr(m + 1);
    }

    return 0;
}

static int fetch_update_cb(const char *refname, const git_oid *a, const git_oid *b, void *payload)
{
    auto up = reinterpret_cast<GitStatus *>(payload);

    git_oid aid, bid;
    aid = *a;
    bid = *b;

    if (git_oid_is_zero(a)) {
        gl->info("fetch: [new] {} {}", bid, refname);
    } else {
        gl->info("fetch: [updated] {} .. {} {}", aid, bid, refname);
    }

    if (strcmp(refname, DEFAULT_BRANCH) == 0) {
        up->remote_master = *b;
    }

    return 0;
}

static int fetch_transfer_progress_cb(const git_indexer_progress *stats, void *payload)
{
    auto up = reinterpret_cast<GitStatus *>(payload);

    if (stats->received_objects == stats->total_objects) {
        gl->info("fetch: resolving deltas {}/{}", stats->indexed_deltas, stats->total_deltas);
    } else if (stats->total_objects > 0) {
        double received = ((1.0 * stats->indexed_objects) / stats->total_objects) * 100;
        if (received - up->received > 10) {
            gl->info("fetch: received {}/{}({:.2}%) objects ({}) in {} bytes",
                     stats->received_objects,
                     stats->total_objects,
                     received,
                     stats->indexed_objects,
                     stats->received_bytes);
            up->received = received;
        }
    }
    return 0;
}

static git_remote *create_default_remote(git_repository *repo)
{
    git_remote *remote = nullptr;
    git_remote_create(&remote, repo, DEFAULT_ORIGIN, GH_REPO);
    return remote;
}


static void checkout_print_checkout_progress(const char *path, size_t completed_steps, size_t total_steps, void *)
{
    if (total_steps != 0) {
        if (path == nullptr) {
            gl->info("checkout: checkout starting, total {} steps", total_steps);
        } else {
            gl->info("checkout: '{}' {}/{} ", path, completed_steps, total_steps);
        }
    }
}


static void checkout_print_perf_data(const git_checkout_perfdata *perfdata, void *)
{
    gl->info("checkout: perf: stat: {} mkdir: {} chmod: {}",
             perfdata->stat_calls,
             perfdata->mkdir_calls,
             perfdata->chmod_calls);
}


}  // namespace

void setup_git()
{
    git_libgit2_init();
    gl = spdlog::default_logger()->clone("git");
}

git_repository *open_repo()
{
    git_repository *repo = nullptr;
    if (git_repository_open_ext(&repo, REPO_ROOT, 0, nullptr) == 0) {
        gl->info("open git repository '{}' success.", REPO_ROOT);
    } else {
        gl->error("open repository '{}' failed.", REPO_ROOT);
        exit(1);
    }
    return repo;
}



void checkout_origin_master(git_repository *repo, GitStatus &gs)
{
    git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;
    git_reference *ref = NULL, *branch = NULL;
    git_commit *target_commit = NULL;
    int err;

    checkout_opts.checkout_strategy = GIT_CHECKOUT_SAFE;
    checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    checkout_opts.progress_cb = checkout_print_checkout_progress;
    checkout_opts.perfdata_cb = checkout_print_perf_data;

    if (git_oid_is_zero(&gs.remote_master)) {
        git_reference *remote_ref = nullptr;
        err = git_reference_lookup(&remote_ref, repo, DEFAULT_REMOTE_MASTER_HEAD);
        if (err != 0) {
            gl->critical("checkout: lookup reference {} failed: {}", DEFAULT_REMOTE_MASTER_HEAD, *git_error_last());
        } else {
            gs.remote_master = *git_reference_target(remote_ref);
        }
        git_reference_free(remote_ref);
    }
    err = git_commit_lookup(&target_commit, repo, &gs.remote_master);

    if (err != 0) {
        gl->critical("checkout: failed to lookup {}: {}", DEFAULT_BRANCH, *git_error_last());
        goto cleanup;
    }

    gl->info("checkout: checkout oid {}", gs.remote_master);

    err = git_checkout_tree(repo, (const git_object *)target_commit, &checkout_opts);
    if (err != 0) {
        gl->error("checkout: failed to checkout tree: %s", *git_error_last());
        goto cleanup;
    }


    err = git_repository_set_head_detached(repo, &gs.remote_master);


    if (err != 0) {
        gl->error("checkout: failed to update HEAD reference: %s", *git_error_last());
    } else {
        gl->info("checkout: checkout finished.");
    }

cleanup:
    git_commit_free(target_commit);
    git_reference_free(branch);
    git_reference_free(ref);
}

int fetch_repo(git_repository *repo)
{
    git_remote *remote = NULL;
    const git_indexer_progress *stats;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;

    GitStatus gs;

    if (git_remote_lookup(&remote, repo, DEFAULT_ORIGIN) < 0) {
        gl->warn("fetch: no origin found.");
        if ((remote = create_default_remote(repo)) == nullptr) {
            auto err = git_error_last();
            gl->critical("remote: create default remote {}:{} failed: {}", DEFAULT_ORIGIN, GH_REPO, *err);
            exit(1);
        }
    }

    assert(remote != nullptr);

    fetch_opts.callbacks.update_tips = &fetch_update_cb;
    fetch_opts.callbacks.sideband_progress = &fetch_progress_cb;
    fetch_opts.callbacks.transfer_progress = fetch_transfer_progress_cb;
    fetch_opts.callbacks.payload = &gs;

    gl->info("fetch: fetch remote {}", git_remote_url(remote));

    if (git_remote_fetch(remote, nullptr, &fetch_opts, "fetch") < 0) {
        auto err = git_error_last();
        gl->error("fetch error: {}", *err);
        goto on_error;
    }

    stats = git_remote_stats(remote);
    if (stats->local_objects > 0) {
        gl->info("fetch: received {}/{} objects in {} bytes (used {} local objects)",
                 stats->indexed_objects,
                 stats->total_objects,
                 stats->received_bytes,
                 stats->local_objects);
    } else {
        gl->info("fetch: received {}/{} objects in {} bytes.",
                 stats->indexed_objects,
                 stats->total_objects,
                 stats->received_bytes);
    }

    git_remote_free(remote);
    gl->info("fetch: finished.");

    checkout_origin_master(repo, gs);
    return 0;

on_error:
    git_remote_free(remote);
    return -1;
}


void merge_remote(git_repository *repo);
