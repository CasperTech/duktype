#include "Callback.h"

#include "Context.h"
#include <iostream>

namespace Duktype
{
    Callback::Callback()
    {
    }
    Callback::~Callback()
    {
        _context->derefCallback(_handle);
    }

    void Callback::call(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        _context->runCallback(info, _handle);
    }

    void Callback::setHandle(const std::string &handle)
    {
        _handle = handle;
    }

    void Callback::setContext(const std::shared_ptr<Context> &context)
    {
        _context = context;
    }
}