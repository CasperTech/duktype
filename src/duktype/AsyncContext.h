#pragma once

#include "AsyncJob.h"
#include "Context.h"

#include <functional>
#include <uv.h>

#include <mutex>
#include <condition_variable>
#include <thread>

namespace Duktype
{
    class AsyncJobScheduler;
    class AsyncNodeScheduler;
    class AsyncContext: public Context
    {
        public:
            explicit AsyncContext();
            ~AsyncContext();

            v8::Local<v8::Function> getCallbackConstructor() override;
            void eval(const Nan::FunctionCallbackInfo<v8::Value> &info) override;
            void runCallback(const Nan::FunctionCallbackInfo<v8::Value> &info, const std::string& cbName) override;
            void resolvePromise(const std::string& promiseHandle, const Nan::FunctionCallbackInfo<v8::Value>& info) override;
            void catchPromise(const std::string& promiseHandle, const Nan::FunctionCallbackInfo<v8::Value>& info) override;
            void setProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info) override;
            void getProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info) override;
            void deleteProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info) override;
            void getObject(const std::string& parentHandle, const Nan::FunctionCallbackInfo<v8::Value> &info) override;
            void createObjectAsync(const std::string& parentObjectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info);
            void callMethod(const std::string& objectHandle, const std::string& methodName, const Nan::FunctionCallbackInfo<v8::Value>& info) override;
            void runGC(const Nan::FunctionCallbackInfo<v8::Value> &info) override;
            void derefObject(const std::string& handle) override;
            void derefCallback(const std::string& handle) override;
            void handleDukThen(const std::string& promiseID, duk_context* c) override;
            void handleDukCatch(const std::string& promiseID, duk_context* c) override;

        private:
            static int handleFunctionCall(duk_context* ctx);
            int functionCall(duk_context* ctx) override;

            std::shared_ptr<AsyncJobScheduler> _dukScheduler;
            std::shared_ptr<AsyncJobScheduler> _jobScheduler;
            std::shared_ptr<AsyncNodeScheduler> _nodeScheduler;
    };
}
