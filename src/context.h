#pragma once

#include "duktype/Context.h"

#include "initnan.h"

#include <memory>

class Context: public Nan::ObjectWrap
{
    public:
        static void Init(v8::Local<v8::Object> exports);

        const std::shared_ptr<Duktype::Context>& getCtx();

    private:
        explicit Context();

        static void New(const Nan::FunctionCallbackInfo<v8::Value> &info);
        static void Eval(const Nan::FunctionCallbackInfo<v8::Value>& info);
        static void GetGlobalObject(const Nan::FunctionCallbackInfo<v8::Value>& info);
        static Nan::Persistent<v8::Function> constructor;

        std::shared_ptr<Duktype::Context> _ctx;
};
