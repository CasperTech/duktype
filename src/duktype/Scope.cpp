#include "Scope.h"
#include "Utils.h"
#include "CallbackData.h"
#include "ContextStack.h"
#include "ExecutionContext.h"

namespace Duktype
{
    void Scope::setContext(const std::shared_ptr<Duktype::Context> &ctx)
    {
        _ctxObj = ctx;
        _ctx = ctx->getContext();
    }

    void Scope::setStack(const std::vector<std::string> &stack)
    {
        _stack = stack;
    }

    int Scope::handleFunctionCall(duk_context* ctx)
    {
        DebugStack d("handleFunctionCall", ctx, 1); // Expect return value to be pushed
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "scope");
        void* ptr = duk_get_pointer(ctx, -1);
        duk_pop_2(ctx);
        if (ptr != nullptr)
        {
            return static_cast<Scope*>(ptr)->functionCall(ctx);
        }
        else
        {
            std::cout << "Ptr is null" << std::endl;
        }
        return 0;
    }

    int Scope::functionCall(duk_context* ctx)
    {
        DebugStack d("functionCall", ctx, 1); // Expect return value to be pushed
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "__callbackName");
        std::string callbackName = duk_to_string(ctx, -1);
        duk_pop_2(ctx);

        try
        {
            auto it = _callbacks.find(callbackName);
            if (it == _callbacks.end())
            {
                return 0;
            }
            Nan::Callback* cb = (*it).second;
            std::vector<v8::Local<v8::Value>> params;
            const Nan::FunctionCallbackInfo<v8::Value> &info = _ctxObj->getCurrentExecutionContext();
            duk_idx_t i, nargs;
            nargs = duk_get_top(ctx);
            Nan::EscapableHandleScope scope;
            try
            {
                for (i = 0; i < nargs; i++)
                {
                    v8::Local<v8::Value> prim = Utils::dukToV8(i, _ctxObj);
                    params.emplace_back(prim);
                }

                Nan::AsyncResource resource("duktype:callback");
                Nan::TryCatch tryCatch;
                v8::MaybeLocal<v8::Value> retMaybe = cb->Call(static_cast<int>(params.size()), &params[0], &resource);
                if (tryCatch.HasCaught())
                {
                    v8::Local<v8::Message> msg = tryCatch.Message();
                    std::string errStr = *Nan::Utf8String(msg->Get());
                    throw std::runtime_error(errStr);
                }
                v8::Local<v8::Value> ret = retMaybe.ToLocalChecked();
                Utils::v8ToDuk(ret, ctx);
            }
            catch (const std::runtime_error& e)
            {
                duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", e.what());
                return duk_throw(ctx);
            }
            return 1;
        }
        catch (const std::exception& e)
        {
            duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", e.what());
            return duk_throw(ctx);
        }

    }

    std::vector<std::string> Scope::createObject(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        ContextStack c(_ctx, _stack);
        DebugStack d("createObject", _ctx);

        if (info.Length() > 0 && info[0]->IsString())
        {
            std::string objNameStr = *Nan::Utf8String(info[0]);
            duk_push_object(_ctx);
            duk_put_prop_string(_ctx, -2, objNameStr.c_str());
            std::vector<std::string> stack = _stack;
            stack.emplace_back(objNameStr);

            return stack;
        }
        else
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
        }
        return {};
    }

    void Scope::callMethod(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() < 1 || !info[0]->IsString())
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
            return;
        }
        ContextStack c(_ctx, _stack);
        DebugStack d("callMethod", _ctx);
        ExecutionContext e(_ctxObj, info);

        int objIndex = duk_get_top(_ctx) - 1;

        std::string funcNameStr = *Nan::Utf8String(info[0]);
        duk_push_string(_ctx, funcNameStr.c_str());

        int args = 0;
        for (int argNum = 1; argNum < info.Length(); argNum++)
        {
            if (Utils::v8ToDuk(info[argNum], _ctx))
            {
                args++;
            }
        }
        try
        {
            if (duk_pcall_prop(_ctx, objIndex, args) != DUK_EXEC_SUCCESS)
            {
                duk_size_t sLength;
                const char* err = duk_safe_to_lstring(_ctx, -1, &sLength);
                duk_pop(_ctx);
                return Nan::ThrowError(Nan::New<v8::String>(err, static_cast<int>(sLength)).ToLocalChecked());
            }
            auto value = Utils::dukToV8(-1, _ctxObj);
            duk_pop(_ctx);
            info.GetReturnValue().Set(value);
        }
        catch (const std::runtime_error &e)
        {
            std::string what = std::string(e.what());
            return Nan::ThrowError(Nan::New<v8::String>(what.c_str(), static_cast<int>(what.size())).ToLocalChecked());
        }
    }

    void Scope::getProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        ContextStack c(_ctx, _stack);
        DebugStack d("deleteProperty", _ctx);
        if (info.Length() < 1 || !info[0]->IsString())
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
            return;
        }

        int objIndex = duk_get_top(_ctx) - 1;
        std::string propNameStr = *Nan::Utf8String(info[0]);
        duk_push_string(_ctx, propNameStr.c_str());
        try
        {
            duk_get_prop(_ctx, objIndex);
            auto value = Utils::dukToV8(-1, _ctxObj);
            duk_pop(_ctx);
            info.GetReturnValue().Set(value);
        }
        catch (const std::runtime_error &e)
        {
            std::string what = std::string(e.what());
            return Nan::ThrowError(Nan::New<v8::String>(what.c_str(), static_cast<int>(what.size())).ToLocalChecked());
        }
    }

    void Scope::deleteProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        ContextStack c(_ctx, _stack);
        DebugStack d("deleteProperty", _ctx);
        if (info.Length() < 1 || !info[0]->IsString())
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
            return;
        }

        int objIndex = duk_get_top(_ctx) - 1;
        std::string propNameStr = *Nan::Utf8String(info[0]);
        duk_push_string(_ctx, propNameStr.c_str());
        try
        {
            duk_del_prop(_ctx, objIndex);
        }
        catch (const std::runtime_error &e)
        {
            std::string what = std::string(e.what());
            return Nan::ThrowError(Nan::New<v8::String>(what.c_str(), static_cast<int>(what.size())).ToLocalChecked());
        }
    }

    void Scope::setProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() < 2 || !info[0]->IsString())
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
            return;
        }

        ContextStack c(_ctx, _stack);
        DebugStack d("setProperty", _ctx);

        std::string propNameStr = *Nan::Utf8String(info[0]);

        if (info[1]->IsFunction())
        {
            try
            {
                duk_push_c_function(_ctx, &Scope::handleFunctionCall, DUK_VARARGS);

                // Callback name as a property for function object.
                duk_push_string(_ctx, propNameStr.c_str());
                duk_put_prop_string(_ctx, -2, "__callbackName");

                // Scope pointer
                duk_push_pointer(_ctx, static_cast<void*>(this));
                duk_put_prop_string(_ctx, -2, "scope");

                // Give the function a name
                duk_put_prop_string(_ctx, -2, propNameStr.c_str());

                _callbacks[propNameStr] = new Nan::Callback(Nan::To<v8::Function>(info[1]).ToLocalChecked());
            }
            catch (...)
            {
                Nan::ThrowError("Error occurred while adding the function");
            }
            return;
        }

        // Some other value
        Utils::v8ToDuk(info[1], _ctx);
        duk_put_prop_string(_ctx, -2, propNameStr.c_str());
    }
}
