// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

void __attribute__ ((constructor)) dll_init(void);
void __attribute__ ((destructor)) dll_fini(void);

void CommonClangInitialize();
void CommonClangTerminate();

void dll_init(void)
{
    CommonClangInitialize();
}

void dll_fini(void)
{
    CommonClangTerminate();
}

