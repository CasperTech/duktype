cmake_minimum_required(VERSION 3.12.0)
project(duktype)

set(CMAKE_CXX_STANDARD 17)

include(NodeJS.cmake)
nodejs_init()

add_library(duktape duktape/src/duktape.c duktape/src/duktape.h duktape/src/duk_config.h)
set_property(TARGET duktape PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(duktape INTERFACE SYSTEM duktape/src)

add_nodejs_module(${PROJECT_NAME}
        main.cpp
        src/context.cpp
        src/context.h
        src/asynccontext.cpp
        src/asynccontext.h
        src/objectscope.cpp
        src/objectscope.h
        src/asyncobjectscope.cpp
        src/asyncobjectscope.h
        src/callback.h
        src/callback.cpp
        src/asynccallback.h
        src/asynccallback.cpp

        src/duktape/DuktapeContext.h
        src/duktape/DuktapeContext.cpp
        src/duktape/DukValue.h
        src/duktape/DukValue.cpp
        src/duktape/DukEnum.h
        src/duktape/DukEnum.cpp
        src/duktape/DukObject.cpp
        src/duktape/DukObject.h
        src/duktape/DukGlobalStash.cpp
        src/duktape/DukGlobalStash.h
        src/duktape/DukGlobalObject.cpp
        src/duktape/DukGlobalObject.h
        src/duktape/DukContextStack.cpp
        src/duktape/DukContextStack.h

        src/duktype/ObjectHandle.h
        src/duktype/ObjectHandle.cpp
        src/duktype/Callback.cpp
        src/duktype/Callback.h
        src/duktype/AsyncCallback.cpp
        src/duktype/AsyncCallback.h
        src/duktype/Context.cpp
        src/duktype/Context.h
        src/duktype/AsyncContext.cpp
        src/duktype/AsyncContext.h
        src/duktype/ObjectScope.cpp
        src/duktype/ObjectScope.h
        src/duktype/AsyncObjectScope.cpp
        src/duktype/AsyncObjectScope.h
        src/duktype/AsyncJob.h
        src/duktype/AsyncJob.cpp
        src/duktype/Promise.h
        src/duktype/Promise.cpp
        src/duktype/PromiseData.cpp
        src/duktype/AsyncJobScheduler.h
        src/duktype/AsyncJobScheduler.cpp
        src/duktype/AsyncNodeScheduler.h
        src/duktype/AsyncNodeScheduler.cpp
        src/duktype/ResourceManager.cpp
        src/duktype/ResourceManager.h

        src/uuid/uuid.h
        src/uuid/uuid.cpp
)

target_link_libraries(${PROJECT_NAME} duktape)
target_include_directories(${PROJECT_NAME} PRIVATE src)
