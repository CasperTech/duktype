#include "DukGlobalStash.h"
#include "DuktapeContext.h"
#include <iostream>

namespace Duktape
{
    DukGlobalStash::DukGlobalStash(const std::shared_ptr<DuktapeContext>& ctx)
            : DukObject(ctx)
    {
        duk_push_global_stash(ctx->getContext());
        _objectIndex = duk_get_top_index(ctx->getContext());
    }

    DukGlobalStash::~DukGlobalStash()
    {
        if (duk_get_top_index(_ctx->getContext()) != _objectIndex)
        {
            std::cerr << "DukGlobalStash: Stack corruption detected" << std::endl;
        }
        duk_pop(_ctx->getContext());
    }
}

