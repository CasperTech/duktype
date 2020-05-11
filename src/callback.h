#pragma once

#include "duktype/Callback.h"

#include "initnan.h"

class DukCallback: public Nan::ObjectWrap
{
    public:
        static void Init(v8::Local<v8::Object> exports);

        static Nan::Persistent<v8::Function> constructor;

        Duktype::Callback* getCallback();

    private:
        explicit DukCallback();

        static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);

        static void Call(const Nan::FunctionCallbackInfo<v8::Value> &info);

        std::unique_ptr<Duktype::Callback> _callback;
};

