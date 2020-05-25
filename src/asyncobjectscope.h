#pragma once

#include "duktype/AsyncContext.h"
#include "duktype/AsyncObjectScope.h"

#include "initnan.h"

class AsyncObjectScope: public Nan::ObjectWrap
{
    public:
        static void Init(v8::Local<v8::Object> exports);

        static Nan::Persistent<v8::Function> constructor;

        void setContext(const std::shared_ptr<Duktype::AsyncContext> &);

        const std::shared_ptr<Duktype::AsyncContext> &getContext();

        Duktype::AsyncObjectScope* getObjectScope();

    private:
        explicit AsyncObjectScope();
        ~AsyncObjectScope();

        static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void SetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void GetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void DeleteProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void CallMethod(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void CreateObject(const Nan::FunctionCallbackInfo<v8::Value> &info);

        std::shared_ptr<Duktype::AsyncContext> _ctx;
        std::unique_ptr<Duktype::AsyncObjectScope> _scope;
};

