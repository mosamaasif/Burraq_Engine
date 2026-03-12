#pragma once

#if defined(_WIN64)
    #ifndef BRQ_PLATFORM_WINDOWS
        #define BRQ_PLATFORM_WINDOWS
    #endif
#elif defined(_WIN32)
    #error "Burraq Engine only supports 64-bit Windows!"
#elif defined(__APPLE__)
    #define BRQ_PLATFORM_MACOS
#elif defined(__linux__)
    #define BRQ_PLATFORM_LINUX
#else
    #error "Unsupported platform!"
#endif



