#pragma once

// Disable some stupid warnings in Visual Studio which nan triggers
#pragma warning( push )
#pragma warning( disable : 4275 4251 )
#include <nan.h>
#pragma warning( pop )