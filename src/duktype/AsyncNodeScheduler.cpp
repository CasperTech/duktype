#include "AsyncNodeScheduler.h"
#include "ResourceManager.h"
#include <uv.h>
#include <initnan.h>

#include <iostream>

namespace Duktype
{
    std::mutex AsyncNodeScheduler::_uvMutex;

    AsyncNodeScheduler::AsyncNodeScheduler()
    {
        std::unique_lock<std::mutex> lk(_threadStartMutex);
        _schedulerThread = std::thread(&AsyncNodeScheduler::run, this);
        _threadStartWait.wait(lk, [&](){ return _threadRunning; });
        _asyncWork = new uv_async_t();
        uv_async_init(uv_default_loop(), _asyncWork, &AsyncNodeScheduler::handleRunInContext);
    }

    AsyncNodeScheduler::~AsyncNodeScheduler()
    {
        {
            std::unique_lock<std::mutex> lk(_queueMutex);
            _exiting = true;
        }
        _workNotify.notify_one();
        _schedulerThread.detach();
        uv_unref(reinterpret_cast<uv_handle_t*>(_asyncWork));
        uv_close(reinterpret_cast<uv_handle_t*>(_asyncWork), [](uv_handle_t* handle){
            // Apparently this causes a crash, already freed?
            //delete handle;
        });
    }

    void AsyncNodeScheduler::addWork(const std::string& label, const std::function<void ()> &job, const std::function<void()>& afterWork, bool priority, bool waitForComplete)
    {
        {
            std::unique_lock<std::mutex> lk(_queueMutex);
            {

                auto nodeJob = std::make_shared<NodeJob>();
                nodeJob->runFunc = job;
                nodeJob->afterWork = afterWork;
                nodeJob->label = label;
                nodeJob->waitForCompletion = waitForComplete;
                if (priority)
                {
                    _priorityQueue.push_front(nodeJob);
                }
                else
                {
                    _workQueue.push_back(nodeJob);
                }
            }
        }
        _workNotify.notify_one();
    }

    void AsyncNodeScheduler::handleRunInContext(uv_async_s *re)
    {
        auto* asyncData = static_cast<NodeJob*>(re->data);
        try
        {
            asyncData->runFunc();
        }
        catch(const std::exception& e)
        {
            std::cout << "WARNING: Caught exception in handleRunInContext" << std::endl;
            std::cout << e.what() << std::endl;
        }

        std::function<void()> afterWorkFunc = asyncData->afterWork;

        {
            std::unique_lock<std::mutex> lk(asyncData->jobMutex);
            asyncData->jobDone = true;
        }

        uv_unref(reinterpret_cast<uv_handle_t*>(asyncData->async));
        uv_async_init(uv_default_loop(), asyncData->async, &AsyncNodeScheduler::handleRunInContext);

        asyncData->jobWait.notify_one();

        if (afterWorkFunc)
        {
            afterWorkFunc();
        }
    }

    void AsyncNodeScheduler::resolvePromise(Nan::Persistent<v8::Promise::Resolver>* resolverPtr, bool reject, std::string& errorMessage, Nan::Persistent<v8::Value>* returnValuePtr)
    {
        if (resolverPtr != nullptr)
        {
            auto* t = new ResolvePromiseTask();
            t->resolver = resolverPtr;
            t->reject = reject;
            t->returnValue = returnValuePtr;
            t->errorMessage = errorMessage;

            addWork("resolvePromise", [t]()
            {
                v8::Isolate* isolate = v8::Isolate::GetCurrent();
                v8::HandleScope scope(isolate);

                if (t->resolver != nullptr && !t->resolver->IsEmpty())
                {
                    auto resolver = Nan::New(*t->resolver);
                    if (t->reject)
                    {
                        // Reject the promise
                        resolver->Reject(isolate->GetCurrentContext(), Nan::Error(t->errorMessage.c_str()));
                    }
                    else
                    {
                        // We need to spawn yet another v8 job so that we don't block our event loop
                        if (t->returnValue)
                        {
                            v8::Local<v8::Value> returnVal = Nan::New(*t->returnValue);
                            resolver->Resolve(isolate->GetCurrentContext(), returnVal);
                            t->returnValue->SetWeak(t->returnValue, &ResourceManager::finaliseDestroyValue, Nan::WeakCallbackType::kParameter);
                        }
                        else
                        {
                            resolver->Resolve(isolate->GetCurrentContext(), Nan::Undefined());
                        }
                    }
                }
                if (t->resolver != nullptr)
                {
                    t->resolver->SetWeak(t->resolver, &ResourceManager::finaliseDestroyPromise, Nan::WeakCallbackType::kParameter);
                }
                delete t;
            }, []()
            {
                v8::Isolate::GetCurrent()->RunMicrotasks();
            }, true, false);
        }
    }

    void AsyncNodeScheduler::run()
    {
        {
            std::unique_lock<std::mutex> lk(_threadStartMutex);
            _threadRunning = true;
        }
        _threadStartWait.notify_one();
        while(!_exiting)
        {
            std::shared_ptr<NodeJob> nextFunc;
            {
                std::unique_lock<std::mutex> lk(_queueMutex);
                if (_workQueue.empty() && _priorityQueue.empty() && !_exiting)
                {
                    _workNotify.wait(lk, [&](){
                        return !_workQueue.empty() || !_priorityQueue.empty() || _exiting;
                    });
                }
                if (_exiting)
                {
                    break;
                }
                if (!_priorityQueue.empty())
                {
                    nextFunc = _priorityQueue.front();
                    _priorityQueue.pop_front();
                }
                else if (!_workQueue.empty())
                {
                    nextFunc = _workQueue.front();
                    _workQueue.pop_front();
                }
            }
            try
            {
                std::unique_lock<std::mutex> lk(nextFunc->jobMutex);
                _asyncWork->data = nextFunc.get();
                nextFunc->async = _asyncWork;
                uv_async_send(_asyncWork);
                nextFunc->jobWait.wait(lk, [&]()
                {
                    return nextFunc->jobDone;
                });
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