#pragma once

#include "Context.h"

#include <map>
#include <vector>
#include <functional>

namespace Duktype
{
    class Scope
    {
        public:
            void setContext(const std::shared_ptr<Context>& ctx);
            void setProperty(const Nan::FunctionCallbackInfo<v8::Value>& info);
            void getProperty(const Nan::FunctionCallbackInfo<v8::Value>& info);
            void deleteProperty(const Nan::FunctionCallbackInfo<v8::Value>& info);
            void callMethod(const Nan::FunctionCallbackInfo<v8::Value>& info);
            void setStack(const std::vector<std::string>& stack);
            std::vector<std::string> createObject(const Nan::FunctionCallbackInfo<v8::Value>& info);

        private:
            static int handleFunctionCall(duk_context* ctx);
            int functionCall(duk_context* ctx);
            std::map<std::string, Nan::Callback *> _callbacks;

            duk_context* _ctx = nullptr;
            std::vector<std::string> _stack;
    };
}

