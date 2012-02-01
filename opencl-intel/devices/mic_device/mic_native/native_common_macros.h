
#pragma once

#define NATIVE_PRINTF(...)   { printf( "SINK: " __VA_ARGS__); fflush(0); }
#define hw_pause()           { asm volatile ("pause"); }

#define IS_ALIGN(address, alignment) ((((size_t)address) & ((size_t)(alignment - 1))) == 0)

inline void _CPUID(unsigned int func, unsigned int* eax, unsigned int* ebx, unsigned int* ecx, unsigned int* edx)
{
		__asm__ __volatile__ ("cpuid"
								: "=a" (*eax),
								"=b" (*ebx),
								"=c" (*ecx),
								"=d" (*edx)
								: "a" (func));
}

inline unsigned int hw_cpu_idx()
{
	unsigned int eax, ebx, ecx, edx;
	_CPUID( 1, &eax, &ebx, &ecx, &edx );
	return ebx >> 24;
}
