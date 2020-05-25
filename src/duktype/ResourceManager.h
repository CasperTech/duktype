#pragma once

#include <initnan.h>

#include <map>
#include <vector>
#include <string>
#include <set>
#include <mutex>
#include <memory>

namespace Duktype
{
    class Promise;
    class ResourceManager
    {
        public:
            ~ResourceManager();

            void cleanAll();

            void addCallbackFromNode(const std::string& description, const std::string& objectID, const std::string& callbackID, Nan::Persistent<v8::Function>* func);
            void addCallbackFromDuk(const std::string& description, const std::string& callbackID);

            void addPromiseFromNode(const std::string& promiseHandle, const std::shared_ptr<Promise>& promise);
            void addPromiseFromDuk(const std::string& promiseHandle, Nan::Persistent<v8::Promise::Resolver>* resolver);

            void addDukObjectForNode(const std::string& description, const std::string& objectID);

            Nan::Persistent<v8::Function>* getNodeCallback(const std::string &callbackID);

            void nodeCallbackDereferencedInDuk(const std::string &objectHandle, const std::string &callbackID);
            void dukObjectDestroyed(const std::string &objectHandle);

            void removeCallbackFromDuk(const std::string &callbackID);

            Nan::Persistent<v8::Promise::Resolver>* resolveDukPromise(const std::string& promiseID);
            std::shared_ptr<Promise> resolveNodePromise(const std::string& promiseHandle);

            void removeNodeObjectHandle(const std::string& handle);

            int getObjectRefCount();

            static void finaliseDestroyPromise(const Nan::WeakCallbackInfo<Nan::Persistent<v8::Promise::Resolver>> &data);
            static void finaliseDestroyValue(const Nan::WeakCallbackInfo<Nan::Persistent<v8::Value>> &data);

            template<class T>
            static void finaliseDestroyPersistent(const Nan::WeakCallbackInfo<Nan::Persistent<T>> &data);

        private:

            static void finaliseDestroyFunction(const Nan::WeakCallbackInfo<Nan::Persistent<v8::Function>> &data);

            std::map<std::string, std::pair<std::string, Nan::Persistent<v8::Function>*>> _nodeCallbacks;
            std::map<std::string, std::pair<std::string, std::vector<std::string>>> _objects;
            std::map<std::string, std::string> _dukCallbacks;
            std::map<std::string, std::pair<std::string, std::shared_ptr<Promise>>> _promises;
            std::map<std::string, std::pair<std::string, Nan::Persistent<v8::Promise::Resolver>*>> _dukPromises;
            std::mutex _resourceLock;
    };
}