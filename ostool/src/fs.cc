#include "fs.h"

namespace ostool
{
namespace fs
{
bool is_exectuable(const path& file)
{
    if (exists(file)) {
        auto st = status(file);
        auto perm = st.permissions();
        return (perm & perms::owner_exec) == perms::owner_exec;
    }
    return false;
}

bool is_exectuable(const std::string& file)
{
    path p(file);
    return is_exectuable(p);
}
}  // namespace fs
}  // namespace ostool