#include "AsyncJobScheduler.h"

#include <iostream>

namespace Duktype
{
    AsyncJobScheduler::AsyncJobScheduler()
    {
        std::unique_lock<std::mutex> lk(_threadStartMutex);
        _schedulerThread = std::thread(&AsyncJobScheduler::run, this);
        _threadStartWait.wait(lk, [&](){ return _threadRunning; });
    }

    AsyncJobScheduler::~AsyncJobScheduler()
    {
        {
            std::unique_lock<std::mutex> lk(_queueMutex);
            _exiting = true;
        }
        _workNotify.notify_one();
        _schedulerThread.join();
    }

    void AsyncJobScheduler::addWork(const std::function<void ()> &job)
    {
        {
            std::unique_lock<std::mutex> lk(_queueMutex);
            _workQueue.push(job);
        }
        _workNotify.notify_one();
    }

    void AsyncJobScheduler::run()
    {
        {
            std::unique_lock<std::mutex> lk(_threadStartMutex);
            _threadRunning = true;
        }
        _threadStartWait.notify_one();
        while(!_exiting)
        {
            std::function<void()> nextFunc;
            {
                std::unique_lock<std::mutex> lk(_queueMutex);
                if (_workQueue.empty() && !_exiting)
                {
                    _workNotify.wait(lk, [&](){
                        return !_workQueue.empty() || _exiting;
                    });
                }
                if (_exiting)
                {
                    break;
                }
                nextFunc = _workQueue.front();
                _workQueue.pop();
            }
            try
            {
                nextFunc();
            }
            catch(...)
            {
                std::cout << "WARNING: Exception caught in scheduler" << std::endl;
            }
        }
        {
            std::unique_lock<std::mutex> lk(_threadStartMutex);
            _threadRunning = false;
        }
    }
}