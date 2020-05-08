#pragma once
#include <duktape.h>

class ContextStack
{
    public:
        ContextStack(duk_context * ctx, const std::vector<std::string>& stack)
            : _ctx(ctx)
            , _stack(stack)
        {
            duk_push_global_object(_ctx);
            bool lastUndefined = false;
            _pops = 1;
            for(const auto& objName: stack)
            {
                if (lastUndefined)
                {
                    while(_pops--)
                    {
                        duk_pop(_ctx);
                    }
                    throw std::runtime_error("Can't access " + objName + " of undefined");
                }
                duk_get_prop_string(ctx, -1, objName.c_str());
                _pops++;
                if (duk_get_type(ctx, -1) == DUK_TYPE_UNDEFINED)
                {
                    lastUndefined = true;
                }
            }
        }

        ~ContextStack()
        {
            while(_pops--)
            {
                duk_pop(_ctx);
            }
        }

    private:
        int _pops = 0;
        duk_context * _ctx;
        std::vector<std::string> _stack;
};