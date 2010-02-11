// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "llvm_backend.h"

using namespace Intel::OpenCL::DeviceBackend;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		if ( NULL == LLVMBackend::GetInstance() )
		{
			return FALSE;
		}
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (NULL == lpReserved) //Detach due to FreeLibrary
		{
			LLVMBackend::GetInstance()->Release();
		}
		//Else, either loading failed or process is terminating, do nothing and let OS reclaim resources
		break;
	}
	return TRUE;
}

