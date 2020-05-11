#pragma once

#include "DebugStack.h"

#include <duktape.h>
#include <initnan.h>

#include <memory>
#include <stack>

namespace Duktype
{
    class Context: public std::enable_shared_from_this<Context>
    {
        public:
            Context();
            ~Context();
            void eval(const Nan::FunctionCallbackInfo<v8::Value>& info);
            void pushExecutionContext(const Nan::FunctionCallbackInfo<v8::Value>& info);
            void popExecutionContext();
            const Nan::FunctionCallbackInfo<v8::Value>& getCurrentExecutionContext();
            duk_context * getContext();

        private:
            std::stack<Nan::FunctionCallbackInfo<v8::Value>*> _currentExecutionContext;
            duk_context* _ctx = nullptr;
            DebugStack * _d = nullptr;
    };
}