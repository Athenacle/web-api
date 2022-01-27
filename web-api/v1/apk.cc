#include "apk.h"

#include "conf.h"
#include "logger.h"

#include <filesystem>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
using namespace cppmhd;

using namespace v1::apk;

namespace
{
class ApkManager
{
    ApkConfig ac;
    RSA *pkey;

    bool check_private_key();

    bool check_path();

  public:
    ApkManager(const ApkConfig &c) : ac(c), pkey(nullptr) {}
    ~ApkManager();

    bool start();


    const auto &config()
    {
        return ac;
    }
};

ApkManager::~ApkManager()
{
    if (pkey) {
    }
}

int pass_cb(char *buf, int size, int, void *u)
{
    auto &conf = reinterpret_cast<ApkManager *>(u)->config();

    if (conf.passphrase.length() == 0) {
        return -1;
    } else {
        auto cp = (size_t)size < conf.passphrase.length() ? (size_t)size : conf.passphrase.length();
        memcpy(buf, conf.passphrase.c_str(), cp);
        return cp;
    }
}

bool ApkManager::start()
{
    return check_private_key() && check_path();
}

bool ApkManager::check_path()
{
    std::filesystem::path root(ac.root_directory);
    std::filesystem::path repo = root.append(ac.repo_name);

    std::error_code ec;
    if (!std::filesystem::exists(repo, ec)) {
        if (!std::filesystem::create_directories(repo, ec)) {
            ERROR("Apk startup mkdir '{}' failed: '{}'", repo.string(), ec.message());
            return false;
        }
    } else {
        if (!std::filesystem::is_directory(repo)) {
            ERROR("Apk startup failed : '{}' exists but not a directory.", ac.root_directory);
            return false;
        }
    }

    auto stat = std::filesystem::status(repo);
    auto perm = stat.permissions();
    if ((perm & std::filesystem::perms::owner_all) != std::filesystem::perms::owner_all) {
        ERROR("Apk startup failed : '{}' permission check failed.", repo.string());
        return false;
    }

    return true;
}

bool ApkManager::check_private_key()
{
    const size_t bs = 512;
    char buffer[bs];

    auto pkbio = BIO_new_file(ac.rsa_pkey_file.c_str(), "r");

    auto ret = true;

    if (pkbio == nullptr) {
        auto ec = ERR_get_error();
        ERR_error_string_n(ec, buffer, bs);
        ERROR("Apk startup failed: load RSA private key file '{}' failed: '{}'", ac.rsa_pkey_file, buffer);
        ret = false;
    } else {
        pkey = PEM_read_bio_RSAPrivateKey(pkbio, &pkey, pass_cb, this);

        if (pkey == nullptr) {
            auto ec = ERR_get_error();
            ERR_error_string_n(ec, buffer, bs);
            ERROR("Apk startup failed: load RSA private key file '{}' failed: '{}'", ac.rsa_pkey_file, buffer);

            ret = false;
        }
    }
    BIO_free(pkbio);
    return ret;
}

class ApkPut : public HttpController
{
    std::shared_ptr<ApkManager> mgr;

  public:
    ApkPut(std::shared_ptr<ApkManager> m) : mgr(m) {}

    virtual void onRequest(HttpRequestPtr, HttpResponsePtr &) override;
};

void ApkPut::onRequest(HttpRequestPtr, HttpResponsePtr &resp)
{
    resp = std::make_shared<HttpResponse>();
    resp->status(k200OK);
}
}  // namespace

namespace v1
{

namespace apk
{

void setRouter(cppmhd::RouterBuilder &rb, const ConfObject &obj)
{
    auto &apk = obj.apk;

    if (!apk.enable) {
        INFO("Apk module not enabled in config file. skip...");
        return;
    } else {
        TRACE("Apk module starting...");
        auto m = std::make_shared<ApkManager>(apk);
        if (m->start()) {
            rb.add<ApkPut>(HttpMethod::PUT, "/{:pkgname}", m);
        }
    }
}
}  // namespace apk

}  // namespace v1