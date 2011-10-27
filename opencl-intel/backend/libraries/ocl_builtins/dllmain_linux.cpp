// dllmain.cpp : Defines the entry point for the DLL application.
#include <cassert>
#include "stdafx.h"

__attribute__ ((constructor)) static void dll_init(void);
__attribute__ ((destructor)) static void dll_fini(void);

void dll_init(void)
{
}

void dll_fini(void)
{
}


extern "C" int clRTLibInitLibrary()
{
	return 0;
}