#pragma once

#include "../Nan.h"

#include <duktape.h>

namespace Duktype
{
    class Utils
    {
        public:
            template<typename T>
            static v8::Local<T> createTypedArray(size_t byteLength, char * ptr);

            static void resolveRelativeIndex(int &index, duk_context * ctx);

            static v8::Local<v8::Value> dukToV8(int index, duk_context * ctx);

            static bool instanceOf(int index, const std::string &type, duk_context * ctx);

            static bool v8ToDuk(const Nan::Persistent<v8::Value> &persistedVal, duk_context * ctx);

        private:
            template<typename T>
            struct V8TypedArrayTraits; // no generic case
            template<>
            struct V8TypedArrayTraits<v8::Float32Array>
            {
                typedef float value_type;
            };
            template<>
            struct V8TypedArrayTraits<v8::Float64Array>
            {
                typedef double value_type;
            };
            template<>
            struct V8TypedArrayTraits<v8::Uint8Array>
            {
                typedef uint8_t value_type;
            };
            template<>
            struct V8TypedArrayTraits<v8::Uint8ClampedArray>
            {
                typedef uint8_t value_type;
            };
            template<>
            struct V8TypedArrayTraits<v8::Uint16Array>
            {
                typedef uint16_t value_type;
            };
            template<>
            struct V8TypedArrayTraits<v8::Uint32Array>
            {
                typedef uint32_t value_type;
            };
            template<>
            struct V8TypedArrayTraits<v8::Int8Array>
            {
                typedef int8_t value_type;
            };
            template<>
            struct V8TypedArrayTraits<v8::Int16Array>
            {
                typedef int16_t value_type;
            };
            template<>
            struct V8TypedArrayTraits<v8::Int32Array>
            {
                typedef int32_t value_type;
            };
    };
}

