#include "AsyncObjectScope.h"
#include "AsyncContext.h"

namespace Duktype
{
    void AsyncObjectScope::createObjectAsync(const Nan::FunctionCallbackInfo<v8::Value> &info)
    {
        if (info.Length() > 0 && info[0]->IsString())
        {
            std::string objNameStr = *Nan::Utf8String(info[0]);
            std::static_pointer_cast<AsyncContext>(_ctx)->createObjectAsync(_handle, info);
        }
        else
        {
            Nan::ThrowError("Invalid or wrong number of arguments");
        }
    }
}