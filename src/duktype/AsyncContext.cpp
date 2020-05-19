#include "AsyncContext.h"
#include "AsyncJobScheduler.h"
#include "AsyncNodeScheduler.h"
#include "CallbackWeakRef.h"
#include "ObjectHandle.h"
#include "DebugStack.h"
#include <asynccallback.h>
#include <sole/sole.hpp>
#include <asyncobjectscope.h>
#include <iostream>

namespace Duktype
{
    AsyncContext::AsyncContext()
        : Context()
    {
        _dukScheduler = std::make_shared<AsyncJobScheduler>();
        _jobScheduler = std::make_shared<AsyncJobScheduler>();
        _nodeScheduler = std::make_shared<AsyncNodeScheduler>();
    }

    AsyncContext::~AsyncContext()
    {
        std::unique_lock<std::mutex> lk(_callbacksLock);
        for(const auto& cb: _callbacks)
        {
            delete cb.second;
        }
        _callbacks.clear();
    }

    v8::Local<v8::Function> AsyncContext::getCallbackConstructor()
    {
        return Nan::New<v8::Function>(::DukAsyncCallback::constructor);
    }

    void AsyncContext::eval(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() < 1 || !info[0]->IsString())
        {
            Nan::ThrowError("Please provide a string to evaluate");
        }

        std::string code = *Nan::Utf8String(info[0]);

        AsyncJob* job = new AsyncJob("Eval", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        job->setWork([&, code](AsyncJob* job, uint32_t args)
        {
            DebugStack d("EvalAsync", _duktape->getContext());
            evalInternal(code, [&](Duktape::DukValue& val)
            {
                job->setReturnValue(val);
            });
        });
        job->runWithPromise(info, false);
    }

