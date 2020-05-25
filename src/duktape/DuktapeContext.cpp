#include "DuktapeContext.h"
#include "DukGlobalStash.h"
#include <stdexcept>
#include <iostream>
#include <duktype/DebugStack.h>

namespace Duktape
{
    DuktapeContext::DuktapeContext()
        : _ctx(duk_create_heap_default())
    {

    }

    duk_context* DuktapeContext::getContext()
    {
        return _ctx;
    }

    DukValue DuktapeContext::getGlobalString(const std::string &type)
    {
        duk_get_global_string(_ctx, type.c_str());
        return DukValue(shared_from_this());
    }

    void DuktapeContext::eval(const std::string& code, const std::function<void(DukValue& val)>& returnValueHandler)
    {
        if (duk_peval_string(_ctx, code.c_str()) != DUK_EXEC_SUCCESS)
        {
            DukValue error(shared_from_this());
            throw std::runtime_error(error.getString());
        }
        DukValue ret(shared_from_this());
        returnValueHandler(ret);
    }

    void DuktapeContext::runCallback(
            const std::shared_ptr<Duktype::Context>& context,
            uint32_t numArgs
    )
    {
        DebugStack d("DuktapeContext::RunCallback", _ctx, 0 - numArgs);
        {
            Duktape::DukValue obj = DukValue::getValue(shared_from_this(), 0 - (numArgs + 2));
            obj.callMethod(context, numArgs);
        }
    }
}