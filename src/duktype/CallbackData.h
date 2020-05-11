#pragma once

#include <initnan.h>
#include <vector>

struct CallbackData
{
    Nan::Callback * callback;
    std::vector<Nan::CopyablePersistentTraits<v8::Value>::CopyablePersistent> params;
};