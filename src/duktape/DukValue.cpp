#include "DukValue.h"
#include "DukEnum.h"
#include "DukGlobalStash.h"
#include "DuktapeContext.h"
#include <duktype/Context.h>
#include <duktype/Promise.h>
#include <duktype/PromiseData.h>
#include <callback.h>
#include <iostream>
#include <sole/sole.hpp>

namespace Duktape
{
    DukValue::DukValue(const std::shared_ptr<DuktapeContext>& ctx, int indexOffset, bool resolved)
        : _ctx(ctx)
        , _stackIndex(duk_get_top_index(ctx->getContext()) + indexOffset)
        , _resolved(resolved)
        , DukObject(ctx)
    {
        static int valIndex = 0;
        _valIndex = valIndex++;
        _objectIndex = _stackIndex;
    }

    std::string DukValue::getString()
    {
        return std::string(duk_safe_to_string(_ctx->getContext(), _stackIndex));
    }

    double DukValue::getNumber()
    {
        return duk_to_number(_ctx->getContext(), _stackIndex);
    }

    uint32_t DukValue::getIndex()
    {
        return _stackIndex;
    }

    void DukValue::fromV8(const std::shared_ptr<Duktype::Context>& context, v8::Local<v8::Value> val)
    {
        auto ctxObj = context->getDuktape();
        auto ctx = ctxObj->getContext();
        auto isolate = v8::Isolate::GetCurrent();
        if (val->IsBoolean())
        {
            bool value = val->ToBoolean(isolate)->Value();
            DukValue::newBoolean(ctxObj, value);
            return;
        }
        else if (val->IsNumber())
        {
            double value = val->ToNumber(isolate)->Value();
            DukValue::newNumber(ctxObj, value);
            return;
        }
        else if (val->IsString())
        {
            std::string value = *Nan::Utf8String(val);
            DukValue::newString(ctxObj, value);
            return;
        }
        else if (val->IsUndefined())
        {
            DukValue::newUndefined(ctxObj);
            return;
        }
        else if (val->IsNull())
        {
            DukValue::newNull(ctxObj);
            return;
        }
        else if (val->IsObject())
        {
            v8::Local<v8::Object> obj = val->ToObject(isolate);

            std::string constructorName = *Nan::Utf8String(obj->GetConstructorName());
            if (constructorName == "Array")
            {
                v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(val);
                uint32_t length = array->Length();

                DukValue::newArray(ctxObj);
                DukValue arr(ctxObj);
                for (uint32_t i = 0; i < length; i++)
                {
                    v8::Local<v8::Value> item = Nan::Get(array, i).ToLocalChecked();
                    DukValue::fromV8(context, item);
                    DukValue it(ctxObj);
                    arr.setPropertyIndex(i, it);
                }
                arr.resolved();
                return;
            }
            else if (constructorName == "Buffer" && val->IsUint8Array())
            {
                size_t length = node::Buffer::Length(val->ToObject(isolate));
                char* buffer = static_cast<char*>(node::Buffer::Data(val->ToObject(isolate)));
                DukValue::newBuffer(ctxObj, buffer, length, false);
                DukValue buf(ctxObj);
                buf.convertBufferToBufferObject(length, DUK_BUFOBJ_NODEJS_BUFFER);
                buf.resolved();
                return;
            }
            else if (val->IsTypedArray())
            {
                int flags = -1;
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

                if (flags > -1)
                {
                    v8::Local<v8::TypedArray> array = v8::Local<v8::TypedArray>::Cast(val);
                    auto buf = array->Buffer();
                    auto contents = buf->GetContents();
                    size_t length = contents.ByteLength();
                    char* buffer = static_cast<char*>(contents.Data());

                    DukValue::newBuffer(ctxObj, buffer, length, false);
                    DukValue dukBuf(ctxObj);
                    dukBuf.convertBufferToBufferObject(length, flags);
                    dukBuf.resolved();
                    return;
                }
            }
            else if (val->IsArrayBuffer())
            {
                v8::Local<v8::ArrayBuffer> buf = v8::Local<v8::ArrayBuffer>::Cast(val);
                auto contents = buf->GetContents();
                size_t length = contents.ByteLength();
                char* buffer = static_cast<char*>(contents.Data());

                DukValue::newBuffer(ctxObj, buffer, length, false);
                DukValue dukBuf(ctxObj);
                dukBuf.convertBufferToBufferObject(length, DUK_BUFOBJ_ARRAYBUFFER);
                dukBuf.resolved();
                return;
            }
            else if (val->IsDataView())
            {
                v8::Local<v8::DataView> view = v8::Local<v8::DataView>::Cast(val);
                v8::Local<v8::ArrayBuffer> buf = view->Buffer();
                auto contents = buf->GetContents();
                size_t length = contents.ByteLength();
                char* buffer = static_cast<char*>(contents.Data());
                DukValue::newBuffer(ctxObj, buffer, length, false);
                DukValue dukBuf(ctxObj);
                dukBuf.convertBufferToBufferObject(length, DUK_BUFOBJ_DATAVIEW);
                dukBuf.resolved();
                return;
            }
            else if (constructorName == "Error" && val->IsNativeError())
            {
                v8::Local<v8::Message> msg = v8::Exception::CreateMessage(isolate, val);
                std::string errorMessage = *Nan::Utf8String(msg->Get());
                duk_push_error_object(ctx, DUK_ERR_ERROR, errorMessage.c_str());
                return;
            }
            else if (constructorName == "Date" && val->IsDate())
            {
                v8::Local<v8::Date> date = v8::Local<v8::Date>::Cast(val);
                double dt = (date->NumberValue(isolate->GetCurrentContext())).ToChecked();
                DukValue d = ctxObj->getGlobalString("Date");
                DukValue::newNumber(ctxObj, dt);
                d.callNew(1);
                d.resolved();
                return;
            }
            else if (constructorName == "Promise")
            {
                std::string promiseHandle = "_promise:" + sole::uuid4().str();

                (void) duk_get_global_string(ctx, "Promise");

                // Push handler function to get resolvers
                duk_push_c_function(ctx, &Duktype::Promise::storePromiseResolvers, DUK_VARARGS);

                duk_push_string(ctx, promiseHandle.c_str());
                duk_put_prop_string(ctx, -2, "__promiseHandle");

                if (duk_pnew(ctx,1) != DUK_EXEC_SUCCESS)
                {
                    std::cerr << "A promise object was encountered but promises are not enabled in this context." << std::endl;
                    DukValue::newUndefined(ctxObj);
                    return;
                }

                duk_push_c_function(ctx, &Duktype::Promise::dukPromiseFinalised, 1 /*nargs*/);
                duk_push_string(ctx, promiseHandle.c_str());
                duk_put_prop_string(ctx, -2, "__promiseHandle");
                duk_push_pointer(ctx, static_cast<void*>(context.get()));
                duk_put_prop_string(ctx, -2, "__context");
                duk_set_finalizer(ctx, -2);

                v8::Local<v8::Promise> promise = v8::Local<v8::Promise>::Cast(val);

                // We need to:
                //
                // 1. Create a new function for .then() and store it as a Nan::Persistent, bound to our ID
                // 2. Create a new function for .catch() and store it as a Nan::Persistent, bound to our ID
                // 3. Create a duktape promise
                // 4. Store the above three somewhere with a uniqueID
                // 5. When .then is received, call duktape promise.then
                // 6. When .catch is received, call duktape promise.catch

                std::shared_ptr<Duktype::Promise> v8Promise = std::make_shared<Duktype::Promise>();
                v8Promise->promiseID = promiseHandle;

                auto* d = new Duktype::PromiseData();
                d->promiseHandle = v8Promise->promiseID;
                d->context = context;

                v8::MaybeLocal<v8::Function> thenFunc = Nan::New<v8::Function>(&Duktype::Promise::handleThen, Nan::New<v8::External>(d));
                v8::MaybeLocal<v8::Function> catchFunc = Nan::New<v8::Function>(&Duktype::Promise::handleCatch, Nan::New<v8::External>(d));

                v8::Local<v8::Function> thenFuncChecked = thenFunc.ToLocalChecked();
                v8::Local<v8::Function> catchFuncChecked = catchFunc.ToLocalChecked();

                Nan::Set(catchFuncChecked, Nan::New("promiseHandle").ToLocalChecked(), Nan::New(v8Promise->promiseID.c_str()).ToLocalChecked());

                promise->Then(isolate->GetCurrentContext(), thenFuncChecked, catchFuncChecked);
                v8Promise->promise.Reset(promise);
                context->registerPromise(v8Promise->promiseID, v8Promise);
                return;
            }
            else
            {
                if (constructorName != "Object")
                {
                    std::cout << "Unsupported object type: " << constructorName << std::endl;
                }
                auto props = Nan::GetPropertyNames(obj).ToLocalChecked();
                DukValue::newObject(ctxObj);
                DukValue dukObj(ctxObj);
                for (uint32_t i = 0; i < props->Length(); i++)
                {
                    v8::Local<v8::String> localKey = props->Get(i)->ToString(v8::Isolate::GetCurrent());

                    std::string key = *Nan::Utf8String(localKey);

                    if (Nan::HasOwnProperty(obj, localKey).FromJust())
                    {
                        auto maybeLocalVal = Nan::Get(obj, localKey);
                        auto localVal = maybeLocalVal.ToLocalChecked();
                        DukValue::fromV8(context, localVal);
                        DukValue nVal(ctxObj);
                        dukObj.setProperty(key, nVal);
                    }
                }
                dukObj.resolved();
                return;
            }
        }
        else
        {
            // Unknown type
            assert(false);
        }
        duk_push_undefined(ctx);
    }

