#ifndef OSTOOL_CRYPTO_H_
#define OSTOOL_CRYPTO_H_

#include "cxx_utils.h"

namespace ostool
{
namespace crypto
{

std::string base64_encode(const void*, size_t);

std::string base64_encode(const cxxutils::Buffer&);


}  // namespace crypto
}  // namespace ostool


#endif