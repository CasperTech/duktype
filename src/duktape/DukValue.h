#pragma once

#include "DukObject.h"

#include <duktape.h>
#include <initnan.h>

#include <string>
#include <memory>

namespace Duktype
{
    class Context;
}
namespace Duktape
{
    class DuktapeContext;
    class DukValue: public DukObject
    {
        public:
            static void newArray(const std::shared_ptr<DuktapeContext>& ctx);
            static void newString(const std::shared_ptr<DuktapeContext>& ctx, const std::string& str);
            static void newBoolean(const std::shared_ptr<DuktapeContext>& ctx, bool value);
            static void newNumber(const std::shared_ptr<DuktapeContext>& ctx, double value);
            static void newUndefined(const std::shared_ptr<DuktapeContext>& ctx);
            static void newNull(const std::shared_ptr<DuktapeContext>& ctx);
            static void newBuffer(const std::shared_ptr<DuktapeContext>& ctx, char* buffer, size_t length, bool dynamic);
            static void newObject(const std::shared_ptr<DuktapeContext>& ctx);
            static DukValue getValue(const std::shared_ptr<DuktapeContext>& ctx, int index);
            
            DukValue(const std::shared_ptr<DuktapeContext>& ctx, int indexOffset = 0, bool resolved = false);
            ~DukValue();
            std::string getString();
            double getNumber();
            uint32_t getIndex();
            void dupe();
            void callNew(int params);
            bool isUndefined();
            bool isCallable();
            bool isObject();
            void resolved(bool resolved = true);
            void convertBufferToBufferObject(size_t length, int type);

            v8::Local<v8::Value> toV8(const std::shared_ptr<Duktype::Context>& context);
            static void fromV8(const std::shared_ptr<Duktype::Context>& context, v8::Local<v8::Value> val);

        private:
            template<typename T>
            static v8::Local<T> createTypedArray(size_t byteLength, char* ptr, int entrySize);
            int _valIndex = 0;
            bool _resolved = false;
            std::shared_ptr<DuktapeContext> _ctx;
            int32_t _stackIndex = -1;
    };
}