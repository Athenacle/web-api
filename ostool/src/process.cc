#include "process.h"

#include "fs.h"

#include <unistd.h>

#include <iostream>
#include <queue>
#include <set>
#include <stack>
#include <thread>

#include <sys/wait.h>

using namespace ostool::fs;
using namespace ostool::process;
using namespace ostool::cxxutils;

static bool check(const std::vector<SingleProcessRequest> &in, ErrorCallback cb)
{
    for (const auto &one : in) {
        path p(one.exec);
        if (!is_exectuable(p)) {
            auto err = ENOENT;
            auto errstr = strerror(ENOENT);
            size_t size = one.exec.length() + strlen(errstr) + 128;
            char *buf = new char[size];

            snprintf(buf, size, "exec: exectuable file %s not found", one.exec.c_str());
            cb(err, buf, one);
            delete[] buf;
            return false;
        }
    }

    return true;
}

/*
 * static void free_arg(char **argv, int argc)
 * {
 *     for (int i = 0; i <= argc; i++) {
 *         free(argv[i]);
 *     }
 *     delete[] argv;
 * }
 */

static char **create_arg(const SingleProcessRequest &arg)
{
    int argc = arg.arg.size() + 2;
    int i = 1;
    char **argv = new char *[argc];
    argv[0] = strdup(arg.exec.c_str());
    for (auto first = arg.arg.cbegin(); first < arg.arg.cend() && i < argc; ++first) {
        argv[i] = strdup(first->c_str());
        i++;
    }
    assert(argc == i + 1);
    argv[argc - 1] = nullptr;

    return argv;
}

namespace ostool
{

namespace process
{

class ProcessCtrl
{
  public:
    std::vector<std::tuple<pid_t, SingleProcessRequest>> chain;
    std::set<int> fds;

    int stdout;
    int stdin;

    pid_t groupid;

    pthread_t tid;
    ChildProcessCB cb;
};


void *start_thread(void *ctrl_)
{
    auto ctrl = reinterpret_cast<ProcessCtrl *>(ctrl_);
    size_t size = 8192;
    auto buf = new unsigned char[size];
    const auto &proc = std::get<1>(*ctrl->chain.cend());
    while (true) {
        auto r = read(ctrl->stdout, buf, size);
        ctrl->cb.stdout(buf, r, proc);
        if (r <= 0) {
            break;
        }
    }
    delete[] buf;
    return nullptr;
}

ProcessCtrl *start_process(const ProcessesRequest &req, const ChildProcessCB &cb)
{
    auto &reqs = req.processes;
    if (!check(reqs, cb.error)) {
        return nullptr;
    }
    auto ctrl = new ProcessCtrl;
    ctrl->cb = cb;

    int out_p[2], in_p[2];


    int stdout = 0;
    int stderr = 0;
    int stdin = 0;


    pipe(in_p);
    ctrl->stdin = in_p[1];
    std::set<int> fds;

    fds.emplace(in_p[1]);

    pid_t pgid = 0;

    for (auto iter = reqs.cbegin(); iter != reqs.cend(); ++iter) {
        pipe(out_p);
        auto first = iter == reqs.cbegin();
        pid_t id = fork();
        if (id == 0) {
            close(STDOUT_FILENO);
            close(STDIN_FILENO);
            if (first) {
                assert(ctrl->chain.size() == 0);

                close(out_p[0]);
                close(in_p[1]);

                dup2(in_p[0], STDIN_FILENO);
                dup2(out_p[1], STDOUT_FILENO);

                close(out_p[1]);
                close(in_p[0]);

                auto mypid = getpid();
                setpgid(mypid, mypid);
            } else {
                close(in_p[0]);
                for (auto fd : fds) {
                    if (fd != ctrl->stdout) {
                        close(fd);
                    }
                }

                assert(ctrl->chain.size() > 0);
                assert(ctrl->stdout != 0);

                dup2(ctrl->stdout, STDIN_FILENO);

                close(out_p[0]);

                dup2(out_p[1], STDOUT_FILENO);
                close(out_p[1]);

                setpgid(getpid(), pgid);
            }

            execv(iter->exec.c_str(), create_arg(*iter));

        } else if (id > 0) {
            if (first) {
                assert(pgid == 0);
                pgid = id;
            }

            const auto &process = *iter;
            close(out_p[1]);

            ctrl->chain.emplace_back(id, process);

            ctrl->stdout = out_p[0];
            fds.emplace(out_p[0]);

            cb.start(id, process);
        } else {
            char buf[] = "fork failed\n";
            write(STDERR_FILENO, buf, sizeof(buf));
        }
    }
    close(in_p[0]);

    auto &ins = req.stdin;
    if (ins.size() > 0) {
        write(ctrl->stdin, ins.c_str(), ins.size());
    }
    close(ctrl->stdin);

    ctrl->fds.swap(fds);
    ctrl->fds.erase(ctrl->stdin);
    ctrl->groupid = pgid;

    pthread_create(&ctrl->tid, nullptr, start_thread, ctrl);
    return ctrl;
}

int wait_process(ProcessCtrl *ctrl)
{
    int ret = 0;
    int pg = ctrl->groupid * -1;

    for (size_t i = 0; i < ctrl->chain.size(); i++) {
        auto pid = waitpid(pg, &ret, 0);
        if (pid > 0) {
            auto f = std::find_if(ctrl->chain.cbegin(), ctrl->chain.cend(), [pid](const auto &first) -> bool {
                return std::get<0>(first) == pid;
            });
            assert(f != ctrl->chain.end());
            ctrl->cb.finish(ret, pid, std::get<1>(*f));
        }
    }

    pthread_join(ctrl->tid, nullptr);
    return ret;
}

void free_process(ProcessCtrl *ctrl)
{
    if (ctrl != nullptr) {
        delete ctrl;
    }
}

void start_process_wait(const std::vector<SingleProcessRequest> &in, Buffer &stdout, ErrorCallback errcb, int &err)
{
    Buffer empty;
    start_process_wait(in, std::move(empty), stdout, errcb, err);
}

void start_process_wait(const std::vector<SingleProcessRequest> &in,
                        cxxutils::Buffer &&stdin,
                        cxxutils::Buffer &stdout,
                        ErrorCallback errcb,
                        int &)
{
    ChildProcessCB cb;
    cb.error = errcb;
    cb.stdout = [&stdout](const uint8_t *data, ssize_t s, const SingleProcessRequest &) {
        if (s > 0) {
            stdout.append(data, s);
        }
    };

    ProcessesRequest req;
    req.processes = in;
    req.stdin = std::move(stdin);

    auto ctrl = start_process(req, cb);
    wait_process(ctrl);
    free_process(ctrl);
}


bool is_exist_normal(ProcessStopCode sc)
{
    return WIFEXITED(sc);
}

bool is_signaled(ProcessStopCode sc)
{
    return WIFSIGNALED(sc);
}

bool is_stopped(ProcessStopCode sc)
{
    return WIFSTOPPED(sc);
}
bool is_continued(ProcessStopCode sc)
{
    return WIFCONTINUED(sc);
}

bool get_normal_exit_code(ProcessStopCode sc, int &ec)
{
    auto normal = is_exist_normal(sc);
    if (normal) {
        ec = WEXITSTATUS(sc);
    }

    return normal;
}

bool get_signal(ProcessStopCode sc, int &sig)
{
    auto signaled = is_signaled(sc);
    if (signaled) {
        sig = WTERMSIG(sc);
    }
    return signaled;
}

}  // namespace process
}  // namespace ostool