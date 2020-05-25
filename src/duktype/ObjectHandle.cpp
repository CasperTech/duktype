#include "ObjectHandle.h"
#include <duktape/DuktapeContext.h>
#include <iostream>

namespace Duktype
{
    ObjectHandle::ObjectHandle(const std::shared_ptr<Duktape::DuktapeContext>& ctx, const std::string& handle)
            : Duktape::DukObject(ctx)
            , _handle(handle)
    {
        if (handle.empty() || handle == "GLOBAL")
        {
            duk_push_global_object(ctx->getContext());
        }
        else
        {
            duk_push_global_stash(ctx->getContext());
            duk_get_prop_string(ctx->getContext(), -1, handle.c_str());
        }

        _objectIndex = duk_get_top_index(ctx->getContext());
    }

    ObjectHandle::~ObjectHandle()
    {
        if (_handle.empty() || _handle == "GLOBAL")
        {
            duk_pop(_ctx->getContext());
        }
        else
        {
            duk_pop_2(_ctx->getContext());
        }
    }
}

