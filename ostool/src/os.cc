#include "os.h"

#include "cxx_utils.h"
#include "fs.h"

#include <pwd.h>
#include <unistd.h>

#include <condition_variable>
#include <cstdlib>
#include <mutex>
#include <thread>

using namespace std;

namespace ostool
{
namespace os
{

uint32_t get_cpu_concurrency()
{
    return std::thread::hardware_concurrency();
}

string find_program(const string &name)
{
    auto path = getenv("PATH");
    string ret;
    if (path != nullptr) {
        vector<string> paths;
        cxxutils::split(path, paths, ":");
        for (const auto &dir : paths) {
            fs::path current(dir);
            current.append(name);
            if (fs::exists(current) && fs::is_exectuable(current)) {
                ret = current.string();
                break;
            }
        }
    }
    return ret;
}

string whoami()
{
    auto uid = getuid();
    auto pwd = getpwuid(uid);
    if (pwd) {
        return pwd->pw_name;
    }
    return string("");
}


}  // namespace os
}  // namespace ostool