#pragma once

#include "duktype/Context.h"
#include "duktype/ObjectScope.h"

#include "initnan.h"

class ObjectScope: public Nan::ObjectWrap
{
    public:
        static void Init(v8::Local<v8::Object> exports);

        static Nan::Persistent<v8::Function> constructor;

        void setContext(const std::shared_ptr<Duktype::Context> &);

        const std::shared_ptr<Duktype::Context> &getContext();

        Duktype::ObjectScope* getObjectScope();

    private:
        explicit ObjectScope();

        static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void SetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void GetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void DeleteProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void CallMethod(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void CreateObject(const Nan::FunctionCallbackInfo<v8::Value> &info);

        std::shared_ptr<Duktype::Context> _ctx;
        std::unique_ptr<Duktype::ObjectScope> _scope;
};

