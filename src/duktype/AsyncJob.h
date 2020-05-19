#pragma once

#include <duktape/DukValue.h>

#include <uv.h>
#include <initnan.h>

#include <functional>
#include <mutex>
#include <condition_variable>

namespace Duktype
{
    class AsyncJobScheduler;
    class AsyncNodeScheduler;
    class AsyncContext;
    class AsyncJob
    {
        public:

            AsyncJob(const std::string& jobID, const std::shared_ptr<AsyncJobScheduler>& _jobScheduler, const std::shared_ptr<AsyncJobScheduler>& _dukScheduler, const std::shared_ptr<AsyncNodeScheduler>& _nodeScheduler, const std::shared_ptr<AsyncContext>& ctx);
            ~AsyncJob();
            void setArgs(const Nan::FunctionCallbackInfo<v8::Value>& info, int startArg);
            void setWork(const std::function<void(AsyncJob*, uint32_t)>& work);
            void setPreArgsWork(const std::function<void(AsyncJob*)>& work);
            void setAfterReturnValueWork(const std::function<void(AsyncJob*)>& work);
            void setReturnValueWork(const std::function<v8::Local<v8::Value>(AsyncJob*)>& work);
            void setPops(uint32_t pops);
            void runWithPromise(const Nan::FunctionCallbackInfo<v8::Value>& info, bool expectReturnValue);
            void run();
            void marshalToNodeThread();
            bool hasError();
            std::string errorMessage();
            void setReturnValue(Duktape::DukValue& val);
            std::weak_ptr<AsyncContext> _ctx;

        private:
            void workFunc();
            void dukFunc();
            void nodeArgsFunc();
            void nodeMarshalFunc();
            void nodeReturnValueFunc();

            bool _error = false;
            std::string _jobID;
            std::string _errorMessage;
            std::mutex _dukWorkMutex;
            std::mutex _nodeWorkMutex;
            std::condition_variable _dukWait;
            std::condition_variable _nodeWait;
            bool _dukFinished = false;
            bool _nodeFinished = false;
            Nan::Persistent<v8::Value>* _returnValue = nullptr;
            Nan::Persistent<v8::Value>* _args = nullptr;
            Nan::Persistent<v8::Promise::Resolver>* _resolver = nullptr;
            uint32_t _argCount = 0;
            uint32_t _pops = 0;
            bool _expectReturnValue = false;
            std::function<void(AsyncJob*)> _preArgsWork;
            std::function<void(AsyncJob*)> _afterReturnValueWork;
            std::function<v8::Local<v8::Value>(AsyncJob*)> _returnValueWork;
            std::function<void(AsyncJob*, uint32_t argCount)> _work;

            std::weak_ptr<AsyncJobScheduler> _dukScheduler;
            std::weak_ptr<AsyncNodeScheduler> _nodeScheduler;
            std::weak_ptr<AsyncJobScheduler> _jobScheduler;
    };
}