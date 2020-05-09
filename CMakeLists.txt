cmake_minimum_required(VERSION 3.12.0)
set(CMAKE_CXX_STANDARD 17)

project(duktype)
include(NodeJS.cmake)
nodejs_init()

add_library(duktape duktape/src/duktape.c duktape/src/duktape.h duktape/src/duk_config.h)
set_property(TARGET duktape PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(duktape INTERFACE SYSTEM duktape/src)

add_nodejs_module(${PROJECT_NAME}
        main.cpp
        src/context.cpp
        src/context.h
        src/duktype/Context.h
        src/duktype/Context.cpp
        src/duktype/Scope.cpp
        src/duktype/Scope.h
        src/scope.cpp
        src/scope.h
        src/duktype/CallbackData.h
        src/duktype/DebugStack.h
        src/duktype/ContextStack.h
        src/duktype/Utils.cpp
        src/duktype/Utils.h
        src/duktype/ExecutionContext.h
        src/duktype/ExecutionContext.cpp
)
target_link_libraries(${PROJECT_NAME} duktape)
