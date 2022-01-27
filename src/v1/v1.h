
#ifndef V1_H_
#define V1_H_

#include "decl.h"

#include <cppmhd/app.h>
#include <cppmhd/controller.h>
#include <cppmhd/router.h>

namespace v1
{

void v1Router(cppmhd::App&, const ConfObject&);

}

#endif