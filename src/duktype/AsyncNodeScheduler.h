#pragma once

#include <functional>
#include <initnan.h>

#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>

namespace Duktype
{
    struct NodeJob
    {
        std::function<void()> runFunc;
        std::function<void()> afterWork;
        uv_async_t* async;
        std::string label;
        std::mutex jobMutex;
        std::condition_variable jobWait;
        bool jobDone = false;
        bool waitForCompletion = true;
    };

    struct ResolvePromiseTask
    {
        Nan::Persistent<v8::Promise::Resolver>* resolver;
        bool reject;
        std::string errorMessage;
        Nan::Persistent<v8::Value>* returnValue;
    };

    class AsyncNodeScheduler
    {
        public:
            AsyncNodeScheduler();
            ~AsyncNodeScheduler();
            void addWork(const std::string& label, const std::function<void()>& job, const std::function<void()>& afterWork, bool priority = false, bool waitForComplete = true);
            void resolvePromise(Nan::Persistent<v8::Promise::Resolver>* resolver, bool reject, std::string& errorMessage, Nan::Persistent<v8::Value>* _returnValue);

        private:
            static void handleRunInContext(uv_async_s* re);

            void run();
            std::thread _schedulerThread;

            std::mutex _queueMutex;
            std::mutex _threadStartMutex;
            std::deque<std::shared_ptr<NodeJob>> _workQueue;
            std::deque<std::shared_ptr<NodeJob>> _priorityQueue;
            std::condition_variable _workNotify;
            std::condition_variable _threadStartWait;
            bool _exiting = false;
            static std::mutex _uvMutex;
            bool _threadRunning;
            uv_async_t* _asyncWork;
    };
}