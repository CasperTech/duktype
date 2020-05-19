#include "DukContextStack.h"
#include "DuktapeContext.h"
#include <iostream>

namespace Duktape
{
    DukContextStack::DukContextStack(const std::shared_ptr<DuktapeContext>& ctx, const std::vector<std::string>& stack)
            : DukObject(ctx)
    {
        duk_push_global_object(ctx->getContext());

        bool lastUndefined = false;
        _pops = 1;
        for(const auto& objName: stack)
        {
            if (lastUndefined)
            {
                while(_pops--)
                {
                    duk_pop(_ctx->getContext());
                }
                throw std::runtime_error("Can't access " + objName + " of undefined");
            }
            duk_get_prop_string(ctx->getContext(), -1, objName.c_str());
            _pops++;
            if (duk_get_type(ctx->getContext(), -1) == DUK_TYPE_UNDEFINED)
            {
                lastUndefined = true;
            }
        }

        _objectIndex = duk_get_top_index(ctx->getContext());
    }

    DukContextStack::~DukContextStack()
    {
        if (duk_get_top_index(_ctx->getContext()) != _objectIndex)
        {
            std::cerr << "DukContextStack: Stack corruption detected" << std::endl;
        }
        while(_pops--)
        {
            duk_pop(_ctx->getContext());
        }
    }
}

