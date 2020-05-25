#include "DukEnum.h"
#include "DuktapeContext.h"
#include <iostream>

namespace Duktape
{
    DukEnum::DukEnum(const std::shared_ptr<DuktapeContext>& ctx, int stackIndex)
            : _ctx(ctx)
            , _objIndex(stackIndex)
    {
        duk_enum(ctx->getContext(), _objIndex, 0);
        _stackIndex = duk_get_top_index(ctx->getContext());
    }

    bool DukEnum::next(bool includeValue, const std::function<void(DukValue&,DukValue&)>& cb)
    {
        bool res = duk_next(_ctx->getContext(), _stackIndex, includeValue ? 1 : 0);
        if (res)
        {
            if(!includeValue)
            {
                duk_push_undefined(_ctx->getContext());
            }
            DukValue key(_ctx, -1);
            DukValue value(_ctx);
            cb(key, value);
        }
        return res;
    }

    DukEnum::~DukEnum()
    {
        if (duk_get_top_index(_ctx->getContext()) != _stackIndex)
        {
            std::cerr << "DukEnum: Stack corruption detected" << std::endl;
        }
        duk_pop(_ctx->getContext());
    }
}