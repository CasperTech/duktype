#include "DukObject.h"
#include "DukValue.h"
#include "DuktapeContext.h"

#include <iostream>

namespace Duktape
{
    DukObject::DukObject(const std::shared_ptr<DuktapeContext> &ctx)
        : _ctx(ctx)
    {

    }

    void DukObject::setProperty(const std::string &propertyName, DukValue& value)
    {
        duk_put_prop_string(_ctx->getContext(), _objectIndex, propertyName.c_str());
        value.resolved();
    }

    int DukObject::getIndex()
    {
        return _objectIndex;
    }

    void DukObject::setPropertyIndex(int index, DukValue& value)
    {
        duk_put_prop_index(_ctx->getContext(), _objectIndex, index);
        value.resolved();
    }

    DukValue DukObject::getProperty(const std::string &propertyName)
    {
        duk_get_prop_string(_ctx->getContext(), _objectIndex, propertyName.c_str());
        return DukValue(_ctx);
    }

    bool DukObject::instanceOf(const std::string& type)
    {
        DukValue typeName = _ctx->getGlobalString(type);
        if (!typeName.isObject())
        {
            return false;
        }
        return duk_instanceof(_ctx->getContext(), _objectIndex, typeName.getIndex());
    }

    void DukObject::callMethod(const std::shared_ptr<Duktype::Context>& context, uint32_t numArgs)
    {
        if (duk_pcall_prop(_ctx->getContext(), _objectIndex, static_cast<int>(numArgs)) != DUK_EXEC_SUCCESS)
        {
            std::string error = std::string(duk_safe_to_string(_ctx->getContext(), -1));
            throw std::runtime_error(error);
        }
    }

    DukValue DukObject::callMethod(const std::shared_ptr<Duktype::Context>& context, const std::string& methodName, Nan::Persistent<v8::Value>* args, uint32_t numArgs)
    {
        DukValue::newString(_ctx, methodName);
        DukValue funcName(_ctx);

        for(size_t x = 0; x < numArgs; x++)
        {
            DukValue::fromV8(context, Nan::New(args[x]));
        }
        delete[] args;
        if (duk_pcall_prop(_ctx->getContext(), _objectIndex, static_cast<int>(numArgs)) != DUK_EXEC_SUCCESS)
        {
            duk_throw(_ctx->getContext());
        }
        funcName.resolved();
        return funcName;
    }

    DukObject::~DukObject()
    {

    }
}