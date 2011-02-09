// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "clang_driver.h"

using namespace Intel::OpenCL::ClangFE;

void __attribute__ ((constructor)) dll_init(void);
void __attribute__ ((destructor)) dll_fini(void);

void dll_init(void)
{
    InitClangDriver();
}

void dll_fini(void)
{
    CloseClangDriver();
}

