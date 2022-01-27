#include "config.h"

#include <atomic>
#include <string>

#include <nlohmann/json_fwd.hpp>

#include <cppmhd/entity.h>


class SerializableObject
{
  public:
    virtual bool json(nlohmann::json& out) const = 0;

    virtual ~SerializableObject();
};

class UniversalReturnObject : public SerializableObject
{
    static std::atomic_uint32_t count;

    uint32_t value_;
    uint32_t sc_;
    std::string msg_;

  public:
    virtual ~UniversalReturnObject();

    virtual bool json(nlohmann::json& out) const override;

    UniversalReturnObject();
};

cppmhd::HttpResponsePtr make_response(const UniversalReturnObject&, const SerializableObject*);

void* readfile(const std::string&, size_t&);
void closefile(void*, size_t);

bool json_from_buffer(nlohmann::json&, const void* data, size_t);


void setup_openssl();