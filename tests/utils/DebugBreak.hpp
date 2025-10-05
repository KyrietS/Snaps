#pragma once

#ifdef _MSC_VER
    #include <intrin.h>
    #define DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
    #include <csignal>
    #define DEBUG_BREAK() std::raise(SIGTRAP)
#else
    #include <cstdlib>
    #define DEBUG_BREAK() std::abort()
#endif