    void AsyncContext::runCallback(const Nan::FunctionCallbackInfo<v8::Value> &info, const std::string& cbName)
    {
        AsyncJob* job = new AsyncJob("RunCallback", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        job->setPreArgsWork([&, cbName](AsyncJob* job)
        {
            duk_push_global_stash(_duktape->getContext());
            job->setPops(1);
            Duktape::DukValue::newString(_duktape, cbName);
        });
        job->setArgs(info, 0);
        job->setWork([&, cbName](AsyncJob* job, uint32_t argCount)
        {
            _duktape->runCallback(shared_from_this(), argCount);
        });
        job->runWithPromise(info, true);
    }

    void AsyncContext::resolvePromise(const std::string& promiseHandle, const Nan::FunctionCallbackInfo<v8::Value>& info)
    {
        auto it = _promises.find(promiseHandle);
        if (it == _promises.end())
        {
            throw std::runtime_error("Attempting to retrieve promise which does not exist");
        }
        auto promise = (*it).second;
        _promises.erase(it);


        AsyncJob* job = new AsyncJob("ResolvePromise", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        job->setPreArgsWork([&, promiseHandle](AsyncJob* job)
                            {
                                duk_push_global_stash(_duktape->getContext());
                                job->setPops(1);
                                Duktape::DukValue::newString(_duktape, promiseHandle + "_resolve");
                            });
        job->setArgs(info, 0);
        job->setWork([&, promiseHandle](AsyncJob* job, uint32_t argCount)
                     {
                         if (duk_pcall_prop(_duktape->getContext(), duk_get_top_index(_duktape->getContext()) - (argCount + 1), argCount) != DUK_EXEC_SUCCESS)
                         {
                             std::string errStr = std::string(duk_safe_to_string(_duktape->getContext(), -1));
                             duk_pop(_duktape->getContext());
                             throw std::runtime_error(errStr);
                         }
                     });
        job->runWithPromise(info, true);
    }

    void AsyncContext::catchPromise(const std::string& promiseHandle, const Nan::FunctionCallbackInfo<v8::Value>& info)
    {
        AsyncJob* job = new AsyncJob("CatchPromise", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        job->setPreArgsWork([&, promiseHandle](AsyncJob* job)
                            {
                                duk_push_global_stash(_duktape->getContext());
                                job->setPops(1);
                                Duktape::DukValue::newString(_duktape, promiseHandle + "_reject");
                            });
        job->setArgs(info, 0);
        job->setWork([&, promiseHandle](AsyncJob* job, uint32_t argCount)
                     {
                         if (duk_pcall_prop(_duktape->getContext(), duk_get_top_index(_duktape->getContext()) - (argCount + 1), argCount) != DUK_EXEC_SUCCESS)
                         {
                             std::string errStr = std::string(duk_safe_to_string(_duktape->getContext(), -1));
                             duk_pop(_duktape->getContext());
                             throw std::runtime_error(errStr);
                         }
                     });
        job->runWithPromise(info, true);
    }

    void AsyncContext::handleDukThen(const std::string& promiseID, duk_context* ctx)
    {
        AsyncJob* job = new AsyncJob("HandleDukThen", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));

        job->setWork([&, ctx, promiseID](AsyncJob* job, uint32_t argCount)
        {
            Nan::HandleScope h;
            DebugStack d("handleDukThen", ctx);
            /* DukTape */
            auto it = _dukPromises.find(promiseID);
            if (it == _dukPromises.end())
            {
                throw std::runtime_error("Promise not registered");
            }
            auto resolver = (*it).second;
            _dukPromises.erase(it);
            int nargs = duk_get_top(ctx);
            v8::Local<v8::Value> ret = Nan::Undefined();
            if (nargs > 0)
            {
                Duktape::DukValue v(_duktape);
                v.resolved();
                ret = v.toV8(shared_from_this());
            }
            auto res = Nan::New(*resolver);
            res->Resolve(v8::Isolate::GetCurrent()->GetCurrentContext(), ret);
        });
        job->marshalToNodeThread();
        if (job->hasError())
        {
            duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", job->errorMessage().c_str());
            delete job;
            duk_throw(ctx);
        }
        delete job;
    }

    void AsyncContext::handleDukCatch(const std::string& promiseID, duk_context* ctx)
    {
        AsyncJob* job = new AsyncJob("HandleDukCatch", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));

        job->setWork([&, ctx, promiseID](AsyncJob* job, uint32_t argCount)
        {
            Nan::HandleScope h;
            /* DukTape */
            auto it = _dukPromises.find(promiseID);
            if (it == _dukPromises.end())
            {
                throw std::runtime_error("Promise not registered");
            }
            auto resolver = (*it).second;
            _dukPromises.erase(it);
            int nargs = duk_get_top(ctx);
            v8::Local<v8::Value> ret = Nan::Undefined();
            if (nargs > 0)
            {
                Duktape::DukValue v(_duktape);
                v.resolved();
                ret = v.toV8(shared_from_this());
            }
            auto res = Nan::New(*resolver);
            res->Reject(v8::Isolate::GetCurrent()->GetCurrentContext(), ret);
        });
        job->marshalToNodeThread();
        if (job->hasError())
        {
            duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", job->errorMessage().c_str());
            delete job;
            duk_throw(ctx);
        }
        delete job;
    }

    void AsyncContext::derefObject(const std::string& handle)
    {
        AsyncJob* job = new AsyncJob("DeRefObject", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        job->setWork([&, handle](AsyncJob* job, uint32_t argCount)
                     {
                         duk_push_global_stash(_duktape->getContext());
                         duk_del_prop_string(_duktape->getContext(), -1, handle.c_str());
                         duk_pop(_duktape->getContext());
                         _objects.erase(handle);
                     });
        job->run();
    }

    void AsyncContext::derefCallback(const std::string &handle)
    {
        AsyncJob* job = new AsyncJob("DeRefCallback", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        job->setWork([&, handle](AsyncJob* job, uint32_t argCount)
                     {
                         DebugStack d("derefCallback", _duktape->getContext());
                         duk_push_global_stash(_duktape->getContext());
                         duk_del_prop_string(_duktape->getContext(), -1, handle.c_str());
                         duk_pop(_duktape->getContext());
                         _dukCallbacks.erase(handle);
                     });
        job->run();

    }

    void AsyncContext::setProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() < 2 || !info[0]->IsString())
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
            return;
        }
        std::string propNameStr = *Nan::Utf8String(info[0]);

        if (info[1]->IsFunction())
        {
            std::string callbackID = "_dcb:" + sole::uuid4().str();

            v8::Local<v8::Function> callbackFunction = Nan::To<v8::Function>(info[1]).ToLocalChecked();
            auto* persistedFunc = new Nan::Persistent<v8::Function>(callbackFunction);
            CallbackWeakRef* ref = new CallbackWeakRef();
            ref->callbackID = callbackID;
            ref->ctx = shared_from_this();
            persistedFunc->SetWeak(ref, &Context::handleCallbackDestroyed, Nan::WeakCallbackType::kParameter);
            {
                std::unique_lock<std::mutex> lk(_callbacksLock);
                _callbacks[callbackID] = persistedFunc;
            }

            AsyncJob* job = new AsyncJob("SetPropertyFunction", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
            job->setWork([&, callbackID, objectHandle, propNameStr](AsyncJob* job, uint32_t argCount)
            {
                DebugStack d("SetPropertyFunc", _duktape->getContext(), 1);
                duk_context* ctx = _duktape->getContext();
                if (objectHandle.empty())
                {
                    duk_push_global_object(ctx);
                    job->setPops(1);
                }
                else
                {
                    duk_push_global_stash(ctx);
                    duk_get_prop_string(ctx, -1, objectHandle.c_str());
                    job->setPops(2);
                }
                int objIndex = duk_get_top_index(ctx);

                duk_push_c_function(ctx, &AsyncContext::handleFunctionCall, DUK_VARARGS);

                {
                    duk_push_c_function(ctx, &AsyncContext::handleFunctionFinalised, 1 /*nargs*/);
                    {
                        duk_push_string(ctx, callbackID.c_str());
                        duk_put_prop_string(ctx, -2, "__callbackID");
                    }
                    {
                        duk_push_pointer(ctx, static_cast<void*>(this));
                        duk_put_prop_string(ctx, -2, "__context");
                    }
                    duk_set_finalizer(ctx, -2);
                }
                // Callback name as a property for function object.
                duk_push_string(ctx, callbackID.c_str());
                duk_put_prop_string(ctx, -2, "__callbackID");
                // Scope pointer
                duk_push_pointer(ctx, static_cast<void*>(this));
                duk_put_prop_string(ctx, -2, "__context");
                // Give the function a name
                duk_put_prop_string(ctx, objIndex, propNameStr.c_str());
            });
            job->runWithPromise(info, false);
            return;
        }

        AsyncJob* job = new AsyncJob("SetPropertyValue", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        job->setArgs(info, 1);
        job->setPreArgsWork([&, objectHandle](AsyncJob* job)
        {
            DebugStack d("setPropertyPre", _duktape->getContext(), 1);
            duk_context* ctx = _duktape->getContext();
            if (objectHandle.empty())
            {
                duk_push_global_object(ctx);
                job->setPops(1);
            }
            else
            {
                duk_push_global_stash(ctx);
                duk_get_prop_string(ctx, -1, objectHandle.c_str());
                job->setPops(2);
            }
        });
        job->setWork([&, objectHandle, propNameStr](AsyncJob* job, uint32_t argCount)
        {
            DebugStack d("setPropertyWork", _duktape->getContext(), -1);
            duk_context* ctx = _duktape->getContext();
            duk_put_prop_string(ctx, -2, propNameStr.c_str());
        });
        job->runWithPromise(info, false);
    }

    int AsyncContext::handleFunctionCall(duk_context *ctx)
    {
        try
        {
            DebugStack s("handleFunctionCall", ctx, 1);
            duk_push_current_function(ctx);
            duk_get_prop_string(ctx, -1, "__context");
            void* ptr = duk_get_pointer(ctx, -1);
            duk_pop(ctx);
            duk_get_prop_string(ctx, -1, "__callbackID");
            std::string callbackName = duk_to_string(ctx, -1);
            duk_pop_2(ctx);
            if (ptr != nullptr)
            {
                return static_cast<AsyncContext*>(ptr)->functionCall(ctx);
            }
            else
            {
                duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", "HandleFunctionCall: ptr is null");
            }
        }
        catch(const std::exception& err)
        {
            std::cout << "caught exception " << std::endl;
            duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", err.what());
            duk_throw(ctx);
        }
        return 1;
    }

    int AsyncContext::functionCall(duk_context *ctx)
    {
        AsyncJob* job = new AsyncJob("FunctionCall", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));

        int result = 0;
        job->setWork([&, ctx](AsyncJob* job, uint32_t argCount)
        {
            DebugStack d("functionCallLambda", ctx, 1);
            Nan::HandleScope scope;
            duk_push_current_function(ctx);
            duk_get_prop_string(ctx, -1, "__callbackID");
            std::string callbackName = duk_to_string(ctx, -1);
            duk_pop_2(ctx);
            Nan::Persistent<v8::Function>* cb;
            {
                std::unique_lock<std::mutex> lk(_callbacksLock);
                auto it = _callbacks.find(callbackName);
                if (it == _callbacks.end())
                {
                    return;
                }
                cb = (*it).second;
            }
            std::vector<v8::Local<v8::Value>> params;
            duk_idx_t i, nargs;
            nargs = duk_get_top(ctx);
            Nan::EscapableHandleScope handleScope;
            std::string uuid = sole::uuid4().str();
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
                catch (const std::exception& e)
                {
                    std::cout << "Exception: " << e.what() << std::endl;
                }
            }

            Nan::AsyncResource resource("duktype:callback");
            Nan::TryCatch tryCatch;


            v8::Local<v8::Function> fn = Nan::New(*cb);
            v8::Handle<v8::Object> global = v8::Isolate::GetCurrent()->GetCurrentContext()->Global();
            v8::MaybeLocal<v8::Value> retMaybe = fn->Call(v8::Isolate::GetCurrent()->GetCurrentContext(), global, static_cast<int>(params.size()), &params[0]);

            if (tryCatch.HasCaught())
            {
                v8::Local<v8::Message> msg = tryCatch.Message();
                std::string errStr = *Nan::Utf8String(msg->Get());
                d.addExpected(-1);
                throw std::runtime_error(errStr);
            }
            v8::Local<v8::Value> ret = retMaybe.ToLocalChecked();
            Duktape::DukValue::fromV8(shared_from_this(), ret);
            result = 1;
        });
        job->marshalToNodeThread();
        if (job->hasError())
        {
            duk_push_error_object(ctx, DUK_ERR_ERROR, "%s", job->errorMessage().c_str());
            delete job;
            duk_throw(ctx);
        }
        delete job;
        return result;
    }

