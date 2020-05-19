#include "ObjectScope.h"

#include <iostream>

namespace Duktype
{
    ObjectScope::ObjectScope()
    {
    }

    ObjectScope::~ObjectScope()
    {
        _ctx->derefObject(_handle);
    }

    void ObjectScope::setHandle(const std::string& handle)
    {
        _handle = handle;
    }

    void ObjectScope::setContext(const std::shared_ptr<Duktype::Context>& ctx)
    {
        _ctx = ctx;
    }

    void ObjectScope::setProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        _ctx->setProperty(_handle, info);
    }

    void ObjectScope::getProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        _ctx->getProperty(_handle, info);
    }

    void ObjectScope::deleteProperty(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        _ctx->deleteProperty(_handle, info);
    }

    std::string ObjectScope::getObject(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() > 0 && info[0]->IsString())
        {
            std::string objNameStr = *Nan::Utf8String(info[0]);
            std::string handle = _ctx->getObject(_handle, info);
            return handle;
        }
        else
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
        }
        return {};
    }

    std::string ObjectScope::createObject(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() > 0 && info[0]->IsString())
        {
            std::string objNameStr = *Nan::Utf8String(info[0]);
            std::string handle = _ctx->createObject(_handle, info);
            return handle;
        }
        else
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
        }
        return {};
    }

    void ObjectScope::callMethod(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() < 1 || !info[0]->IsString())
        {
            Nan::ThrowError("Please provide a method name");
            return;
        }
        std::string methodName = *Nan::Utf8String(info[0]);
        _ctx->callMethod(_handle, methodName, info);
    }
}