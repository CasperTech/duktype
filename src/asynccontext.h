#pragma once

#include "duktype/AsyncContext.h"

#include "initnan.h"

#include <memory>

class AsyncContext: public Nan::ObjectWrap
{
    public:
        static void Init(v8::Local<v8::Object> exports);

        const std::shared_ptr<Duktype::AsyncContext>& getCtx();

    private:
        explicit AsyncContext();
        ~AsyncContext();

        static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
        static void Eval(const Nan::FunctionCallbackInfo<v8::Value>& info);
        static void RunGC(const Nan::FunctionCallbackInfo<v8::Value>& info);
        static void GetObjectReferenceCount(const Nan::FunctionCallbackInfo<v8::Value>& info);

        static void GetGlobalObject(const Nan::FunctionCallbackInfo<v8::Value>& info);
        static Nan::Persistent<v8::Function> constructor;

        std::shared_ptr<Duktype::AsyncContext> _ctx;
};
