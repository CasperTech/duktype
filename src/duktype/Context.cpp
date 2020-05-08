#include "Context.h"
#include "ContextStack.h"
#include "ExecutionContext.h"
#include "Utils.h"
#include "DebugStack.h"

#include <iostream>
#include <vector>

namespace Duktype
{
    Context::Context()
    {
        _ctx = duk_create_heap_default();
        _d = new DebugStack("MainContext", _ctx);
    }

    Context::~Context()
    {
        delete _d;
        duk_destroy_heap(_ctx);
    }


    duk_context * Context::getContext()
    {
        return _ctx;
    }

    void Context::eval(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        ContextStack c(_ctx, {});
        DebugStack d("eval", _ctx);

        ExecutionContext e(info);

        if(info.Length() > 0 && info[0]->IsString())
        {
            std::string cppStr = *v8::String::Utf8Value(info[0]);
            try
            {
                if (duk_peval_string(_ctx, cppStr.c_str()) != 0)
                {
                    duk_size_t sLength;
                    const char * err = duk_safe_to_lstring(_ctx, -1, &sLength);
                    duk_pop(_ctx);
                    return Nan::ThrowError(Nan::New<v8::String>(err, static_cast<int>(sLength)).ToLocalChecked());
                }
            }
            catch(const std::runtime_error& e)
            {
                std::string what = std::string(e.what());
                return Nan::ThrowError(Nan::New<v8::String>(what.c_str(), static_cast<int>(what.size())).ToLocalChecked());
            }
            auto value = Utils::dukToV8(-1, _ctx);
            duk_pop(_ctx);
            info.GetReturnValue().Set(value);
        }
        else
        {
            Nan::ThrowError("Please provide a string to evaluate");
        }
    }

}