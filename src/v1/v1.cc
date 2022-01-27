#include "v1.h"

#include "apk.h"

#include <cppmhd/router.h>

using namespace cppmhd;

namespace v1
{

void v1Router(cppmhd::App& app, const ConfObject& obj)
{
    RouterBuilder apk;
    apk::setRouter(apk, obj);

    RouterBuilder v1;
    v1.add("/apk", apk);

    app.builder().add("/v1", v1);
}


}  // namespace v1