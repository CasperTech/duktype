#pragma once

#include <duktape.h>
#include <initnan.h>

#include <string>
#include <memory>
#include <functional>

namespace Duktape
{
    class DukValue;
    class DuktapeContext;
    class DukEnum
    {
        public:
            DukEnum(const std::shared_ptr<DuktapeContext>& ctx, int stackIndex);
            bool next(bool includeValue, const std::function<void(DukValue&,DukValue&)>& cb);
            ~DukEnum();
        private:
            std::shared_ptr<DuktapeContext> _ctx;
            uint32_t _stackIndex = 0;
            uint32_t _objIndex = 0;
    };
}