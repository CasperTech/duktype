#include "AsyncJob.h"

#include "AsyncContext.h"
#include "AsyncJobScheduler.h"
#include "AsyncNodeScheduler.h"

#include <uuid/uuid.h>

#include <iostream>

namespace Duktype
{
    std::mutex AsyncJob::activeJobsMutex;
    std::map<std::string, std::string> AsyncJob::activeJobs;

    AsyncJob::AsyncJob(const std::string& jobID, const std::shared_ptr<AsyncJobScheduler>& jobScheduler, const std::shared_ptr<AsyncJobScheduler>& dukScheduler, const std::shared_ptr<AsyncNodeScheduler>& nodeScheduler, const std::shared_ptr<AsyncContext>& ctx)
        : _dukScheduler(dukScheduler)
        , _nodeScheduler(nodeScheduler)
        , _jobScheduler(jobScheduler)
        , _ctx(ctx)
        , _jobID(jobID)
    {
        _jobUUID = UUID::v4();
        {
            std::unique_lock<std::mutex> lk(activeJobsMutex);
            AsyncJob::activeJobs[_jobUUID] = jobID;
        }
    }

    AsyncJob::~AsyncJob()
    {
        {
            std::unique_lock<std::mutex> lk(activeJobsMutex);
            auto it = activeJobs.find(_jobUUID);
            if (it != activeJobs.end())
            {
                activeJobs.erase(it);
            }
        }
    }

    void AsyncJob::setArgs(const Nan::FunctionCallbackInfo<v8::Value>& info, int startArg)
    {
        if (_args != nullptr)
        {
            delete[] _args;
        }
        _args = new Nan::Persistent<v8::Value>[info.Length() - startArg];
        for (int argNum = startArg; argNum < info.Length(); argNum++)
        {
            _args[argNum - startArg].Reset(info[argNum]);
        }
        _argCount = info.Length() - startArg;
    }

    void AsyncJob::setWork(const std::function<void(AsyncJob*, uint32_t)>& work)
    {
        _work = work;
    }

    void AsyncJob::setPreArgsWork(const std::function<void(AsyncJob*)>& work)
    {
        _preArgsWork = work;
    }


    void AsyncJob::setReturnValue(Duktape::DukValue& val)
    {
        // It might be more convenient to set the return value in the work callback,
        // so this is here for that.

        _nodeFinished = false;
        std::unique_lock<std::mutex> lk(_nodeWorkMutex);

        _nodeScheduler.lock()->addWork("SetReturnValue", std::bind(&AsyncJob::nodeReturnValueFunc, this), nullptr);

        // Wait for it to finish
        _nodeWait.wait(lk, [&]()
        {
            return _nodeFinished;
        });
    }

    void AsyncJob::runWithPromise(const Nan::FunctionCallbackInfo<v8::Value>& info, bool expectReturnValue)
    {
        auto resolver = v8::Promise::Resolver::New(info.GetIsolate()->GetCurrentContext()).ToLocalChecked();
        auto promise = resolver->GetPromise();
        info.GetReturnValue().Set(promise);
        _resolver = new Nan::Persistent<v8::Promise::Resolver>(resolver);
        _expectReturnValue = expectReturnValue;

        run();
    }

    void AsyncJob::run()
    {
        _jobScheduler.lock()->addWork(std::bind(&AsyncJob::workFunc, this));
    }

    void AsyncJob::workFunc()
    {
        // We're in a worker thread now, no-man's land. But we've release the node thread back so it can carry on with life.

        // First we need to get into a Duktape context so we can be sure we won't corrupt the stack.
        {
            _dukFinished = false;
            std::unique_lock<std::mutex> lk(_dukWorkMutex);

            _dukScheduler.lock()->addWork(std::bind(&AsyncJob::dukFunc, this));


            // Wait for it to finish
            _dukWait.wait(lk, [&]()
            {
                return _dukFinished;
            });
        }

        // We're done with duktape. Now let's handle our return value and clean up
        if (_resolver)
        {
            // We need to queue yet another job so we don't delay our event loop
            _nodeScheduler.lock()->resolvePromise(_resolver, _error, _errorMessage, _returnValue);
        }
        else if (_returnValue)
        {
            std::cout << "Clearing return value" << std::endl;
            delete _returnValue;
        }
        delete this;
    }

