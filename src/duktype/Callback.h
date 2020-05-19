#pragma once

#include <initnan.h>
#include <mutex>

namespace Duktype
{
    class Context;
    class Callback
    {
        public:
            Callback();
            ~Callback();
            void call(const Nan::FunctionCallbackInfo<v8::Value> &info);
            void setHandle(const std::string& handle);
            void setContext(const std::shared_ptr<Context>& context);

        private:
            std::string _handle;
            std::shared_ptr<Context> _context;
    };
}