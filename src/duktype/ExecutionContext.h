#pragma once

#include "../Nan.h"

#include <stack>
#include <functional>

namespace Duktype
{
    typedef std::function<Nan::Persistent<v8::Value>(Nan::Callback *, const std::vector<Nan::CopyablePersistentTraits<v8::Value>::CopyablePersistent>& args)> ExecContext;

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