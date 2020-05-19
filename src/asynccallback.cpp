#include "asynccallback.h"
#include <iostream>

Nan::Persistent<v8::Function> DukAsyncCallback::constructor;

DukAsyncCallback::DukAsyncCallback()
{
    _callback = std::unique_ptr<Duktype::AsyncCallback>(new Duktype::AsyncCallback());
}

void DukAsyncCallback::Init(v8::Local<v8::Object> exports)
{
    v8::Local<v8::Context> context = exports->CreationContext();
    Nan::HandleScope scope;

    // Prepare constructor template
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("DukAsyncCallback").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "call", Call);

    constructor.Reset(tpl->GetFunction(context).ToLocalChecked());
    exports->Set(context, Nan::New("DukAsyncCallback").ToLocalChecked(), tpl->GetFunction(context).ToLocalChecked());
}

void DukAsyncCallback::New(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    if (info.IsConstructCall())
    {
        auto* obj = new DukAsyncCallback();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
        info.GetReturnValue().Set(cons->NewInstance(context).ToLocalChecked());
    }
}

Duktype::AsyncCallback* DukAsyncCallback::getCallback()
{
    return _callback.get();
}

void DukAsyncCallback::Call(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * callbackWrapper = ObjectWrap::Unwrap<DukAsyncCallback>(info.Holder());
        callbackWrapper->getCallback()->call(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

