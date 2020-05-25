#pragma once

#include <initnan.h>
#include <duktape.h>
#include <mutex>

namespace Duktype
{
    class Context;
    class Promise
    {
        public:
            static void handleThen(const Nan::FunctionCallbackInfo<v8::Value>&);

            static void handleCatch(const Nan::FunctionCallbackInfo<v8::Value>&);

            static int dukPromiseFinalised(duk_context* ctx);

            static int dukThenFinalised(duk_context* ctx);

            static int dukCatchFinalised(duk_context* ctx);

            static int storePromiseResolvers(duk_context* ctx);

            static int handleDukThen(duk_context* ctx);

            static int handleDukCatch(duk_context* ctx);

            std::string promiseID;
            Nan::Persistent<v8::Promise> promise;
    };
}
