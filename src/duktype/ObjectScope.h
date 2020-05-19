#pragma once

#include "Context.h"
#include <initnan.h>

#include <string>

namespace Duktype
{
    class ObjectScope
    {
        public:
            ObjectScope();
            ~ObjectScope();
            void setProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);
            void getProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);
            void deleteProperty(const Nan::FunctionCallbackInfo<v8::Value> &info);
            void callMethod(const Nan::FunctionCallbackInfo<v8::Value> &info);
            std::string createObject(const Nan::FunctionCallbackInfo<v8::Value> &info);
            std::string getObject(const Nan::FunctionCallbackInfo<v8::Value> &info);
            void setHandle(const std::string& handle);
            void setContext(const std::shared_ptr<Duktype::Context>& ctx);

        protected:
            std::string _handle;
            std::shared_ptr<Duktype::Context> _ctx;
    };
}