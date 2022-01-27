#include "apk.h"
#include "v1.h"

#include <nlohmann/json.hpp>

namespace v1
{
namespace apk
{


void from_json(const nlohmann::json& j, ApkConfig& ac)
{
    ac.enable = true;

    CONTAIN_GET(j, repo_name, ac);
    CONTAIN_GET(j, enable, ac);
    CONTAIN_GET(j, root_directory, ac);
    CONTAIN_GET(j, rsa_pkey_file, ac);
    CONTAIN_GET(j, passphrase, ac);
}


}  // namespace apk

}  // namespace v1