#include "ExecutionContext.h"

namespace Duktype
{
    std::stack<ExecContext> ExecutionContext::_contexts;

    ExecutionContext::ExecutionContext(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        _contexts.push([&](Nan::Callback * cb, const std::vector<Nan::CopyablePersistentTraits<v8::Value>::CopyablePersistent>& params) {
            Nan::HandleScope scope;
            std::vector<v8::Local<v8::Value>> args;
            for (const auto& arg: params)
            {
                args.emplace_back(Nan::New(arg));
            }
            Nan::AsyncResource resource("duktype:callback");
            Nan::TryCatch tryCatch;

            v8::MaybeLocal<v8::Value> ret = cb->Call(static_cast<int>(args.size()), &args[0], &resource);
            if (tryCatch.HasCaught())
            {
                v8::Local<v8::Message> msg = tryCatch.Message();
                std::string errStr = *v8::String::Utf8Value(msg->Get());
                throw std::runtime_error(errStr);
            }
            return ret.ToLocalChecked();
        });
    }

    ExecutionContext::~ExecutionContext()
    {
        _contexts.pop();
    }

    ExecContext ExecutionContext::getContext()
    {
        if (_contexts.empty())
        {
            throw std::runtime_error("No available execution context");
        }
        return _contexts.top();
    }
}