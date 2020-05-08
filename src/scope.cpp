#include "scope.h"

Nan::Persistent<v8::Function> Scope::constructor;

Scope::Scope()
{
    _scope = std::make_unique<Duktype::Scope>();
}

void Scope::Init(v8::Local<v8::Object> exports)
{
    v8::Local<v8::Context> context = exports->CreationContext();
    Nan::HandleScope scope;

    // Prepare constructor template
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("Scope").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    Nan::SetPrototypeMethod(tpl, "setProperty", SetProperty);
    Nan::SetPrototypeMethod(tpl, "deleteProperty", DeleteProperty);
    Nan::SetPrototypeMethod(tpl, "getProperty", GetProperty);
    Nan::SetPrototypeMethod(tpl, "callMethod", CallMethod);
    Nan::SetPrototypeMethod(tpl, "createObject", CreateObject);

    constructor.Reset(tpl->GetFunction(context).ToLocalChecked());
    exports->Set(context, Nan::New("Scope").ToLocalChecked(), tpl->GetFunction(context).ToLocalChecked());
}

Duktype::Scope * Scope::getScope()
{
    return _scope.get();
}

void Scope::CreateObject(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<Scope>(info.Holder());
        std::vector<std::string> stack = scopeWrapper->getScope()->createObject(info);

        Nan::EscapableHandleScope scope;

        v8::Local<v8::Function> cons = Nan::New<v8::Function>(::Scope::constructor);
        v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
        auto ins1 = cons->NewInstance(context);
        auto instance = ins1.ToLocalChecked();
        auto * scp = Nan::ObjectWrap::Unwrap<Scope>(instance);
        scp->setContext(scopeWrapper->getContext());
        scp->getScope()->setStack(stack);
        info.GetReturnValue().Set(instance);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

const std::shared_ptr<Duktype::Context>& Scope::getContext()
{
    return _ctx;
}

void Scope::GetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<Scope>(info.Holder());
        scopeWrapper->getScope()->getProperty(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void Scope::SetProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<Scope>(info.Holder());
        scopeWrapper->getScope()->setProperty(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void Scope::DeleteProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<Scope>(info.Holder());
        scopeWrapper->getScope()->deleteProperty(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void Scope::CallMethod(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    try
    {
        auto * scopeWrapper = ObjectWrap::Unwrap<Scope>(info.Holder());
        scopeWrapper->getScope()->callMethod(info);
    }
    catch(const std::runtime_error& err)
    {
        Nan::ThrowError(err.what());
    }
}

void Scope::setContext(const std::shared_ptr<Duktype::Context>& ctx)
{
    _ctx = ctx;
    _scope->setContext(ctx);
}

void Scope::New(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    if (info.IsConstructCall())
    {
        auto* obj = new Scope();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
        info.GetReturnValue().Set(cons->NewInstance(context).ToLocalChecked());
    }
}


