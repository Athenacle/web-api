#include "config.h"
#include "v1/apk.h"

#include <string>

using namespace v1;

struct ConfApp {
    std::string address;
    uint32_t port;
    uint32_t threads;

    ConfApp() : address("0.0.0.0"), port(8888), threads(2) {}
};

struct ConfObject {
    ConfApp lis;
    apk::ApkConfig apk;

    bool from_json(const void*, size_t);
};
