#pragma once

#include "Context.h"

#include <memory>

namespace Duktype
{
    class ExecutionContext
    {
    public:
        ExecutionContext(const std::shared_ptr<Context>& ctx, const Nan::FunctionCallbackInfo<v8::Value> &info);
        ~ExecutionContext();

    private:
        std::shared_ptr<Context> _ctx;
    };
}

