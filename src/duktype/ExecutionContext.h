#pragma once

#include "../Nan.h"

#include <stack>
#include <functional>

namespace Duktype
{
    typedef std::function<v8::Local<v8::Value>(Nan::Callback *, std::vector<v8::Local<v8::Value>> args)> ExecContext;

    class ExecutionContext
    {
        public:
            explicit ExecutionContext(const Nan::FunctionCallbackInfo<v8::Value> &info);
            ~ExecutionContext();
            static ExecContext getContext();

        private:
            static std::stack<ExecContext> _contexts;
    };
}