    void DukValue::callNew(int params)
    {
        duk_new(_ctx->getContext(), params);
    }

    void DukValue::newArray(const std::shared_ptr<DuktapeContext>& ctx)
    {
        duk_push_array(ctx->getContext());
    }

    void DukValue::resolved()
    {
        if (_resolved)
        {
            throw std::runtime_error("Resolved called on DukValue which has already been resolved");
        }
        _resolved = true;
    }

    void DukValue::dupe()
    {
        duk_dup(_ctx->getContext(), _stackIndex);
    }

    void DukValue::newBuffer(const std::shared_ptr<DuktapeContext>& ctx, char* buffer, size_t length, bool dynamic)
    {
        char* ptr = static_cast<char*>(duk_push_buffer(ctx->getContext(), length, false));
        std::memcpy(ptr, buffer, length);
    }

    void DukValue::convertBufferToBufferObject(size_t length, int type)
    {
        duk_push_buffer_object(_ctx->getContext(), getIndex(), 0, length, type);
        duk_replace(_ctx->getContext(), getIndex());
    }

    void DukValue::newString(const std::shared_ptr<DuktapeContext>& ctx, const std::string& str)
    {
        duk_push_string(ctx->getContext(), str.c_str());
    }

    void DukValue::newObject(const std::shared_ptr<DuktapeContext>& ctx)
    {
        duk_push_object(ctx->getContext());
    }

