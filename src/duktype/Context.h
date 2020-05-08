#pragma once

#include "DebugStack.h"

#include <duktape.h>
#include "../Nan.h"

namespace Duktype
{
    class Context
    {
        public:
            Context();
            ~Context();
            void eval(const Nan::FunctionCallbackInfo<v8::Value>& info);

            duk_context * getContext();

        private:
            duk_context* _ctx = nullptr;
            DebugStack * _d = nullptr;
    };
}