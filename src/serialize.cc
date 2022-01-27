#include "conf.h"
#include "logger.h"
#include "utils.h"

#include <nlohmann/json.hpp>

using namespace cppmhd;
using njson = nlohmann::json;


void from_json(const njson& j, ConfApp& p)
{
    CONTAIN_GET(j, address, p);
    CONTAIN_GET(j, port, p);
    CONTAIN_GET(j, threads, p);
}


HttpResponsePtr make_response(const UniversalReturnObject& json, const SerializableObject* ser)
{
    auto resp = std::make_shared<HttpResponse>();
    njson ret;
    json.json(ret);
    if (ser != nullptr) {
        njson data;
        ser->json(data);
        ret["result"] = data;
    }

    auto s = ret.dump();
    resp->body(s);

    return resp;
}

bool UniversalReturnObject::json(njson& result) const
{
    result["status"] = sc_;
    result["id"] = value_;

    if (msg_.length() > 0) {
        result["message"] = msg_;
    }

    return true;
}

bool json_from_buffer(njson& j, const void* data, size_t size)
{
    auto p = reinterpret_cast<const char*>(data);
    try {
        j = njson::parse(p, p + size);
        return true;
    } catch (std::exception& ex) {
        ERROR("parse json failed: {}", ex.what());
    }
    return false;
}

bool ConfObject::from_json(const void* data, size_t size)
{
    njson conf;
    if (json_from_buffer(conf, data, size)) {
        // ConfApp
        if (conf.contains("listen")) {
            if (auto l = conf.at("listen"); l.is_object()) {
                this->lis = l.get<ConfApp>();
            }
        }

        if (conf.contains("apk")) {
            if (auto a = conf.at("apk"); a.is_object()) {
                this->apk = a.get<apk::ApkConfig>();
            }
        }

        return true;
    }
    return false;
}
