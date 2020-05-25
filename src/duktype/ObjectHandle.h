#pragma once
#pragma once

#include <duktape/DukObject.h>

#include <duktape.h>
#include <initnan.h>

#include <string>
#include <memory>
#include <vector>

namespace Duktype
{
    class DuktapeContext;
    class ObjectHandle: public Duktape::DukObject
    {
        public:
            ObjectHandle(const std::shared_ptr<Duktape::DuktapeContext>& ctx, const std::string& handle);
            ~ObjectHandle();

        private:
            std::string _handle = "GLOBAL";
            int _pops = 0;
    };
}