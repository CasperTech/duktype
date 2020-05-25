#include "ResourceManager.h"

#include <iostream>

namespace Duktype
{
    ResourceManager::~ResourceManager()
    {
        cleanAll();
    }

    void ResourceManager::addCallbackFromDuk(const std::string& description, const std::string& callbackID)
    {
        std::unique_lock<std::mutex> lk(_resourceLock);
        _dukCallbacks.insert(std::make_pair(callbackID, description));
    }

    void ResourceManager::cleanAll()
    {
        std::unique_lock<std::mutex> lk(_resourceLock);
        for(const auto& cb: _nodeCallbacks)
        {
            Nan::Persistent<v8::Function>* func = cb.second.second;
            func->SetWeak(func, &ResourceManager::finaliseDestroyFunction, Nan::WeakCallbackType::kParameter);
        }
        _nodeCallbacks.clear();
    }

    void ResourceManager::removeCallbackFromDuk(const std::string &callbackID)
    {
        std::unique_lock<std::mutex> lk(_resourceLock);
        _dukCallbacks.erase(callbackID);
    }

    Nan::Persistent<v8::Function> * ResourceManager::getNodeCallback(const std::string &callbackID)
    {
        auto it = _nodeCallbacks.find(callbackID);
        if (it == _nodeCallbacks.end())
        {
            return nullptr;
        }
        return (*it).second.second;
    }

    void ResourceManager::addCallbackFromNode(const std::string& description, const std::string& objectID, const std::string& callbackID, Nan::Persistent<v8::Function>* func)
    {
        std::unique_lock<std::mutex > lk(_resourceLock);

        _nodeCallbacks[callbackID] = std::make_pair(description, func);
        if (_objects.find(objectID) == _objects.end())
        {
            _objects[objectID] = std::make_pair("Added due to callback: " + description, std::vector<std::string>());
        }
        _objects[objectID].second.push_back(callbackID);
    }

    std::shared_ptr<Promise> ResourceManager::resolveNodePromise(const std::string& promiseHandle)
    {
        std::unique_lock<std::mutex > lk(_resourceLock);
        auto it = _promises.find(promiseHandle);
        if (it == _promises.end())
        {
            return {};
        }
        auto promise = (*it).second;
        _promises.erase(it);
        return promise.second;
    }

    void ResourceManager::addDukObjectForNode(const std::string& description, const std::string& objectID)
    {
        std::unique_lock<std::mutex > lk(_resourceLock);
        _objects[objectID] = std::make_pair(description, std::vector<std::string>());
    }

    void ResourceManager::addPromiseFromDuk(const std::string& promiseHandle, Nan::Persistent<v8::Promise::Resolver>* resolver)
    {
        std::unique_lock<std::mutex > lk(_resourceLock);
        _dukPromises[promiseHandle] = std::make_pair("Promise", resolver);
    }

    void ResourceManager::addPromiseFromNode(const std::string& promiseHandle, const std::shared_ptr<Promise>& promise)
    {
        std::unique_lock<std::mutex > lk(_resourceLock);
        _promises[promiseHandle] = std::make_pair("Promise", promise);
    }

    void ResourceManager::finaliseDestroyFunction(const Nan::WeakCallbackInfo<Nan::Persistent<v8::Function>> &data)
    {
        auto* ref = data.GetParameter();
        delete ref;
    }

    void ResourceManager::removeNodeObjectHandle(const std::string &handle)
    {
        auto it = _objects.find(handle);
        if (it == _objects.end())
        {
            return;
        }
        _objects.erase(it);
    }

    void ResourceManager::nodeCallbackDereferencedInDuk(const std::string &objectHandle, const std::string &callbackID)
    {
        std::unique_lock<std::mutex > lk(_resourceLock);

        const auto objectIt = _objects.find(objectHandle);
        if (objectIt != _objects.end())
        {
            auto itr = std::find(_objects[objectHandle].second.begin(), _objects[objectHandle].second.end(), callbackID);
            if (itr != _objects[objectHandle].second.end())
            {
                _objects[objectHandle].second.erase(itr);
            }
        }

        auto it = _nodeCallbacks.find(callbackID);
        if (it == _nodeCallbacks.end())
        {
            return;
        }
        (*it).second.second->SetWeak((*it).second.second, &ResourceManager::finaliseDestroyFunction, Nan::WeakCallbackType::kParameter);
        _nodeCallbacks.erase(it);
    }

    void ResourceManager::dukObjectDestroyed(const std::string &objectHandle)
    {
        std::unique_lock<std::mutex > lk(_resourceLock);

        /*
        auto it = _objects.find(objectHandle);
        if (it == _objects.end())
        {
            return;
        }

        std::vector<std::string> callbacks = (*it).second.second;
        _objects.erase(it);
         */

        auto it = _objects.find(objectHandle);
        if (it == _objects.end())
        {
            return;
        }
        for(const auto& cb: (*it).second.second)
        {
            auto cbIt = _nodeCallbacks.find(cb);
            if (cbIt == _nodeCallbacks.end())
            {
                continue;
            }
            (*cbIt).second.second->SetWeak((*cbIt).second.second, &ResourceManager::finaliseDestroyFunction, Nan::WeakCallbackType::kParameter);
            _nodeCallbacks.erase(cbIt);
        }
        (*it).second.second.clear();
    }

    Nan::Persistent<v8::Promise::Resolver>* ResourceManager::resolveDukPromise(const std::string& promiseID)
    {
        std::unique_lock<std::mutex > lk(_resourceLock);
        auto it = _dukPromises.find(promiseID);
        if (it == _dukPromises.end())
        {
            return nullptr;
        }
        auto resolver = (*it).second.second;
        _dukPromises.erase(it);
        return resolver;
    }

    int ResourceManager::getObjectRefCount()
    {
        std::unique_lock<std::mutex > lk(_resourceLock);
        for(const auto& obj: _objects)
        {
            std::cout << "Object reference still active: " << obj.first << ": " << obj.second.first << std::endl;
        }
        for(const auto& cb: _nodeCallbacks)
        {
            std::cout << "Callback reference still active: " << cb.first << ": " << cb.second.first << std::endl;
        }

        return static_cast<int>(_objects.size());
    }

    void ResourceManager::finaliseDestroyValue(const Nan::WeakCallbackInfo<Nan::Persistent<v8::Value>>& data)
    {
        Nan::Persistent<v8::Value>* ref = data.GetParameter();
        delete ref;
    }

    void ResourceManager::finaliseDestroyPromise(const Nan::WeakCallbackInfo<Nan::Persistent<v8::Promise::Resolver>>& data)
    {
        Nan::Persistent<v8::Promise::Resolver>* ref = data.GetParameter();
        delete ref;
    }

    template<class T>
    void ResourceManager::finaliseDestroyPersistent(const Nan::WeakCallbackInfo<Nan::Persistent<T>>& data)
    {
        Nan::Persistent<T>* ref = data.GetParameter();
        delete ref;
    }
}