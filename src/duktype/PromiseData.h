#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <mutex>

namespace Duktype
{
    class Context;
    struct PromiseData
    {
        std::string promiseHandle;
        std::weak_ptr<Context> context;
    };


}