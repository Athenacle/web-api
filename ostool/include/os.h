#ifndef OSTOOL_USER_H_
#define OSTOOL_USER_H_

#include <string>

namespace ostool
{
namespace os
{
std::string whoami();

std::string find_program(const std::string &name);

bool is_exectuable(const std::string &);

uint32_t get_cpu_concurrency();

}  // namespace os
}  // namespace ostool

#endif
