#ifndef OSTOOL_FORK_H_
#define OSTOOL_FORK_H_

#include "cxx_utils.h"

#include <functional>

namespace ostool
{
namespace process
{
class SingleProcessRequest
{
  public:
    std::string exec;
    std::vector<std::string> arg;
};

class ProcessesRequest
{
  public:
    std::vector<SingleProcessRequest> processes;
    cxxutils::Buffer stdin;
};

using ProcessStopCode = int;

bool is_exist_normal(ProcessStopCode);
bool is_signaled(ProcessStopCode);
bool is_stopped(ProcessStopCode);
bool is_continued(ProcessStopCode);

bool get_normal_exit_code(ProcessStopCode, int &);
bool get_signal(ProcessStopCode, int &);

using ChildStartCallback = std::function<void(pid_t, const SingleProcessRequest &)>;

using ChildWriteStdoutCallback = std::function<void(const uint8_t *, ssize_t, const SingleProcessRequest &)>;

using ChildWriteStderrCallback = ChildWriteStdoutCallback;

using ChildFinishCallback = std::function<void(int, pid_t, const SingleProcessRequest &)>;

using ErrorCallback = std::function<void(int, const char *, const SingleProcessRequest &)>;

class ProcessCtrl;

int wait_process(ProcessCtrl *);
void free_process(ProcessCtrl *);
void kill_process_chain(ProcessCtrl *, int);

class ChildProcessCB
{
  public:
    ChildStartCallback start;
    ChildWriteStdoutCallback stdout;
    ChildWriteStderrCallback stderr;
    ChildFinishCallback finish;
    ErrorCallback error;

    ChildProcessCB()
    {
        auto finish_fn = [](int, pid_t, const SingleProcessRequest &) {};
        auto start_fn = [](pid_t, const SingleProcessRequest &) {};
        start = start_fn;
        finish = finish_fn;
        auto write_fn = [](const uint8_t *, ssize_t, const SingleProcessRequest &) {};
        stdout = stderr = write_fn;
        error = [](int, const char *, const SingleProcessRequest &) {};
    }
};

ProcessCtrl *start_process(const ProcessesRequest &, const ChildProcessCB &);


void start_process_wait(const std::vector<SingleProcessRequest> &in,
                        cxxutils::Buffer &&stdin,
                        cxxutils::Buffer &stdout,
                        ErrorCallback errcb,
                        int &err);

void start_process_wait(const std::vector<SingleProcessRequest> &in,
                        cxxutils::Buffer &stdout,
                        ErrorCallback errcb,
                        int &err);

}  // namespace process
}  // namespace ostool

#endif