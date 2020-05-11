#include "context.h"
#include "scope.h"
#include "callback.h"
#include "duktype/Callback.h"
#include <iostream>

Nan::Persistent<v8::Function> Context::constructor;

Context::Context()
{
    _ctx = std::make_shared<Duktype::Context>();
}

const std::shared_ptr<Duktype::Context>& Context::getCtx()
{
    return _ctx;
}

void Context::Init(v8::Local<v8::Object> exports)
{
    v8::Local<v8::Context> context = exports->CreationContext();
    Nan::HandleScope scope;

    // Prepare constructor template
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("Context").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(tpl, "eval", Eval);
    Nan::SetPrototypeMethod(tpl, "getGlobalObject", GetGlobalObject);


    constructor.Reset(tpl->GetFunction(context).ToLocalChecked());
    exports->Set(context,
                 Nan::New("Context").ToLocalChecked(),
                 tpl->GetFunction(context).ToLocalChecked());
}

void Context::Eval(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * contextWrapper = ObjectWrap::Unwrap<Context>(info.Holder());
        contextWrapper->getCtx()->eval(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void Context::New(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    if (info.IsConstructCall())
    {
        Context * obj = new Context();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
        info.GetReturnValue().Set(cons->NewInstance(context).ToLocalChecked());
    }
}

void Context::GetGlobalObject(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    Nan::EscapableHandleScope scope;

    v8::Local<v8::Function> cons = Nan::New<v8::Function>(::Scope::constructor);
    v8::Local<v8::Object> instance = Nan::NewInstance(cons).ToLocalChecked();
    auto* scp = Nan::ObjectWrap::Unwrap<Scope>(instance);
    auto* ctxWrapper = ObjectWrap::Unwrap<Context>(info.Holder());
    scp->setContext(ctxWrapper->getCtx());
    info.GetReturnValue().Set(instance);
}

