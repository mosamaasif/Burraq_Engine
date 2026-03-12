#pragma once

#include "Platform/Platform.h"
#include "Utilities/Types.h"
#include "Logger/Log.h"

#if defined(_MSC_VER)
    #define BRQ_DEBUG_BREAK() __debugbreak()
#elif defined(__clang__) || defined(__GNUC__)
    #define BRQ_DEBUG_BREAK() __builtin_trap()
#else
    #define BRQ_DEBUG_BREAK() ((void)0)
#endif

#ifdef BRQ_DEBUG
    #define BRQ_ASSERT_DEFAULT(x)               BRQ_FATAL("Assertion Failed: {}  Line: {}, File: {}, Function: {}", #x, __LINE__, __FILE__, __FUNCTION__);
    #define BRQ_CORE_ASSERT_DEFAULT(x)          BRQ_CORE_FATAL("Assertion Failed: {}  Line: {}, File: {}, Function: {}", #x, __LINE__, __FILE__, __FUNCTION__);

    #define BRQ_ASSERT(x)                       { if(!(x)) { BRQ_ASSERT_DEFAULT(x); BRQ_DEBUG_BREAK(); } }
    #define BRQ_CORE_ASSERT(x)                  { if(!(x)) { BRQ_CORE_ASSERT_DEFAULT(x); BRQ_DEBUG_BREAK(); } }
#else
    #define BRQ_ASSERT(x)
    #define BRQ_CORE_ASSERT(x)
#endif

#define BIT(x)  (1 << x)

#if defined(_MSC_VER)
    #define BRQ_ALIGN(x) __declspec(align(x))
#else
    #define BRQ_ALIGN(x) alignas(x)
#endif

#define BRQ_BIND_EVENT_FN(function) [this](auto&&... args) -> decltype(auto) { return this->function(std::forward<decltype(args)>(args)...); }