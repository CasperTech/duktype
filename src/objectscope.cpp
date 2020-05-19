#include "objectscope.h"
#include <initnan.h>

#include <memory>
#include <vector>

Nan::Persistent<v8::Function> ObjectScope::constructor;

ObjectScope::ObjectScope()
{
    _scope = std::unique_ptr<Duktype::ObjectScope>(new Duktype::ObjectScope());
}

void ObjectScope::Init(v8::Local<v8::Object> exports)
{
    v8::Local<v8::Context> context = exports->CreationContext();
    Nan::HandleScope scope;

    // Prepare constructor template
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("ObjectScope").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "setProperty", SetProperty);
    Nan::SetPrototypeMethod(tpl, "deleteProperty", DeleteProperty);
    Nan::SetPrototypeMethod(tpl, "getProperty", GetProperty);
    Nan::SetPrototypeMethod(tpl, "callMethod", CallMethod);
    Nan::SetPrototypeMethod(tpl, "createObject", CreateObject);

    constructor.Reset(tpl->GetFunction(context).ToLocalChecked());
    exports->Set(context, Nan::New("ObjectScope").ToLocalChecked(), tpl->GetFunction(context).ToLocalChecked());
}

Duktype::ObjectScope * ObjectScope::getObjectScope()
{
    return _scope.get();
}

void ObjectScope::CreateObject(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<ObjectScope>(info.Holder());
        std::string handle = scopeWrapper->getObjectScope()->createObject(info);

        Nan::EscapableHandleScope scope;

        v8::Local<v8::Function> cons = Nan::New<v8::Function>(::ObjectScope::constructor);
        v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
        auto ins1 = cons->NewInstance(context);
        auto instance = ins1.ToLocalChecked();
        auto * scp = Nan::ObjectWrap::Unwrap<ObjectScope>(instance);
        scp->setContext(scopeWrapper->getContext());
        scp->getObjectScope()->setHandle(handle);
        info.GetReturnValue().Set(instance);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

const std::shared_ptr<Duktype::Context>& ObjectScope::getContext()
{
    return _ctx;
}

void ObjectScope::GetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<ObjectScope>(info.Holder());
        scopeWrapper->getObjectScope()->getProperty(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void ObjectScope::SetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<ObjectScope>(info.Holder());
        scopeWrapper->getObjectScope()->setProperty(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void ObjectScope::DeleteProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<ObjectScope>(info.Holder());
        scopeWrapper->getObjectScope()->deleteProperty(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void ObjectScope::CallMethod(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<ObjectScope>(info.Holder());
        scopeWrapper->getObjectScope()->callMethod(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void ObjectScope::setContext(const std::shared_ptr<Duktype::Context>& ctx)
{
    _ctx = ctx;
    _scope->setContext(ctx);
}

void ObjectScope::New(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    if (info.IsConstructCall())
    {
        auto* obj = new ObjectScope();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
        info.GetReturnValue().Set(cons->NewInstance(context).ToLocalChecked());
    }
}


