#pragma once

#include <memory>
#include <string>

namespace Duktype
{
    class Context;
    struct CallbackWeakRef
    {
        std::string callbackID;
        std::shared_ptr<Context> ctx;
    };
}