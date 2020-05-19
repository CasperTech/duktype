#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

namespace Duktype
{
    class AsyncJobScheduler
    {
        public:
            AsyncJobScheduler();
            ~AsyncJobScheduler();
            void addWork(const std::function<void()>& job);

        private:
            void run();
            std::thread _schedulerThread;

            std::mutex _queueMutex;
            std::mutex _threadStartMutex;
            std::queue<std::function<void()>> _workQueue;
            std::condition_variable _workNotify;
            std::condition_variable _threadStartWait;
            bool _exiting = false;
            bool _threadRunning;
    };
}