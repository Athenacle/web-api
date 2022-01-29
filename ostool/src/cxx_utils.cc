#include "cxx_utils.h"

using namespace std;
using namespace ostool;

namespace ostool
{
namespace cxxutils
{

void split(const string& in, vector<string>& out, const string& sep)
{
    std::string tmp(in);
    do {
        auto pos = tmp.find_first_of(sep);
        if (pos == std::string::npos) {
            if (tmp.length() > 0) {
                out.emplace_back(tmp);
            }
            break;
        } else if (pos != 0) {
            std::string sub(tmp, 0, pos);
            tmp = tmp.substr(pos + sep.length());
            out.emplace_back(sub);
        } else {
            tmp = tmp.substr(sep.length());
        }

    } while (true);
}

void LineIO::feed(const char* in, size_t size)
{
    buffer_.append(in, size);
}

string LineIO::eatAll()
{
    return string(std::move(buffer_));
}

string LineIO::eat()
{
    string ret;
    auto n = buffer_.find_first_of('\n');
    if (n != buffer_.npos) {
        ret = buffer_.substr(0, n);
        buffer_ = buffer_.substr(n + 1);
    }
    return ret;
}

}  // namespace cxxutils
}  // namespace ostool