    DukValue DukValue::getValue(const std::shared_ptr<DuktapeContext>& ctx, int index)
    {
        if (index < 0)
        {
            index += duk_get_top(ctx->getContext());
        }
        return DukValue(ctx, index - duk_get_top_index(ctx->getContext()), true);
    }

    void DukValue::newBoolean(const std::shared_ptr<DuktapeContext>& ctx, bool value)
    {
        duk_push_boolean(ctx->getContext(), value);
    }

    void DukValue::newNumber(const std::shared_ptr<DuktapeContext>& ctx, double value)
    {
        duk_push_number(ctx->getContext(), value);
    }

    void DukValue::newUndefined(const std::shared_ptr<DuktapeContext>& ctx)
    {
        duk_push_undefined(ctx->getContext());
    }

    void DukValue::newNull(const std::shared_ptr<DuktapeContext>& ctx)
    {
        duk_push_null(ctx->getContext());
    }

    v8::Local<v8::Value> DukValue::toV8(const std::shared_ptr<Duktype::Context>& context)
    {
        duk_context* ctx = _ctx->getContext();
        int type = duk_get_type(ctx, _stackIndex);
        switch (type)
        {
            case DUK_TYPE_NULL:
            {
                return Nan::Null();
            }
            case DUK_TYPE_BOOLEAN:
            {
                bool result = duk_get_boolean(ctx, _stackIndex);
                return Nan::New<v8::Boolean>(result);
            }
            case DUK_TYPE_BUFFER:
            {
                duk_size_t bufLen;
                char* ptr = static_cast<char*>(duk_get_buffer(ctx, _stackIndex, &bufLen));
                Nan::MaybeLocal<v8::Object> buf = Nan::CopyBuffer(ptr, static_cast<uint32_t>(bufLen));
                return buf.ToLocalChecked();
            }
            case DUK_TYPE_STRING:
            {
                duk_size_t size = 0;
                const char* str = duk_get_lstring(ctx, _stackIndex, &size);
                return Nan::New<v8::String>(str, static_cast<int>(size)).ToLocalChecked();
            }
            case DUK_TYPE_NUMBER:
            {
                double num = duk_get_number(ctx, _stackIndex);
                return Nan::New<v8::Number>(num);
            }
            case DUK_TYPE_OBJECT:
            {
                if (duk_is_buffer_data(ctx, _stackIndex))
                {
                    duk_size_t bufLen;
                    char* ptr = static_cast<char*>(duk_get_buffer_data(ctx, _stackIndex, &bufLen));
                    if (instanceOf("DataView"))
                    {
                        v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), bufLen);
                        v8::Local<v8::DataView> view = v8::DataView::New(buffer, 0, bufLen);
                        auto contents = buffer->GetContents();
                        char* charBuf = static_cast<char*>(contents.Data());
                        memcpy(charBuf, ptr, bufLen);
                        return view;
                    }
                    else if (instanceOf("ArrayBuffer"))
                    {
                        v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), bufLen);
                        auto contents = buffer->GetContents();
                        char* charBuf = static_cast<char*>(contents.Data());
                        memcpy(charBuf, ptr, bufLen);
                        return buffer;
                    }
                    else if (instanceOf("Buffer"))
                    {
                        auto buf = Nan::CopyBuffer(ptr, static_cast<uint32_t>(bufLen)).ToLocalChecked();
                        return buf;
                    }
                    else if (instanceOf("Uint8Array"))
                    {
                        return createTypedArray<v8::Uint8Array>(bufLen, ptr, 1);
                    }
                    else if (instanceOf("Uint8ClampedArray"))
                    {
                        return createTypedArray<v8::Uint8ClampedArray>(bufLen, ptr, 1);
                    }
                    else if (instanceOf("Uint16Array"))
                    {
                        return createTypedArray<v8::Uint16Array>(bufLen, ptr, 2);
                    }
                    else if (instanceOf("Uint32Array"))
                    {
                        return createTypedArray<v8::Uint32Array>(bufLen, ptr, 4);
                    }
                    if (instanceOf("Int8Array"))
                    {
                        return createTypedArray<v8::Int8Array>(bufLen, ptr, 1);
                    }
                    else if (instanceOf("Int16Array"))
                    {
                        return createTypedArray<v8::Int16Array>(bufLen, ptr, 2);
                    }
                    else if (instanceOf("Int32Array"))
                    {
                        return createTypedArray<v8::Int32Array>(bufLen, ptr, 4);
                    }
                    else if (instanceOf("Float32Array"))
                    {
                        return createTypedArray<v8::Float32Array>(bufLen, ptr, 4);
                    }
                    else if (instanceOf("Float64Array"))
                    {
                        return createTypedArray<v8::Float64Array>(bufLen, ptr, 8);
                    }
                    else
                    {
                        std::cerr << "Unsupported Buffer Type" << std::endl;
                    }
                }
                else if (duk_is_array(ctx, _stackIndex))
                {
                    DukEnum e(_ctx, _stackIndex);
                    std::vector<v8::Local<v8::Value>> items;
                    while (e.next(true, [&](DukValue& key, DukValue& value)
                    {
                        items.emplace_back(value.toV8(context));
                    }));

                    v8::Local<v8::Array> a = Nan::New<v8::Array>(static_cast<unsigned int>(items.size()));
                    for (size_t idx = 0; idx < items.size(); idx++)
                    {
                        Nan::Set(a, static_cast<uint32_t>(idx), items[idx]);
                    }
                    return a;
                }
                else if (duk_is_object(ctx, _stackIndex))
                {
                    if (instanceOf("Date"))
                    {
                        DukValue result = callMethod(context, "getTime");

                        auto date = Nan::New<v8::Date>(result.getNumber());
                        return date.ToLocalChecked();
                    }
                    else if (instanceOf("Error"))
                    {
                        DukValue prop = getProperty("message");
                        return Nan::Error(prop.getString().c_str());
                    }
                    else if (instanceOf("Promise"))
                    {
                        // We need to:
                        //
                        // 1. callMethod "then" and push a handler function with a bound promiseID
                        std::string promiseHandle = "_promiseDuk:" + sole::uuid4().str();

                        {
                            DukValue::newString(_ctx, "then");
                            DukValue funcName(_ctx);
                            duk_push_c_function(ctx, &Duktype::Promise::handleDukThen, DUK_VARARGS);
                            int funcIndex = duk_get_top_index(ctx);

                            duk_push_c_function(ctx, &Duktype::Promise::dukThenFinalised, 1);
                             duk_push_string(ctx, promiseHandle.c_str());
                            duk_put_prop_string(ctx, -2, "__promiseHandle");
                            duk_push_pointer(ctx, context.get());
                            duk_put_prop_string(ctx, -2, "__context");
                            duk_set_finalizer(ctx, funcIndex);


                            duk_push_string(ctx, promiseHandle.c_str());
                            duk_put_prop_string(ctx, funcIndex, "__promiseHandle");
                            duk_push_pointer(ctx, context.get());
                            duk_put_prop_string(ctx, funcIndex, "__context");
                            callMethod(context, 1);

                            // 2. callMethod "catch" on the returned object and push a handler function with a bound promiseID
                            {
                                DukValue::newString(_ctx, "catch");
                                DukValue catchFuncName(_ctx);
                                duk_push_c_function(ctx, &Duktype::Promise::handleDukCatch, DUK_VARARGS);

                                duk_push_c_function(ctx, &Duktype::Promise::dukCatchFinalised, 1 /*nargs*/);
                                duk_push_string(ctx, promiseHandle.c_str());
                                duk_put_prop_string(ctx, -2, "__promiseHandle");
                                duk_push_pointer(ctx, context.get());
                                duk_put_prop_string(ctx, -2, "__context");
                                duk_set_finalizer(ctx, -2);

                                duk_push_string(ctx, promiseHandle.c_str());
                                duk_put_prop_string(ctx, -2, "__promiseHandle");

                                duk_push_pointer(ctx, context.get());
                                duk_put_prop_string(ctx, -2, "__context");

                                callMethod(context, 1);
                            }
                        }

                        // 3. create a v8 promise, store it and return it
                        auto resolver = v8::Promise::Resolver::New(v8::Isolate::GetCurrent()->GetCurrentContext()).ToLocalChecked();
                        auto promise = resolver->GetPromise();
                        auto* promiseResolver = new Nan::Persistent<v8::Promise::Resolver>(resolver);
                        context->registerDukPromise(promiseHandle, promiseResolver);
                        return promise;
                    }
                    else if (duk_is_function(ctx, _stackIndex))
                    {
                        // Possibly a callback, let's prepare for that possibility..
                        std::string callbackHandle = "_cb:" + sole::uuid4().str();

                        {
                            DukGlobalStash globalStash(_ctx);
                            {
                                dupe();
                                DukValue v(_ctx);
                                globalStash.setProperty(callbackHandle, v);
                            }
                        }
                        v8::Local<v8::Function> cons = context->getCallbackConstructor();
                        v8::Local<v8::Object> obj = Nan::NewInstance(cons).ToLocalChecked();

                        DukEnum e(_ctx, _stackIndex);
                        while (e.next(true, [&](DukValue& key, DukValue& value)
                        {
                            std::string str = key.getString();
                            obj->Set(Nan::New<v8::String>(str.c_str(), static_cast<int>(str.length())).ToLocalChecked(), value.toV8(context));
                        }));

                        auto* cb = Nan::ObjectWrap::Unwrap<::DukCallback>(obj);
                        Duktype::Callback* unwrapped = cb->getCallback();
                        unwrapped->setHandle(callbackHandle);
                        unwrapped->setContext(context);
                        context->addCallback(callbackHandle);
                        return obj;
                    }
                    else
                    {
                        // Plain object - not a function
                        auto objTemplate = Nan::New<v8::Object>();

                        DukEnum e(_ctx, _stackIndex);
                        while (e.next(true, [&](DukValue& key, DukValue& value)
                        {
                            std::string str = key.getString();
                            objTemplate->Set(Nan::New<v8::String>(str.c_str(), static_cast<int>(str.length())).ToLocalChecked(), value.toV8(context));
                        }));
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
            default:
                assert(false);
        }
        return Nan::Undefined();
    }

    template<typename T>
    v8::Local<T> DukValue::createTypedArray(size_t byteLength, char* ptr, int entrySize)
    {
        v8::Local<v8::ArrayBuffer> buffer = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), byteLength);

        auto contents = buffer->GetContents();
        char* charBuf = static_cast<char*>(contents.Data());
        memcpy(charBuf, ptr, byteLength);

        size_t elements = byteLength / entrySize;
        v8::Local<T> result = T::New(buffer, 0, elements);
        return result;
    }

    bool DukValue::isCallable()
    {
        return duk_is_callable(_ctx->getContext(), _stackIndex);
    }

    bool DukValue::isObject()
    {
        return duk_is_object(_ctx->getContext(), _stackIndex);
    }

    bool DukValue::isUndefined()
    {
        return duk_is_undefined(_ctx->getContext(), _stackIndex);
    }

    DukValue::~DukValue()
    {
        if (!_ctx && _stackIndex < 0)
        {
            return;
        }
        if (!_resolved)
        {
            if (duk_get_top_index(_ctx->getContext()) != _stackIndex)
            {
                std::cerr << "STACK CORRUPTION VAL " << _valIndex << " - Constructed top does not match destructed top (" << _stackIndex << " -> " << duk_get_top_index(_ctx->getContext()) << ")" << std::endl;
            }
            duk_pop(_ctx->getContext());
        }
    }
};