    void AsyncJob::dukFunc()
    {
        {
            std::unique_lock<std::mutex> lk(_dukWorkMutex);
            // Alright, we have a duktape context.

            // We might need to push something onto the stack before the arguments,
            // so make that call

            if (_preArgsWork)
            {
                _preArgsWork(this);
            }

            // Do we need to marshal any arguments onto the duktape stack?
            if (_argCount > 0)
            {
                // Ok, we need to get node back in to do that.
                _nodeFinished = false;
                std::unique_lock<std::mutex> lk(_nodeWorkMutex);

                _nodeScheduler.lock()->addWork("NodeArgsFunc for " + _jobID, std::bind(&AsyncJob::nodeArgsFunc, this), nullptr);

                // Wait for it to finish
                _nodeWait.wait(lk, [&]()
                {
                    return _nodeFinished;
                });
            }

            // Ok, node is free again to do its business.
            // We still have duktape locked in kinky bondage.
            // Let's run the main task
            try
            {
                try
                {
                    _work(this, _argCount);

                    // The work is now done.
                    // Are we expecting a return value that we need to marshal back?

                    if (_expectReturnValue || _returnValueWork)
                    {
                        // Ok, we need to get node back in to do that.
                        _nodeFinished = false;
                        std::unique_lock<std::mutex> lk(_nodeWorkMutex);

                        _nodeScheduler.lock()->addWork("NodeReturnvalueFunc for " + _jobID, std::bind(&AsyncJob::nodeReturnValueFunc, this), nullptr);

                        // Wait for it to finish
                        _nodeWait.wait(lk, [&]()
                        {
                            return _nodeFinished;
                        });
                    }
                }
                catch (const std::exception& error)
                {
                    _error = true;
                    _errorMessage = error.what();
                }
            }
            catch(...)
            {
                // Something else was thrown, we have no way of knowing what this is
                _error = true;
                _errorMessage = "An error occurred";
            }

            {
                while (_pops-- > 0)
                {
                    duk_pop(_ctx.lock()->getDuktape()->getContext());
                }
            }

            // Okay, we have our return value marshalled back in. Let's release the duktape thread so we can resolve the promise without causing chaos.
            _dukFinished = true;
        }
        _dukWait.notify_one();
    }

    bool AsyncJob::hasError()
    {
        return _error;
    }

    std::string AsyncJob::errorMessage()
    {
        return _errorMessage;
    }

    void AsyncJob::marshalToNodeThread()
    {
        // Ok, we need to get node back in to do that.
        _nodeFinished = false;
        std::unique_lock<std::mutex> lk(_nodeWorkMutex);

        _nodeScheduler.lock()->addWork("marshalToNodeThread", std::bind(&AsyncJob::nodeMarshalFunc, this), nullptr);

        // Wait for it to finish
        _nodeWait.wait(lk, [&]()
        {
            return _nodeFinished;
        });
    }

    void AsyncJob::setAfterReturnValueWork(const std::function<void(AsyncJob*)>& work)
    {
        _afterReturnValueWork = work;
    }

    void AsyncJob::setPops(uint32_t pops)
    {
        _pops = pops;
    }

    void AsyncJob::nodeMarshalFunc()
    {
        {
            // We have safely secured both a duktape and a node context at this point.
            // Let's copy our arguments onto the duktape stack.
            std::unique_lock<std::mutex> lk(_nodeWorkMutex);
            try
            {
                try
                {
                    _work(this, _argCount);
                }
                catch (const std::exception& error)
                {
                    _error = true;
                    _errorMessage = error.what();
                }
            }
            catch (...)
            {
                // Something else was thrown, we have no way of knowing what this is
                _error = true;
                _errorMessage = "An error occurred";
            }
            _nodeFinished = true;
        }
        _nodeWait.notify_one();
    }

    void AsyncJob::nodeArgsFunc()
    {
        {
            // We have safely secured both a duktape and a node context at this point.
            // Let's copy our arguments onto the duktape stack.
            std::unique_lock<std::mutex> lk(_nodeWorkMutex);
            Nan::HandleScope scope;
            for (uint32_t x = 0; x < _argCount; x++)
            {
                v8::Local<v8::Value> v = Nan::New(_args[x]);
                Duktape::DukValue::fromV8(_ctx.lock(), v);
            }
            _nodeFinished = true;
        }
        _nodeWait.notify_one();
    }

    void AsyncJob::nodeReturnValueFunc()
    {
        {
            if (_returnValue != nullptr)
            {
                delete _returnValue;
            }

            // Ok, we've grabbed Node again and we can marshal the return value back.
            if (_returnValueWork)
            {
                _returnValue = new Nan::Persistent<v8::Value>();
                auto obj = _returnValueWork(this);
                _returnValue->Reset(obj);
            }
            else
            {
                std::unique_lock<std::mutex> lk(_nodeWorkMutex);
                Duktape::DukValue val(_ctx.lock()->getDuktape());

                // If we're not "expecting" a return value, it means we already have a handle for it
                // so let's not pop it twice
                if (!_expectReturnValue)
                {
                    val.resolved();
                }

                Nan::HandleScope scope;
                v8::Local<v8::Value> returnValue = val.toV8(_ctx.lock());

                // We need to wrap our return value into a Nan::Persistent so we can access it after we've released the duktape context
                _returnValue = new Nan::Persistent<v8::Value>();
                _returnValue->Reset(returnValue);
            }
            _nodeFinished = true;
        }
        _nodeWait.notify_one();
    }

    void AsyncJob::setReturnValueWork(const std::function<v8::Local<v8::Value> (AsyncJob *)> &work)
    {
        _returnValueWork = work;
    }
}