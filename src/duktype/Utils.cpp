#include "Utils.h"
#include "DebugStack.h"
#include "Context.h"
#include <callback.h>

#include <sole/sole.hpp>
#include <scope.h>

#include <sstream>

namespace Duktype
{
    template<typename T>
    v8::Local<T> Utils::createTypedArray(size_t byteLength, char* ptr, int entrySize)
    {
        v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), byteLength);

        auto contents = buffer->GetContents();
        char* charBuf = static_cast<char*>(contents.Data());
        memcpy(charBuf, ptr, byteLength);

        size_t elements = byteLength / entrySize;
        v8::Local<T> result = T::New(buffer, 0, elements);
        return result;
    }

    void Utils::resolveRelativeIndex(int &index, duk_context* ctx)
    {
        if (index < 0)
        {
            index += duk_get_top(ctx);
        }
    }

    v8::Local<v8::Value> Utils::dukToV8(int index, std::shared_ptr<Duktype::Context>& ctxObj)
    {
        duk_context* ctx = ctxObj->getContext();
        resolveRelativeIndex(index, ctx);
        DebugStack d("dukToV8", ctx);
        int type = duk_get_type(ctx, index);
        switch (type)
        {
            case DUK_TYPE_BOOLEAN:
            {
                bool result = duk_get_boolean(ctx, index);
                return Nan::New<v8::Boolean>(result);
            }
            case DUK_TYPE_BUFFER:
            {
                std::cout << "Plain Buffer" << std::endl;
                duk_size_t bufLen;
                char* ptr = static_cast<char*>(duk_get_buffer(ctx, index, &bufLen));
                Nan::MaybeLocal<v8::Object> buf = Nan::CopyBuffer(ptr, static_cast<uint32_t>(bufLen));
                return buf.ToLocalChecked();
            }
            case DUK_TYPE_STRING:
            {
                duk_size_t size = 0;
                const char* str = duk_get_lstring(ctx, index, &size);
                return Nan::New<v8::String>(str, static_cast<int>(size)).ToLocalChecked();
            }
            case DUK_TYPE_NUMBER:
            {
                double num = duk_get_number(ctx, index);
                return Nan::New<v8::Number>(num);
            }
            case DUK_TYPE_OBJECT:
            {
                if (duk_is_buffer_data(ctx, index))
                {
                    duk_size_t bufLen;
                    char* ptr = static_cast<char*>(duk_get_buffer_data(ctx, index, &bufLen));
                    if (instanceOf(index, "Uint8Array", ctx))
                    {
                        return createTypedArray<v8::Uint8Array>(bufLen, ptr, 1);
                    }
                    else if (instanceOf(index, "DataView", ctx))
                    {
                        v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), bufLen);
                        v8::Local<v8::DataView> view = v8::DataView::New(buffer, 0, bufLen);
                        auto contents = buffer->GetContents();
                        char* charBuf = static_cast<char*>(contents.Data());
                        memcpy(charBuf, ptr, bufLen);
                        return view;
                    }
                    else if (instanceOf(index, "ArrayBuffer", ctx))
                    {
                        v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), bufLen);
                        auto contents = buffer->GetContents();
                        char* charBuf = static_cast<char*>(contents.Data());
                        memcpy(charBuf, ptr, bufLen);
                        return buffer;
                    }
                    else if (instanceOf(index, "Uint8ClampedArray", ctx))
                    {
                        return createTypedArray<v8::Uint8ClampedArray>(bufLen, ptr, 1);
                    }
                    else if (instanceOf(index, "Uint16Array", ctx))
                    {
                        return createTypedArray<v8::Uint16Array>(bufLen, ptr, 2);
                    }
                    else if (instanceOf(index, "Uint32Array", ctx))
                    {
                        return createTypedArray<v8::Uint32Array>(bufLen, ptr, 4);
                    }
                    if (instanceOf(index, "Int8Array", ctx))
                    {
                        return createTypedArray<v8::Int8Array>(bufLen, ptr, 1);
                    }
                    else if (instanceOf(index, "Int16Array", ctx))
                    {
                        return createTypedArray<v8::Int16Array>(bufLen, ptr, 2);
                    }
                    else if (instanceOf(index, "Int32Array", ctx))
                    {
                        return createTypedArray<v8::Int32Array>(bufLen, ptr, 4);
                    }
                    else if (instanceOf(index, "Float32Array", ctx))
                    {
                        return createTypedArray<v8::Float32Array>(bufLen, ptr, 4);
                    }
                    else if (instanceOf(index, "Float64Array", ctx))
                    {
                        return createTypedArray<v8::Float64Array>(bufLen, ptr, 8);
                    }
                    else if (instanceOf(index, "Buffer", ctx))
                    {
                        Nan::MaybeLocal<v8::Object> buf = Nan::CopyBuffer(ptr, static_cast<uint32_t>(bufLen));
                        return buf.ToLocalChecked();
                    }
                    else
                    {
                        std::cerr << "Unsupported Buffer Type" << std::endl;
                    }
                }
                else if (duk_is_array(ctx, index))
                {
                    duk_enum(ctx, index, 0);
                    std::vector<v8::Local<v8::Value>> items;
                    while (duk_next(ctx, -1 /*enum_idx*/, 1 /*get_value*/))
                    {
                        items.emplace_back(dukToV8(-1, ctxObj));
                        duk_pop_2(ctx);
                    }
                    duk_pop(ctx);
                    v8::Local<v8::Array> a = Nan::New<v8::Array>(static_cast<unsigned int>(items.size()));
                    for (size_t idx = 0; idx < items.size(); idx++)
                    {
                        Nan::Set(a, static_cast<uint32_t>(idx), items[idx]);
                    }
                    return a;
                }
                else if (duk_is_object(ctx, index))
                {
                    if (instanceOf(index, "Date", ctx))
                    {
                        duk_push_string(ctx, "getTime");
                        duk_call_prop(ctx, index, 0);
                        double num = duk_get_number(ctx, -1);
                        duk_pop(ctx);
                        auto date = Nan::New<v8::Date>(num);
                        return date.ToLocalChecked();
                    }
                    else if (duk_is_function(ctx, index))
                    {
                        // Possibly a callback, let's prepare for that possibility..

                        std::string callbackHandle = "_cb:" + sole::uuid4().str();
                        duk_push_global_stash(ctx);
                        duk_dup(ctx, index);
                        duk_put_prop_string(ctx, -2, callbackHandle.c_str());
                        duk_pop(ctx);


                        v8::Local<v8::Function> cons = Nan::New<v8::Function>(::DukCallback::constructor);
                        v8::Local<v8::Object> obj = Nan::NewInstance(cons).ToLocalChecked();

                        // Copy object properties
                        duk_enum(ctx, index, 0);
                        while (duk_next(ctx, -1 /*enum_idx*/, 1 /*get_value*/))
                        {
                            duk_size_t size = 0;
                            const char* str = duk_get_lstring(ctx, -2, &size);
                            obj->Set(Nan::New<v8::String>(str, static_cast<int>(size)).ToLocalChecked(), dukToV8(-1, ctxObj));
                            duk_pop_2(ctx);
                        }
                        duk_pop(ctx);

                        auto* cb = Nan::ObjectWrap::Unwrap<::DukCallback>(obj);
                        Duktype::Callback* unwrapped = cb->getCallback();
                        unwrapped->setHandle(callbackHandle);
                        unwrapped->setContextObj(ctxObj);
                        return obj;
                    }
                    else
                    {
                        // Plain object - not a function

                        duk_enum(ctx, index, 0);
                        auto objTemplate = Nan::New<v8::Object>();
                        while (duk_next(ctx, -1 /*enum_idx*/, 1 /*get_value*/))
                        {
                            duk_size_t size = 0;
                            const char* str = duk_get_lstring(ctx, -2, &size);
                            objTemplate->Set(Nan::New<v8::String>(str, static_cast<int>(size)).ToLocalChecked(), dukToV8(-1, ctxObj));
                            duk_pop_2(ctx);
                        }
                        duk_pop(ctx);
                        return objTemplate;
                    }
                }
                else
                {
                    std::cerr << "Unsupported type" << std::endl;
                }
                break;
            }
            case DUK_TYPE_UNDEFINED:
            {
                return Nan::Undefined();
            }
            case DUK_TYPE_NULL:
            {
                return Nan::Null();
            }
            default:
                assert(false);
        }
        return Nan::Undefined();
    }

    bool Utils::v8ToDuk(const v8::Local<v8::Value> &val, duk_context* ctx)
    {
        auto isolate = v8::Isolate::GetCurrent();
        if (val->IsBoolean())
        {
            bool value = val->ToBoolean(isolate)->Value();
            duk_push_boolean(ctx, value);
            return true;
        }
        if (val->IsNumber())
        {
            double value = val->ToNumber(isolate)->Value();
            duk_push_number(ctx, value);
            return true;
        }
        else if (val->IsString())
        {
            std::string dukString = *Nan::Utf8String(val);
            duk_push_string(ctx, dukString.c_str());
            return true;
        }
        else if (val->IsUndefined())
        {
            duk_push_undefined(ctx);
            return true;
        }
        else if (val->IsNull())
        {
            duk_push_null(ctx);
            return true;
        }
        else if (val->IsObject())
        {
            v8::Local<v8::Object> obj = val->ToObject(isolate);

            std::string constructorName = *Nan::Utf8String(obj->GetConstructorName());
            if (constructorName == "Array")
            {
                v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(val);
                uint32_t length = array->Length();

                int arrayIdx = duk_push_array(ctx);
                for (uint32_t i = 0; i < length; i++)
                {
                    v8::Local<v8::Value> item = Nan::Get(array, i).ToLocalChecked();
                    if (v8ToDuk(item, ctx))
                    {
                        duk_put_prop_index(ctx, arrayIdx, i);
                    }
                }
                return true;
            }
            else if (constructorName == "Buffer" && val->IsUint8Array())
            {
                size_t length = node::Buffer::Length(val->ToObject(isolate));
                char* buffer = static_cast<char*>(node::Buffer::Data(val->ToObject(isolate)));
                char* ptr = static_cast<char*>(duk_push_buffer(ctx, length, false));
                std::memcpy(ptr, buffer, length);
                duk_push_buffer_object(ctx, -1, 0, length, DUK_BUFOBJ_NODEJS_BUFFER);
                duk_replace(ctx, -2);
                return true;
            }
            else if (val->IsTypedArray())
            {
                int flags = DUK_BUFOBJ_UINT8ARRAY;
                if (val->IsUint8ClampedArray())
                {
                    flags = DUK_BUFOBJ_UINT8CLAMPEDARRAY;
                }
                else if (val->IsUint16Array())
                {
                    flags = DUK_BUFOBJ_UINT16ARRAY;
                }
                else if (val->IsUint32Array())
                {
                    flags = DUK_BUFOBJ_UINT32ARRAY;
                }
                else if (val->IsInt8Array())
                {
                    flags = DUK_BUFOBJ_INT8ARRAY;
                }
                else if (val->IsInt16Array())
                {
                    flags = DUK_BUFOBJ_INT16ARRAY;
                }
                else if (val->IsInt32Array())
                {
                    flags = DUK_BUFOBJ_INT32ARRAY;
                }
                else if (val->IsFloat32Array())
                {
                    flags = DUK_BUFOBJ_FLOAT32ARRAY;
                }
                else if (val->IsFloat64Array())
                {
                    flags = DUK_BUFOBJ_FLOAT64ARRAY;
                }
                else if (val->IsUint8Array())
                {
                    flags = DUK_BUFOBJ_UINT8ARRAY;
                }
                else
                {
                    std::cerr << "Unknown TypedArray" << std::endl;
                    return false;
                }
                v8::Local<v8::TypedArray> array = v8::Local<v8::TypedArray>::Cast(val);
                auto buf = array->Buffer();
                auto contents = buf->GetContents();
                size_t length = contents.ByteLength();
                char* buffer = static_cast<char*>(contents.Data());


                char* ptr = static_cast<char*>(duk_push_fixed_buffer(ctx, length));
                std::memcpy(ptr, buffer, length);

                duk_push_buffer_object(ctx, -1, 0, length, flags);
                duk_replace(ctx, -2);
                return true;
            }
            else if (val->IsArrayBuffer())
            {
                v8::Local<v8::ArrayBuffer> buf = v8::Local<v8::ArrayBuffer>::Cast(val);
                auto contents = buf->GetContents();
                size_t length = contents.ByteLength();
                char* buffer = static_cast<char*>(contents.Data());
                char* ptr = static_cast<char*>(duk_push_fixed_buffer(ctx, length));
                std::memcpy(ptr, buffer, length);
                duk_push_buffer_object(ctx, -1, 0, length, DUK_BUFOBJ_ARRAYBUFFER);
                duk_replace(ctx, -2);
                return true;
            }
            else if (val->IsDataView())
            {
                v8::Local<v8::DataView> view = v8::Local<v8::DataView>::Cast(val);
                v8::Local<v8::ArrayBuffer> buf = view->Buffer();
                auto contents = buf->GetContents();
                size_t length = contents.ByteLength();
                char* buffer = static_cast<char*>(contents.Data());
                char* ptr = static_cast<char*>(duk_push_fixed_buffer(ctx, length));
                std::memcpy(ptr, buffer, length);
                duk_push_buffer_object(ctx, -1, 0, length, DUK_BUFOBJ_DATAVIEW);
                duk_replace(ctx, -2);
                return true;
            }
            else if (constructorName == "Date" && val->IsDate())
            {
                v8::Local<v8::Date> date = v8::Local<v8::Date>::Cast(val);
                double dt = (date->NumberValue(isolate->GetCurrentContext())).ToChecked();
                auto t = static_cast<std::int64_t>(dt);
                std::stringstream s;
                s << t;
                std::string str = s.str();

                // There has to be a better way than this..
                duk_eval_string(ctx, std::string("new Date(" + str + ")").c_str());
                return true;
            }
            else
            {
                if (constructorName != "Object")
                {
                    std::cout << "Unsupported object type: " << constructorName << std::endl;
                }
                auto props = Nan::GetPropertyNames(obj).ToLocalChecked();
                int objIdx = duk_push_object(ctx);
                for (uint32_t i = 0; i < props->Length(); i++)
                {
                    v8::Local<v8::String> localKey = props->Get(i)->ToString(v8::Isolate::GetCurrent());

                    std::string key = *Nan::Utf8String(localKey);

                    if (Nan::HasOwnProperty(obj, localKey).FromJust())
                    {
                        auto maybeLocalVal = Nan::Get(obj, localKey);
                        auto localVal = maybeLocalVal.ToLocalChecked();
                        v8ToDuk(localVal, ctx);
                        duk_put_prop_string(ctx, objIdx, key.c_str());
                    }
                }
            }
        }
        else
        {
            std::cout << "Unknown type!" << std::endl;
            // Unknown type
            assert(false);
        }
        return false;
    }

    bool Utils::instanceOf(int index, const std::string &type, duk_context* ctx)
    {
        resolveRelativeIndex(index, ctx);
        duk_get_global_string(ctx, type.c_str());
        bool result = duk_instanceof(ctx, index, -1);
        duk_pop(ctx);
        return result;
    }
}
