#pragma once

#include <initnan.h>

#include <duktape.h>

namespace Duktype
{
    class Context;
    class Utils
    {
        public:
            template<typename T>
            static v8::Local<T> createTypedArray(size_t byteLength, char * ptr, int entrySize);

            static void resolveRelativeIndex(int &index, duk_context * ctx);

            static v8::Local<v8::Value> dukToV8(int index, std::shared_ptr<Duktype::Context>& ctxObj);

            static bool instanceOf(int index, const std::string &type, duk_context * ctx);

            static bool v8ToDuk(const v8::Local<v8::Value> &persistedVal, duk_context * ctx);
    };
}

