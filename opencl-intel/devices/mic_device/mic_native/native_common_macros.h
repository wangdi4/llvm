
#pragma once

#define NATIVE_PRINTF(...)   { printf( "SINK: " __VA_ARGS__); fflush(0); }
#define hw_pause()           { asm volatile ("pause"); }

