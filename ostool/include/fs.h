#ifndef OSTOOL_FS_H_
#define OSTOOL_FS_H_

#include <filesystem>
#include <string>

namespace ostool
{
namespace fs
{
using namespace std::filesystem;

bool is_exectuable(const std::string &);

bool is_exectuable(const path &);

}  // namespace fs
}  // namespace ostool

#endif