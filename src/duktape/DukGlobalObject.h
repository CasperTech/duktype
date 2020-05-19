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
    class DukGlobalObject: public DukObject
    {
        public:
            DukGlobalObject(const std::shared_ptr<DuktapeContext>& ctx);
            ~DukGlobalObject();
    };
}