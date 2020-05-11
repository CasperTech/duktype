#include "ExecutionContext.h"

namespace Duktype
{
    ExecutionContext::ExecutionContext(const std::shared_ptr<Context>& ctx, const Nan::FunctionCallbackInfo<v8::Value> &info)
        : _ctx(ctx)
    {
        _ctx->pushExecutionContext(info);
    }

    ExecutionContext::~ExecutionContext()
    {
        _ctx->popExecutionContext();
    }
}
