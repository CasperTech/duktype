#include "Promise.h"
#include "PromiseData.h"
#include "Context.h"

#include <iostream>

namespace Duktype
{
    void Promise::handleThen(const Nan::FunctionCallbackInfo<v8::Value>& info)
    {
        PromiseData* dat = static_cast<PromiseData*>(info.Data().As<v8::External>()->Value());
        std::shared_ptr<Context> ctxObj = dat->context.lock();
        std::string promiseHandle = dat->promiseHandle;
        delete dat;
        dat = nullptr;
        if (ctxObj)
        {
            ctxObj->resolvePromise(promiseHandle, info);
        }
    }

    void Promise::handleCatch(const Nan::FunctionCallbackInfo<v8::Value>& info)
    {
        PromiseData* dat = static_cast<PromiseData*>(info.Data().As<v8::External>()->Value());
        std::shared_ptr<Context> ctxObj = dat->context.lock();
        std::string promiseHandle = dat->promiseHandle;
        delete dat;
        dat = nullptr;
        if (ctxObj)
        {
            ctxObj->catchPromise(promiseHandle, info);
        }
    }

    int Promise::handleDukThen(duk_context* ctx)
    {
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "__context");
        void* ptr = duk_get_pointer(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_string(ctx, -1, "__promiseHandle");
        std::string promiseID = duk_to_string(ctx, -1);
        duk_pop_2(ctx);

        Duktype::Context* c = static_cast<Duktype::Context*>(ptr);
        c->handleDukThen(promiseID, ctx);
        return 0;
    }

    int Promise::handleDukCatch(duk_context* ctx)
    {
        duk_push_current_function(ctx);
        duk_get_prop_string(ctx, -1, "__context");
        void* ptr = duk_get_pointer(ctx, -1);
        duk_pop(ctx);
        duk_get_prop_string(ctx, -1, "__promiseHandle");
        std::string promiseID = duk_to_string(ctx, -1);
        duk_pop_2(ctx);

        Duktype::Context* c = static_cast<Duktype::Context*>(ptr);
        c->handleDukCatch(promiseID, ctx);
        return 0;
    }

    int Promise::storePromiseResolvers(duk_context *ctx)
    {
        int args = duk_get_top(ctx);
        if (args == 2)
        {
            duk_push_current_function(ctx);
            duk_get_prop_string(ctx, -1, "__promiseHandle");
            std::string promiseHandle = duk_get_string(ctx, -1);
            duk_pop_2(ctx);

            duk_push_global_stash(ctx);
            duk_dup(ctx, 0);
            duk_put_prop_string(ctx, -2, std::string(promiseHandle + "_resolve").c_str());
            duk_dup(ctx, 1);
            duk_put_prop_string(ctx, -2, std::string(promiseHandle + "_reject").c_str());
            duk_pop(ctx);
        }
        return 0;
    }

    int Promise::dukPromiseFinalised(duk_context* ctx)
    {
        return 0;
    }

    int Promise::dukThenFinalised(duk_context* ctx)
    {
        return 0;
    }

    int Promise::dukCatchFinalised(duk_context* ctx)
    {
        return 0;
    }
}