#pragma once

#include "duktype/AsyncCallback.h"

#include "initnan.h"

class DukAsyncCallback: public Nan::ObjectWrap
{
    public:
        static void Init(v8::Local<v8::Object> exports);

        static Nan::Persistent<v8::Function> constructor;

        Duktype::AsyncCallback* getCallback();

    private:
        explicit DukAsyncCallback();

        static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void Call(const Nan::FunctionCallbackInfo<v8::Value> &info);

        std::unique_ptr<Duktype::AsyncCallback> _callback;
};

