#include "conf.h"
#include "config.h"
#include "logger.h"
#include "utils.h"
#include "v1/v1.h"

#include <cppmhd/app.h>
#include <cppmhd/logger.h>

#include <spdlog/spdlog.h>

using namespace cppmhd;


namespace
{
bool read_config(ConfObject& config, int argc, const char* argv[])
{
#define DEFAULT_CONFIG SRC_ROOT "/config.json"
    size_t size;
    const char* fn;
    bool ret = false;
    if (argc == 1) {
        fn = DEFAULT_CONFIG;
    } else {
        fn = argv[1];
    }

    if (auto ptr = readfile(fn, size); ptr == nullptr) {
        spdlog::error("open configure file {} failed: {}", fn, strerror(errno));
    } else {
        if (config.from_json(ptr, size)) {
            ret = true;
            TRACE("parse configure file {} success.", fn);
        } else {
            TRACE("parse configure file {} failed", fn);
        }
        closefile(ptr, size);
    }
    return ret;
}

}  // namespace

int main(int argc, const char* argv[])
{
    cppmhd::log::setLogger<WALogger>();
    ConfObject conf;

    if (!read_config(conf, argc, argv)) {
        exit(1);
    }

    spdlog::set_level(spdlog::level::trace);

    setup_openssl();

    App app(conf.lis.address, conf.lis.port);
    app.threadCount(conf.lis.threads);

    v1::v1Router(app, conf);

    app.start();

    return 0;
}