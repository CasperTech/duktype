#pragma once

#include "../Nan.h"

#include <duktape.h>

namespace Duktype
{
    class Utils
    {
        public:
            template<typename T>
            static v8::Local<T> createTypedArray(size_t byteLength, char * ptr, int entrySize);

            static void resolveRelativeIndex(int &index, duk_context * ctx);

            static v8::Local<v8::Value> dukToV8(int index, duk_context * ctx);

            static bool instanceOf(int index, const std::string &type, duk_context * ctx);

            static bool v8ToDuk(const Nan::Persistent<v8::Value> &persistedVal, duk_context * ctx);
    };
}

