#pragma once

#include "DukObject.h"

#include <duktape.h>
#include <initnan.h>

#include <string>
#include <memory>
#include <functional>

namespace Duktape
{
    class DuktapeContext;
    class DukGlobalStash: public DukObject
    {
        public:
            DukGlobalStash(const std::shared_ptr<DuktapeContext>& ctx);
            ~DukGlobalStash();
    };
}