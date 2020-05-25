#include "Context.h"
#include "DebugStack.h"
#include "ObjectHandle.h"
#include <objectscope.h>
#include "CallbackWeakRef.h"
#include <duktape/DukGlobalStash.h>
#include <uuid/uuid.h>
#include <callback.h>
#include <iostream>
#include <mutex>


namespace Duktype
{
    Context::Context()
        : _duktape(std::make_shared<Duktape::DuktapeContext>())
        , _resourceManager(std::unique_ptr<ResourceManager>(new ResourceManager()))
    {

    }

    Context::~Context()
    {

    }

    void Context::runCallback(const Nan::FunctionCallbackInfo<v8::Value> &info, const std::string& cbName)
    {
        DebugStack d("runCallback", _duktape->getContext());
        /* NodeJS */
        auto* args = new Nan::Persistent<v8::Value>[info.Length()];
        for (int argNum = 0; argNum < info.Length(); argNum++)
        {
            args[argNum].Reset(info[argNum]);
        }

        Duktape::DukGlobalStash global(_duktape);
        Duktape::DukValue::newString(_duktape, cbName);
        Duktape::DukValue funcName(_duktape);

        for(int x = 0; x < static_cast<int>(info.Length()); x++)
        {
            Duktape::DukValue::fromV8(shared_from_this(), info[x]);
        }

        _duktape->runCallback(shared_from_this(), info.Length());

        info.GetReturnValue().Set(funcName.toV8(shared_from_this()));
    }

    std::shared_ptr<::Duktape::DuktapeContext> Context::getDuktape()
    {
        return _duktape;
    }

    void Context::registerPromise(const std::string& promiseHandle, const std::shared_ptr<Promise>& promise)
    {
        _resourceManager->addPromiseFromNode(promiseHandle, promise);
    }

    void Context::registerDukPromise(const std::string& promiseHandle, Nan::Persistent<v8::Promise::Resolver>* resolver)
    {
        _resourceManager->addPromiseFromDuk(promiseHandle, resolver);
    }

    void Context::handleDukThen(const std::string& promiseID, duk_context* c)
    {
        DebugStack d("handleDukThen", c);
        /* DukTape */
        auto resolver = _resourceManager->resolveDukPromise(promiseID);
        if (resolver == nullptr)
        {
            throw std::runtime_error("Promise not registered");
        }

        int nargs = duk_get_top(c);
        v8::Local<v8::Value> ret = Nan::Undefined();
        if (nargs > 0)
        {
            Duktape::DukValue v(_duktape);
            v.resolved();
            ret = v.toV8(shared_from_this());
        }
        auto res = Nan::New(*resolver);
        res->Resolve(v8::Isolate::GetCurrent()->GetCurrentContext(), ret);
        delete resolver;
    }

    void Context::handleDukCatch(const std::string& promiseID, duk_context* c)
    {
        DebugStack d("handleDukCatch", c);
        /* DukTape */
        auto resolver = _resourceManager->resolveDukPromise(promiseID);
        if (resolver == nullptr)
        {
            throw std::runtime_error("Promise not registered");
        }

        int nargs = duk_get_top(c);
        v8::Local<v8::Value> ret = Nan::Undefined();
        if (nargs > 0)
        {
            Duktape::DukValue v(_duktape);
            v.resolved();
            ret = v.toV8(shared_from_this());
        }
        auto res = Nan::New(*resolver);
        res->Reject(v8::Isolate::GetCurrent()->GetCurrentContext(), ret);
        delete resolver;
    }

