// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "framework_proxy.h"

#pragma comment(lib, "cl_sys_utils.lib")
#pragma comment(lib, "cl_logger.lib")
#if defined(_M_X64)
#ifdef BUILD_EXPERIMENTAL_21
    #pragma comment(lib, "task_executor64_2_1.lib")
#else //BUILD_EXPERIMENTAL_21
    #pragma comment(lib, "task_executor64.lib")
#endif //BUILD_EXPERIMENTAL_21
#else
#ifdef BUILD_EXPERIMENTAL_21
    #pragma comment(lib, "task_executor32_2_1.lib")
#else //BUILD_EXPERIMENTAL_21
    #pragma comment(lib, "task_executor32.lib")
#endif //BUILD_EXPERIMENTAL_21
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#ifdef _DEBUG  // this is needed to initialize allocated objects DB, which is maintained in only in debug
        InitSharedPtrs();
#endif
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		// release the framework proxy object 
		Intel::OpenCL::Framework::FrameworkProxy::Destroy();
#ifdef _DEBUG
        FiniSharedPts();
#endif
		break;
	}
	return TRUE;
}
