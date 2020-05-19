#include "DukGlobalObject.h"
#include "DuktapeContext.h"
#include <iostream>

namespace Duktape
{
    DukGlobalObject::DukGlobalObject(const std::shared_ptr<DuktapeContext>& ctx)
            : DukObject(ctx)
    {
        duk_push_global_object(ctx->getContext());
        _objectIndex = duk_get_top_index(ctx->getContext());
    }

    DukGlobalObject::~DukGlobalObject()
    {
        if (duk_get_top_index(_ctx->getContext()) != _objectIndex)
        {
            std::cerr << "DukGlobalObject: Stack corruption detected" << std::endl;
        }
        duk_pop(_ctx->getContext());
    }
}

