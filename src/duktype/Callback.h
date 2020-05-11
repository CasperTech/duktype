#pragma once

#include <duktape.h>
#include <initnan.h>

#include <string>

namespace Duktype
{
    class Context;
    class Callback
    {
        public:
            ~Callback();

            void call(const Nan::FunctionCallbackInfo <v8::Value> &info);

            void setHandle(const std::string &cbHandle);

            void setContextObj(std::shared_ptr<Duktype::Context>& ptr);

        private:
            std::string _cbHandle;
            std::shared_ptr<Duktype::Context> _ctx;
    };
}