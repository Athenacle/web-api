
#ifndef V1_APK_H_
#define V1_APK_H_
#include "decl.h"

#include <nlohmann/json_fwd.hpp>

#include <cppmhd/router.h>

namespace v1
{

namespace apk
{

struct ApkConfig {
    bool enable;
    std::string repo_name;
    std::string root_directory;
    std::string rsa_pkey_file;
    std::string passphrase;

    ApkConfig() : enable(false) {}
};

void from_json(const nlohmann::json&, ApkConfig&);

void setRouter(cppmhd::RouterBuilder& rb, const ConfObject&);
}  // namespace apk

}  // namespace v1

#endif