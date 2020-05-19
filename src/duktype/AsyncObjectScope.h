#pragma once

#include "ObjectScope.h"

namespace Duktype
{
    class AsyncObjectScope: public ObjectScope
    {
        public:
            void createObjectAsync(const Nan::FunctionCallbackInfo<v8::Value> &info);
    };
}
