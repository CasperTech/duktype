#include "asynccontext.h"
#include "asyncobjectscope.h"
#include <iostream>

Nan::Persistent<v8::Function> AsyncContext::constructor;

AsyncContext::~AsyncContext()
{
    
}

AsyncContext::AsyncContext()
{
    _ctx = std::make_shared<Duktype::AsyncContext>();
}

const std::shared_ptr<Duktype::AsyncContext>& AsyncContext::getCtx()
{
    return _ctx;
}

void AsyncContext::Init(v8::Local<v8::Object> exports)
{
    v8::Local<v8::Context> context = exports->CreationContext();
    Nan::HandleScope scope;

    // Prepare constructor template
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("AsyncContext").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    Nan::SetPrototypeMethod(tpl, "eval", Eval);
    Nan::SetPrototypeMethod(tpl, "getGlobalObject", GetGlobalObject);
    Nan::SetPrototypeMethod(tpl, "cleanRefs", CleanRefs);
    Nan::SetPrototypeMethod(tpl, "getObjectReferenceCount", GetObjectReferenceCount);
    Nan::SetPrototypeMethod(tpl, "runGC", RunGC);


    constructor.Reset(tpl->GetFunction(context).ToLocalChecked());
    exports->Set(context,
                 Nan::New("AsyncContext").ToLocalChecked(),
                 tpl->GetFunction(context).ToLocalChecked());
}

void AsyncContext::RunGC(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * contextWrapper = ObjectWrap::Unwrap<AsyncContext>(info.Holder());
        contextWrapper->getCtx()->runGC(info);
    }
    catch(const std::exception& err)
    {
        Nan::ThrowError(err.what());
    }
}

void AsyncContext::GetObjectReferenceCount(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * contextWrapper = ObjectWrap::Unwrap<AsyncContext>(info.Holder());
        contextWrapper->getCtx()->getObjectReferenceCount(info);
    }
    catch(const std::exception& err)
    {
        Nan::ThrowError(err.what());
    }
}

void AsyncContext::Eval(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * contextWrapper = ObjectWrap::Unwrap<AsyncContext>(info.Holder());
        contextWrapper->getCtx()->eval(info);
    }
    catch(const std::exception& err)
    {
        Nan::ThrowError(err.what());
    }
}

void AsyncContext::New(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    if (info.IsConstructCall())
    {
        auto* obj = new AsyncContext();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
        info.GetReturnValue().Set(cons->NewInstance(context).ToLocalChecked());
    }
}

void AsyncContext:: CleanRefs(const Nan::FunctionCallbackInfo<v8::Value>& info)
{
    try
    {
        auto * contextWrapper = ObjectWrap::Unwrap<AsyncContext>(info.Holder());
        contextWrapper->getCtx()->cleanRefs(info);
    }
    catch(const std::exception& err)
    {
        Nan::ThrowError(err.what());
    }
}

void AsyncContext::GetGlobalObject(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * contextWrapper = ObjectWrap::Unwrap<AsyncContext>(info.Holder());
        contextWrapper->getCtx()->getObject("", info);
    }
    catch(const std::exception& err)
    {
        Nan::ThrowError(err.what());
    }
}

