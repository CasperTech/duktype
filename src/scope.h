#pragma once

#include "duktype/Context.h"
#include "duktype/Scope.h"

#include "initnan.h"

class Scope: public Nan::ObjectWrap
{
    public:
        static void Init(v8::Local<v8::Object> exports);

        static Nan::Persistent<v8::Function> constructor;

        void setContext(const std::shared_ptr<Duktype::Context> &);

        const std::shared_ptr<Duktype::Context> &getContext();

        Duktype::Scope* getScope();

    private:
        explicit Scope();

        static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void SetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void GetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void DeleteProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void CallMethod(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void CreateObject(const Nan::FunctionCallbackInfo<v8::Value> &info);

        std::shared_ptr<Duktype::Context> _ctx;
        std::unique_ptr<Duktype::Scope> _scope;
};

