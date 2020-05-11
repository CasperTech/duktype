#include "Context.h"
#include "ContextStack.h"
#include "ExecutionContext.h"
#include "Utils.h"

#include <iostream>

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


    duk_context* Context::getContext()
    {
        return _ctx;
    }

    void Context::pushExecutionContext(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        _currentExecutionContext.push(const_cast<Nan::FunctionCallbackInfo<v8::Value>*>(&info));
    }

    const Nan::FunctionCallbackInfo<v8::Value> &Context::getCurrentExecutionContext()
    {
        if (_currentExecutionContext.empty())
        {
            throw std::runtime_error("There is no available execution context");
        }
        return *_currentExecutionContext.top();
    }

    void Context::popExecutionContext()
    {
        _currentExecutionContext.pop();
    }

    void Context::eval(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        ContextStack c(_ctx, {});
        DebugStack d("eval", _ctx);

        ExecutionContext e(shared_from_this(), info);

        if (info.Length() > 0 && info[0]->IsString())
        {
            std::string cppStr = *Nan::Utf8String(info[0]);;
            try
            {
                if (duk_peval_string(_ctx, cppStr.c_str()) != 0)
                {
                    duk_size_t sLength;
                    const char* err = duk_safe_to_lstring(_ctx, -1, &sLength);
                    duk_pop(_ctx);
                    return Nan::ThrowError(Nan::New<v8::String>(err, static_cast<int>(sLength)).ToLocalChecked());
                }
            }
            catch (const std::runtime_error &e)
            {
                std::string what = std::string(e.what());
                return Nan::ThrowError(Nan::New<v8::String>(what.c_str(), static_cast<int>(what.size())).ToLocalChecked());
            }
            auto ref = shared_from_this();
            auto value = Utils::dukToV8(-1, ref);
            duk_pop(_ctx);
            info.GetReturnValue().Set(value);
        }
        else
        {
            Nan::ThrowError("Please provide a string to evaluate");
        }
    }
}