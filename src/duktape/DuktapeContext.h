#pragma once

#include "DukValue.h"
#include <duktape.h>

#include <string>

namespace Duktape
{
    class DuktapeContext: public std::enable_shared_from_this<DuktapeContext>
    {
        public:
            DuktapeContext();
            void eval(const std::string& code, const std::function<void(DukValue& val)>& returnValueHandler);
            void runCallback(
                    const std::shared_ptr<Duktype::Context>& context,
                    uint32_t numArgs
            );
            DukValue getGlobalString(const std::string& type);
            duk_context* getContext();

        private:
            bool _async = false;
            duk_context* _ctx;
    };
}

