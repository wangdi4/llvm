// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "clang_driver.h"

using namespace Intel::OpenCL::ClangFE;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		//Else, either loading failed or process is terminating, do nothing and let OS reclaim resources
		break;
	}
	return TRUE;
}

