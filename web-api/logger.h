#include <cppmhd/logger.h>

#include <spdlog/spdlog.h>

using LL = cppmhd::log::LogLevel;

class WALogger : public cppmhd::log::AbstractLogger
{
    using LL = cppmhd::log::LogLevel;

  public:
    WALogger();

    virtual void output(LL lv, const char* func, const char* file, int line, const char* message) override;

    virtual void fflush() override;
};

#define ERROR spdlog::error
#define INFO spdlog::info
#define WARN spdlog::warn
#define TRACE spdlog::trace