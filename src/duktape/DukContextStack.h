#pragma once
#pragma once

#include "DukObject.h"

#include <duktape.h>
#include <initnan.h>

#include <string>
#include <memory>
#include <vector>

namespace Duktape
{
    class DuktapeContext;
    class DukContextStack: public DukObject
    {
        public:
            DukContextStack(const std::shared_ptr<DuktapeContext>& ctx, const std::vector<std::string>& stack);
            ~DukContextStack();

        private:
            int _pops = 0;
    };
}