    void AsyncContext::getProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() < 1 || !info[0]->IsString())
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
            return;
        }
        std::string propNameStr = *Nan::Utf8String(info[0]);
        AsyncJob* job = new AsyncJob("GetProperty", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        job->setWork([&, objectHandle, propNameStr](AsyncJob* job, uint32_t argCount)
        {
            duk_context* ctx = _duktape->getContext();
            if (objectHandle.empty())
            {
                duk_push_global_object(ctx);
                job->setPops(1);
            }
            else
            {
                duk_push_global_stash(ctx);
                duk_get_prop_string(ctx, -1, objectHandle.c_str());
                job->setPops(2);
            }
            int objIndex = duk_get_top_index(ctx);
            duk_push_string(ctx, propNameStr.c_str());
            duk_get_prop(ctx, objIndex);
        });
        job->runWithPromise(info, true);
    }

    void AsyncContext::deleteProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() < 1 || !info[0]->IsString())
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
            return;
        }
        std::string propNameStr = *Nan::Utf8String(info[0]);
        AsyncJob* job = new AsyncJob("DeleteProperty", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        job->setWork([&, objectHandle, propNameStr](AsyncJob* job, uint32_t argCount)
        {
            duk_context* ctx = _duktape->getContext();
            if (objectHandle.empty())
            {
                duk_push_global_object(ctx);
                job->setPops(1);
            }
            else
            {
                duk_push_global_stash(ctx);
                duk_get_prop_string(ctx, -1, objectHandle.c_str());
                job->setPops(2);
            }
            int objIndex = duk_get_top_index(ctx);
            duk_push_string(ctx, propNameStr.c_str());
            duk_del_prop(ctx, objIndex);
        });
        job->runWithPromise(info, false);
    }

    void AsyncContext::createObjectAsync(const std::string& parentObjectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() < 1 || !info[0]->IsString())
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
            return;
        }

        std::string objectHandle = sole::uuid4().str();

        AsyncJob* job = new AsyncJob("CreateObjectAsync", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        _objects.insert(objectHandle);
        std::string objNameStr = *Nan::Utf8String(info[0]);
        job->setWork([&, parentObjectHandle, objectHandle, objNameStr](AsyncJob* job, uint32_t argCount)
        {
            duk_context* ctx = _duktape->getContext();
            ObjectHandle handle(_duktape, parentObjectHandle);


            Duktape::DukValue::newObject(_duktape);
            duk_push_c_function(ctx, &Context::handleObjectFinalised, 1 /*nargs*/);
            duk_push_string(ctx, objectHandle.c_str());
            duk_put_prop_string(ctx, -2, "__promiseHandle");
            duk_push_pointer(ctx, static_cast<void*>(this));
            duk_put_prop_string(ctx, -2, "__context");
            duk_set_finalizer(ctx, -2);

            duk_push_global_stash(ctx);
            duk_dup(ctx, -2);
            duk_put_prop_string(ctx, -2, objectHandle.c_str());
            duk_pop(ctx);
            duk_put_prop_string(ctx, -2, objNameStr.c_str());
        });
        job->setReturnValueWork([&, objectHandle](AsyncJob* job)->v8::Local<v8::Value>
        {
            Nan::HandleScope scope;
            v8::Local<v8::Function> cons = Nan::New<v8::Function>(::AsyncObjectScope::constructor);
            v8::Local<v8::Context> context = v8::Isolate::GetCurrent()->GetCurrentContext();
            auto ins1 = cons->NewInstance(context);
            auto instance = ins1.ToLocalChecked();
            auto* scp = Nan::ObjectWrap::Unwrap<::AsyncObjectScope>(instance);
            scp->setContext(std::static_pointer_cast<AsyncContext>(shared_from_this()));
            scp->getObjectScope()->setHandle(objectHandle);
            return instance;
        });
        job->runWithPromise(info, true);
    }

    std::string AsyncContext::getObject(const std::string& parentObjectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        return "";
    }

    void AsyncContext::callMethod(const std::string& objectHandle, const std::string& methodName, const Nan::FunctionCallbackInfo<v8::Value>& info)
    {
        AsyncJob* job = new AsyncJob("CallMethod", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        job->setPreArgsWork([&, objectHandle, methodName](AsyncJob* job)
        {
            duk_context * ctx = _duktape->getContext();
            if (objectHandle.empty())
            {
                duk_push_global_object(ctx);
                job->setPops(1);
            }
            else
            {
                duk_push_global_stash(ctx);
                duk_get_prop_string(ctx, -1, objectHandle.c_str());
                job->setPops(2);
            }

            Duktape::DukValue::newString(_duktape, methodName);
        });
        job->setArgs(info, 1);
        job->setWork([&](AsyncJob* job, uint32_t argCount)
        {
            duk_context * ctx = _duktape->getContext();
            if (duk_pcall_prop(ctx, duk_get_top_index(ctx) - (argCount + 1), argCount) != DUK_EXEC_SUCCESS)
            {
                duk_size_t sLength;
                const char* err = duk_safe_to_lstring(ctx, -1, &sLength);
                throw std::runtime_error(std::string(err, sLength));
            }
        });
        job->runWithPromise(info, true);
    }

    void AsyncContext::runGC(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        AsyncJob* job = new AsyncJob("RunGC", _jobScheduler, _dukScheduler, _nodeScheduler, std::static_pointer_cast<AsyncContext>(shared_from_this()));
        job->setWork([&](AsyncJob* job, uint32_t argCount)
                     {
                         duk_gc(_duktape->getContext(), DUK_GC_COMPACT);
                         duk_gc(_duktape->getContext(), DUK_GC_COMPACT);
                     });
        job->runWithPromise(info, false);
    }
}