#include "logger.h"

#include "config.h"

#include <spdlog/spdlog.h>

using namespace spdlog;
namespace
{
using sl = spdlog::level::level_enum;

level::level_enum dispatch(LL l)
{
    switch (l) {
        case LL::kDebug:
            return sl::debug;
        case LL::kInfo:
            return sl::info;
        case LL::kTrace:
            return sl::trace;
        case LL::kWarning:
            return sl::warn;
        case LL::kError:
            return sl::err;
        case LL::kFatal:
        default:
            return sl::critical;
    }
}

}  // namespace

void WALogger::output(
    LL lv, MAYBE_UNUSED const char* func, MAYBE_UNUSED const char* file, MAYBE_UNUSED int line, const char* message)
{
#ifndef NDEBUG
    spdlog::log(dispatch(lv), "({}:{}): {}", file, line, message);
#else
    spdlog::log(dispatch(lv), "{}", message);
#endif
}

void WALogger::fflush() {}

WALogger::WALogger() {}