#pragma once

#include <duktape/DuktapeContext.h>

#include <initnan.h>

#include <memory>
#include <map>
#include <set>
#include <mutex>

namespace Duktype
{
    struct CallbackWeakRef;
    class Promise;
    class Context: public std::enable_shared_from_this<Context>
    {
        public:
            ~Context();
            Context();

            virtual void eval(const Nan::FunctionCallbackInfo<v8::Value> &info);
            virtual void runCallback(const Nan::FunctionCallbackInfo<v8::Value> &info, const std::string& cbName);
            virtual v8::Local<v8::Function> getCallbackConstructor();
            virtual void registerPromise(const std::string& promiseHandle, const std::shared_ptr<Promise>& promise);
            virtual void resolvePromise(const std::string& promiseHandle, const Nan::FunctionCallbackInfo<v8::Value>& info);
            virtual void catchPromise(const std::string& promiseHandle, const Nan::FunctionCallbackInfo<v8::Value>& info);
            virtual void registerDukPromise(const std::string& promiseHandle, Nan::Persistent<v8::Promise::Resolver>* resolver);
            virtual void setProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info);
            virtual void getProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info);
            virtual void deleteProperty(const std::string& objectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info);
            virtual std::string createObject(const std::string& parentObjectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info);
            virtual std::string getObject(const std::string& parentObjectHandle, const Nan::FunctionCallbackInfo<v8::Value> &info);
            virtual void getObjectReferenceCount(const Nan::FunctionCallbackInfo<v8::Value> &info);
            virtual void callMethod(const std::string& objectHandle, const std::string& methodName, const Nan::FunctionCallbackInfo<v8::Value>& info);
            virtual void handleDukThen(const std::string& promiseID, duk_context* c);
            virtual void handleDukCatch(const std::string& promiseID, duk_context* c);
            virtual void runGC(const Nan::FunctionCallbackInfo<v8::Value> &info);
            virtual void derefObject(const std::string& handle);
            virtual void derefCallback(const std::string& handle);
            virtual void addCallback(const std::string& handle);
            std::shared_ptr<::Duktape::DuktapeContext> getDuktape();
            void callbackDestroyed(const std::string& callbackID);

        protected:
            static void handleCallbackDestroyed(const Nan::WeakCallbackInfo<CallbackWeakRef> &data);
            static int handleFunctionFinalised(duk_context* ctx);
            static int handleObjectFinalised(duk_context* ctx);
            void evalInternal(const std::string& code, const std::function<void(Duktape::DukValue& val)>& returnFunc);
            std::shared_ptr<::Duktape::DuktapeContext> _duktape;
            std::map<std::string, Nan::Persistent<v8::Function>*> _callbacks;
            std::set<std::string> _dukCallbacks;
            std::set<std::string> _objects;
            std::map<std::string, std::shared_ptr<Promise>> _promises;
            std::map<std::string, Nan::Persistent<v8::Promise::Resolver>*> _dukPromises;
            std::mutex _callbacksLock;

        private:

            static int handleFunctionCall(duk_context* ctx);
            virtual int functionCall(duk_context* ctx);
    };
}