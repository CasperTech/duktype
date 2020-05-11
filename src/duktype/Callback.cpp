#include "Callback.h"
#include "DebugStack.h"
#include "ExecutionContext.h"
#include "Utils.h"

#include <iostream>

namespace Duktype
{
    Callback::~Callback()
    {
        if (!_cbHandle.empty())
        {
            duk_context * ctx = _ctx->getContext();
            duk_push_global_stash(ctx);
            duk_del_prop_string(ctx, -1, _cbHandle.c_str());
            _cbHandle.clear();
            duk_pop(ctx);
        }
    }

    void Callback::call(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        duk_context * ctx = _ctx->getContext();
        DebugStack d("Call", ctx);
        ExecutionContext e(_ctx, info);
        duk_push_global_stash(ctx);
        duk_get_prop_string(ctx, -1, _cbHandle.c_str());
        if (!duk_is_undefined(ctx, -1) && duk_is_callable(ctx, -1))
        {
            try
            {
                if (duk_pcall(ctx, 0) != DUK_EXEC_SUCCESS)
                {
                    duk_size_t sLength;
                    const char* err = duk_safe_to_lstring(ctx, -1, &sLength);
                    duk_pop_2(ctx);
                    return Nan::ThrowError(Nan::New<v8::String>(err, static_cast<int>(sLength)).ToLocalChecked());
                }
                auto value = Utils::dukToV8(-1, _ctx);
                info.GetReturnValue().Set(value);
            }
            catch (const std::runtime_error &e)
            {
                std::string what = std::string(e.what());
                return Nan::ThrowError(Nan::New<v8::String>(what.c_str(), static_cast<int>(what.size())).ToLocalChecked());
            }
        }
        duk_pop_2(ctx);
    }

    void Callback::setHandle(const std::string& cbHandle)
    {
        _cbHandle = cbHandle;
    }

    void Callback::setContextObj(std::shared_ptr<Duktype::Context>& ptr)
    {
        _ctx = ptr;
    }
}