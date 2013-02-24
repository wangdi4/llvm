
#pragma once

#ifdef DEBUG
    #define NATIVE_PRINTF(...)   { printf( "SINK: " __VA_ARGS__); fflush(0); }
#else
    #define NATIVE_PRINTF(...)
#endif

