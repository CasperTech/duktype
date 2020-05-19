#include "asyncobjectscope.h"
#include <initnan.h>

#include <memory>
#include <vector>
#include <iostream>

Nan::Persistent<v8::Function> AsyncObjectScope::constructor;

AsyncObjectScope::AsyncObjectScope()
{
    _scope = std::unique_ptr<Duktype::AsyncObjectScope>(new Duktype::AsyncObjectScope());
}

void AsyncObjectScope::Init(v8::Local<v8::Object> exports)
{
    v8::Local<v8::Context> context = exports->CreationContext();
    Nan::HandleScope scope;

    // Prepare constructor template
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("AsyncObjectScope").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "setProperty", SetProperty);
    Nan::SetPrototypeMethod(tpl, "deleteProperty", DeleteProperty);
    Nan::SetPrototypeMethod(tpl, "getProperty", GetProperty);
    Nan::SetPrototypeMethod(tpl, "callMethod", CallMethod);
    Nan::SetPrototypeMethod(tpl, "createObject", CreateObject);

    constructor.Reset(tpl->GetFunction(context).ToLocalChecked());
    exports->Set(context, Nan::New("AsyncObjectScope").ToLocalChecked(), tpl->GetFunction(context).ToLocalChecked());
}

Duktype::AsyncObjectScope * AsyncObjectScope::getObjectScope()
{
    return _scope.get();
}

void AsyncObjectScope::CreateObject(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<AsyncObjectScope>(info.Holder());
        scopeWrapper->getObjectScope()->createObjectAsync(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

const std::shared_ptr<Duktype::AsyncContext>& AsyncObjectScope::getContext()
{
    return _ctx;
}

void AsyncObjectScope::GetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<AsyncObjectScope>(info.Holder());
        scopeWrapper->getObjectScope()->getProperty(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void AsyncObjectScope::SetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<AsyncObjectScope>(info.Holder());
        scopeWrapper->getObjectScope()->setProperty(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void AsyncObjectScope::DeleteProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<AsyncObjectScope>(info.Holder());
        scopeWrapper->getObjectScope()->deleteProperty(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void AsyncObjectScope::CallMethod(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<AsyncObjectScope>(info.Holder());
        scopeWrapper->getObjectScope()->callMethod(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void AsyncObjectScope::setContext(const std::shared_ptr<Duktype::AsyncContext>& ctx)
{
    _ctx = ctx;
    _scope->setContext(ctx);
}

void AsyncObjectScope::New(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    if (info.IsConstructCall())
    {
        auto* obj = new AsyncObjectScope();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
        info.GetReturnValue().Set(cons->NewInstance(context).ToLocalChecked());
    }
}