    int Context::handleFunctionFinalised(duk_context* ctx)
    {
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "__context");
        void* ptr = duk_get_pointer(ctx, -1);
        duk_pop_2(ctx);
        return static_cast<Context*>(ptr)->functionFinalised(ctx);
    }

    int Context::functionFinalised(duk_context *ctx)
    {
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "__callbackID");
        std::string callbackID = duk_to_string(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_string(ctx, -1, "__objectHandle");
        std::string objectHandle = duk_to_string(ctx, -1);
        duk_pop_2(ctx);

        _resourceManager->nodeCallbackDereferencedInDuk(objectHandle, callbackID);
        return 0;
    }

    int Context::objectFinalised(duk_context *ctx)
    {
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "__objectHandle");
        std::string objectName = duk_to_string(ctx, -1);
        duk_pop_2(ctx);
        _resourceManager->dukObjectDestroyed(objectName);
        return 0;
    }

    int Context::handleObjectFinalised(duk_context *ctx)
    {
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "__context");
        void* ptr = duk_get_pointer(ctx, -1);
        duk_pop_2(ctx);
        return static_cast<Context*>(ptr)->objectFinalised(ctx);
    }

    void Context::derefObject(const std::string& handle)
    {
        if (!handle.empty() && handle != "GLOBAL")
        {
            duk_push_global_stash(_duktape->getContext());
            duk_del_prop_string(_duktape->getContext(), -1, handle.c_str());
            duk_pop(_duktape->getContext());
            _resourceManager->removeNodeObjectHandle(handle);
        }
    }

    void Context::derefCallback(const std::string &handle)
    {
        DebugStack d("derefCallback", _duktape->getContext());
        duk_push_global_stash(_duktape->getContext());
        duk_del_prop_string(_duktape->getContext(), -1, handle.c_str());
        duk_pop(_duktape->getContext());

        _resourceManager->removeCallbackFromDuk(handle);
    }

    void Context::addCallback(const std::string& description, const std::string& handle)
    {
        _resourceManager->addCallbackFromDuk(description, handle);
    }

    void Context::cleanRefs(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        _resourceManager->cleanAll();
    }

    void Context::runGC(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        DebugStack d("runGC", _duktape->getContext());
        duk_gc(_duktape->getContext(), DUK_GC_COMPACT);
        duk_gc(_duktape->getContext(), DUK_GC_COMPACT);
    }

    int Context::handleFunctionCall(duk_context* ctx)
    {
        DebugStack d("handleFunctionCall", ctx, 1);
        try
        {
            duk_push_current_function(ctx);
            duk_get_prop_string(ctx, -1, "__context");
            void* ptr = duk_get_pointer(ctx, -1);
            duk_pop(ctx);
            duk_get_prop_string(ctx, -1, "__callbackID");
            std::string callbackName = duk_to_string(ctx, -1);
            duk_pop_2(ctx);
            if (ptr != nullptr)
            {
                return static_cast<Context*>(ptr)->functionCall(ctx);
            }
            else
            {
                duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", "HandleFunctionCall: ptr is null");
            }
        }
        catch(const std::exception& err)
        {
            duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", err.what());
            duk_throw(ctx);
        }
        return 1;
    }

    void Context::callMethod(const std::string& objectHandle, const std::string& methodName, const Nan::FunctionCallbackInfo<v8::Value>& info)
    {
        DebugStack d("callMethod", _duktape->getContext());
        /* NodeJS */
        duk_context* ctx = _duktape->getContext();

        ObjectHandle handle(_duktape, objectHandle);

        int objIndex = duk_get_top_index(ctx);

        std::string funcNameStr = *Nan::Utf8String(info[0]);
        duk_push_string(ctx, funcNameStr.c_str());
        Duktape::DukValue value(_duktape);
        int args = 0;
        for (int argNum = 1; argNum < info.Length(); argNum++)
        {
            Duktape::DukValue::fromV8(shared_from_this(), info[argNum]);
            args++;
        }
        try
        {
            if (duk_pcall_prop(ctx, objIndex, args) != DUK_EXEC_SUCCESS)
            {
                duk_size_t sLength;
                const char* err = duk_safe_to_lstring(ctx, -1, &sLength);
                return Nan::ThrowError(Nan::New<v8::String>(err, static_cast<int>(sLength)).ToLocalChecked());
            }
            auto returnValue = value.toV8(shared_from_this());
            info.GetReturnValue().Set(returnValue);
        }
        catch (const std::runtime_error &e)
        {
            std::string what = std::string(e.what());
            return Nan::ThrowError(Nan::New<v8::String>(what.c_str(), static_cast<int>(what.size())).ToLocalChecked());
        }
    }

    int Context::functionCall(duk_context* ctx)
    {
        DebugStack d("functionCall", _duktape->getContext(), 1);
        Nan::HandleScope scope;

        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "__callbackID");
        std::string callbackName = duk_to_string(ctx, -1);
        duk_pop_2(ctx);

        try
        {
            Nan::Persistent<v8::Function>* cb = _resourceManager->getNodeCallback(callbackName);
            if (cb == nullptr)
            {
                duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", std::string("Failed to find callback " + callbackName).c_str());
                duk_throw(ctx);
                return 0;
            }
            std::vector<v8::Local<v8::Value>> params;
            duk_idx_t i, nargs;
            nargs = duk_get_top(ctx);
            Nan::EscapableHandleScope scope;
            std::string uuid = UUID::v4();
            try
            {
                for (i = 0; i < nargs; i++)
                {
                    int argIndex = (0 - nargs) + (i + 1);
                    Duktape::DukValue v(getDuktape(), argIndex);
                    v.resolved();
                    try
                    {
                        v8::Local<v8::Value> prim = v.toV8(shared_from_this());
                        params.emplace_back(prim);
                    }
                    catch(std::exception e)
                    {
                        std::cout << "Exception: " << e.what() << std::endl;
                    }
                }

                Nan::AsyncResource resource("duktype:callback");
                Nan::TryCatch tryCatch;
                v8::Local<v8::Function> fn = Nan::New(*cb);

                v8::Handle<v8::Object> global = v8::Isolate::GetCurrent()->GetCurrentContext()->Global();
                void* parameters = nullptr;
                if(params.size() > 0)
                {
                    parameters = &params[0];
                }
                v8::MaybeLocal<v8::Value> retMaybe = fn->Call(v8::Isolate::GetCurrent()->GetCurrentContext(), global, static_cast<int>(params.size()), static_cast<v8::Local<v8::Value>*>(parameters));
                if (tryCatch.HasCaught())
                {
                    v8::Local<v8::Message> msg = tryCatch.Message();
                    std::string errStr = *Nan::Utf8String(msg->Get());
                    throw std::runtime_error(errStr);
                }
                v8::Local<v8::Value> ret = retMaybe.ToLocalChecked();
                Duktape::DukValue::fromV8(shared_from_this(), ret);
                return 1;
            }
            catch (const std::runtime_error& e)
            {
                duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", e.what());
                duk_throw(ctx);
            }
        }
        catch (const std::exception& e)
        {
            duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", e.what());
            duk_throw(ctx);
        }
    }

    void Context::setProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() < 2 || !info[0]->IsString())
        {
            throw std::runtime_error("Invalid or wrong number of arguments");
        }
        std::string propNameStr = *Nan::Utf8String(info[0]);
        DebugStack d("setProperty", _duktape->getContext());
        duk_context* ctx = _duktape->getContext();

        ObjectHandle handle(_duktape, objectHandle);
        auto propertyValue = info[1];
        if (propertyValue->IsFunction())
        {
            std::string callbackID = "_dcb:" + UUID::v4();
            try
            {
                duk_push_c_function(ctx, &Context::handleFunctionCall, DUK_VARARGS);
                duk_push_c_function(ctx, &Context::handleFunctionFinalised, 1 /*nargs*/);
                duk_push_string(ctx, callbackID.c_str());
                duk_put_prop_string(ctx, -2, "__callbackID");
                duk_push_string(ctx, objectHandle.c_str());
                duk_put_prop_string(ctx, -2, "__objectHandle");
                duk_push_pointer(ctx, static_cast<void*>(this));
                duk_put_prop_string(ctx, -2, "__context");
                duk_set_finalizer(ctx, -2);

                // Handle of the containing object
                duk_push_string(ctx, objectHandle.c_str());
                duk_put_prop_string(ctx, -2, "__objectHandle");
                // Callback name as a property for function object.
                duk_push_string(ctx, callbackID.c_str());
                duk_put_prop_string(ctx, -2, "__callbackID");
                // Scope pointer
                duk_push_pointer(ctx, static_cast<void*>(this));
                duk_put_prop_string(ctx, -2, "__context");

                // Give the function a name
                duk_put_prop_string(ctx, -2, propNameStr.c_str());
                {
                    Nan::HandleScope scope;
                    v8::Local<v8::Function> callbackFunction = Nan::To<v8::Function>(propertyValue).ToLocalChecked();

                    auto* persistedFunc = new Nan::Persistent<v8::Function>();
                    persistedFunc->Reset(callbackFunction);
                    _resourceManager->addCallbackFromNode(std::string("Property " + propNameStr + " on object " + objectHandle), objectHandle, callbackID, persistedFunc);
                }
            }
            catch (const std::exception& e)
            {
                Nan::ThrowError(e.what());
            }
            return;
        }

        // Some other value
        Duktape::DukValue::fromV8(shared_from_this(), propertyValue);
        duk_put_prop_string(ctx, -2, propNameStr.c_str());
    }


    void Context::getProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        DebugStack d("getProperty", _duktape->getContext(), 1);
        if (info.Length() < 1 || !info[0]->IsString())
        {
            throw std::runtime_error("Invalid or wrong number of arguments");
        }

        duk_context* ctx = _duktape->getContext();
        ObjectHandle handle(_duktape, objectHandle);
        int objIndex = duk_get_top_index(ctx);

        std::string propNameStr = *Nan::Utf8String(info[0]);
        duk_push_string(ctx, propNameStr.c_str());
        try
        {
            duk_get_prop(ctx, objIndex);
            Duktape::DukValue v(_duktape);
            v.resolved();
            auto value = v.toV8(shared_from_this());
            info.GetReturnValue().Set(value);
        }
        catch (const std::runtime_error &e)
        {
            std::string what = std::string(e.what());
            return Nan::ThrowError(Nan::New<v8::String>(what.c_str(), static_cast<int>(what.size())).ToLocalChecked());
        }

    }

    void Context::deleteProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        DebugStack d("deleteProperty", _duktape->getContext());
        if (info.Length() < 1 || !info[0]->IsString())
        {
            throw std::runtime_error("Invalid or wrong number of arguments");
        }

        duk_context* ctx = _duktape->getContext();
        ObjectHandle handle(_duktape, objectHandle);

        if (info.Length() < 1 || !info[0]->IsString())
        {
            throw std::runtime_error("Invalid or wrong number of arguments");
        }

        int objIndex = duk_get_top(ctx) - 1;
        std::string propNameStr = *Nan::Utf8String(info[0]);
        duk_push_string(ctx, propNameStr.c_str());
        try
        {
            duk_del_prop(ctx, objIndex);
        }
        catch (const std::runtime_error &e)
        {
            duk_pop(ctx);
            std::string what = std::string(e.what());
            return Nan::ThrowError(Nan::New<v8::String>(what.c_str(), static_cast<int>(what.size())).ToLocalChecked());
        }
    }

    std::string Context::createObject(const std::string& parentHandle, const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        DebugStack d("createObject", _duktape->getContext());
        if (info.Length() < 1 || !info[0]->IsString())
        {
            throw std::runtime_error("Invalid or wrong number of arguments");
        }

        std::string objectHandle = UUID::v4();

        duk_context* ctx = _duktape->getContext();
        ObjectHandle handle(_duktape, parentHandle);

        std::string objNameStr = *Nan::Utf8String(info[0]);
        Duktape::DukValue::newObject(_duktape);
        duk_push_c_function(ctx, &Context::handleObjectFinalised, 1 /*nargs*/);
            duk_push_string(ctx, objectHandle.c_str());
            duk_put_prop_string(ctx, -2, "__objectHandle");
            duk_push_pointer(ctx, static_cast<void*>(this));
            duk_put_prop_string(ctx, -2, "__context");
            duk_set_finalizer(ctx, -2);

        duk_push_global_stash(ctx);
        duk_dup(ctx, -2);
        duk_put_prop_string(ctx, -2, objectHandle.c_str());
        duk_pop(ctx);
        duk_put_prop_string(ctx, -2, objNameStr.c_str());

        _resourceManager->addDukObjectForNode("(sync createObject) " + objNameStr + " on parent " + parentHandle, objectHandle);
        return objectHandle;
    }

    void Context::getObject(const std::string& parentHandle, const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        DebugStack d("getObject", _duktape->getContext());
        std::string objectHandle = UUID::v4();
        std::string objNameStr;

        if (info.Length() > 0 && info[0]->IsString())
        {
            objNameStr = *Nan::Utf8String(info[0]);
        }

        duk_context* ctx = _duktape->getContext();
        ObjectHandle handle(_duktape, parentHandle);
        if (!objNameStr.empty())
        {
            duk_get_prop_string(ctx, -1, objNameStr.c_str());
        }
        if (!duk_is_object(ctx, -1))
        {
            throw std::runtime_error("No such object in this scope");
        }

        duk_push_global_stash(ctx);
        duk_dup(ctx, -2);
        duk_put_prop_string(ctx, -2, objectHandle.c_str());
        duk_pop(ctx);
        if (!objNameStr.empty())
        {
            duk_pop(ctx);
        }

        _resourceManager->addDukObjectForNode("(sync getObject) " + objNameStr + " on parent " + parentHandle, objectHandle);

        v8::Local<v8::Function> cons = Nan::New<v8::Function>(::ObjectScope::constructor);
        v8::Local<v8::Object> instance = Nan::NewInstance(cons).ToLocalChecked();
        auto* scp = Nan::ObjectWrap::Unwrap<::ObjectScope>(instance);
        scp->setContext(shared_from_this());
        scp->getObjectScope()->setHandle(objectHandle);
        info.GetReturnValue().Set(instance);
    }

    void Context::getObjectReferenceCount(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        info.GetReturnValue().Set(Nan::New(static_cast<double>(_resourceManager->getObjectRefCount())));
    }

    void Context::resolvePromise(const std::string& promiseHandle, const Nan::FunctionCallbackInfo<v8::Value>& info)
    {
        DebugStack d("resolvePromise", _duktape->getContext());
        auto promise = _resourceManager->resolveNodePromise(promiseHandle);
        if (!promise)
        {
            throw std::runtime_error("Attempting to retrieve promise which does not exist");
        }
        try
        {
            duk_context* ctx = _duktape->getContext();
            duk_push_global_stash(ctx);
            int stashIndex = duk_get_top_index(ctx);
            {
                duk_push_string(ctx, std::string(promiseHandle + "_resolve").c_str());
                Duktape::DukValue val(_duktape);
                int args = 0;
                for (int argNum = 0; argNum < info.Length(); argNum++)
                {
                    Duktape::DukValue::fromV8(shared_from_this(), info[argNum]);
                    args++;
                }
                if (duk_pcall_prop(ctx, stashIndex, args) != DUK_EXEC_SUCCESS)
                {
                    std::string errStr = val.getString();
                    throw std::runtime_error(errStr);
                }
                else
                {
                    info.GetReturnValue().Set(val.toV8(shared_from_this()));
                }
            }
            duk_pop(ctx);
        }
        catch(const std::exception& err)
        {
            std::cout << "Caught error: " << err.what() << std::endl;
            Nan::ThrowError(err.what());
        }
    }

    void Context::catchPromise(const std::string& promiseHandle, const Nan::FunctionCallbackInfo<v8::Value>& info)
    {
        DebugStack d("catchPromise", _duktape->getContext());
        auto promise = _resourceManager->resolveNodePromise(promiseHandle);
        if (!promise)
        {
            throw std::runtime_error("Attempting to retrieve promise which does not exist");
        }
        try
        {
            duk_context* ctx = _duktape->getContext();
            duk_push_global_stash(ctx);
            int stashIndex = duk_get_top_index(ctx);
            {
                duk_push_string(ctx, std::string(promiseHandle + "_reject").c_str());
                Duktape::DukValue val(_duktape);
                int args = 0;
                for (int argNum = 0; argNum < info.Length(); argNum++)
                {
                    Duktape::DukValue::fromV8(shared_from_this(), info[argNum]);
                    args++;
                }
                if (duk_pcall_prop(ctx, stashIndex, args) != DUK_EXEC_SUCCESS)
                {
                    std::string errStr = val.getString();
                    throw std::runtime_error(errStr);
                }
                else
                {
                    info.GetReturnValue().Set(val.toV8(shared_from_this()));
                }
            }
            duk_pop(ctx);
        }
        catch(const std::exception& err)
        {
            Nan::ThrowError(err.what());
        }
    }

    void Context::eval(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        DebugStack d("eval", _duktape->getContext());
        if (info.Length() > 0 && info[0]->IsString())
        {
            std::string evalStr = *Nan::Utf8String(info[0]);
            evalInternal(evalStr, [&](Duktape::DukValue& val){
                auto returnValue = val.toV8(shared_from_this());
                info.GetReturnValue().Set(returnValue);
            });
        }
        else
        {
            Nan::ThrowError("Please provide a string to evaluate");
        }
    }

    void Context::evalInternal(const std::string &code, const std::function<void(Duktape::DukValue& val)>& returnFunc)
    {
        _duktape->eval(code, returnFunc);
    }

    v8::Local<v8::Function> Context::getCallbackConstructor()
    {
        return Nan::New<v8::Function>(::DukCallback::constructor);
    }
}