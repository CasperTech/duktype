#pragma once

#include "DukObject.h"

#include <duktape.h>
#include <initnan.h>

#include <string>
#include <memory>
#include <functional>

namespace Duktype
{
    class Context;
}
namespace Duktape
{
    class DukValue;
    class DuktapeContext;
    class DukObject
    {
        public:
            explicit DukObject(const std::shared_ptr<DuktapeContext>& ctx);
            virtual ~DukObject();
            void setProperty(const std::string& propertyName, DukValue& value);
            int getIndex();
            DukValue getProperty(const std::string& propertyName);
            void setPropertyIndex(int index, DukValue& value);
            bool instanceOf(const std::string& type);
            DukValue callMethod(const std::shared_ptr<Duktype::Context>& context, const std::string& methodName, Nan::Persistent<v8::Value>* args = nullptr, uint32_t numArgs = 0);
            void callMethod(const std::shared_ptr<Duktype::Context>& context, uint32_t numArgs);

        protected:
            std::shared_ptr<DuktapeContext> _ctx;
            uint32_t _objectIndex;
    };
}