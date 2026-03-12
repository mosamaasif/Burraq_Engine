#pragma once

#ifdef _WIN64
    #define BRQ_PLATFORM_WINDOWS
#elif defined(_WIN32)
    #error "Burraq Engine only supports 64-bit Windows!"
#else
    #error "Burraq Engine only supports Windows!"
#endif



