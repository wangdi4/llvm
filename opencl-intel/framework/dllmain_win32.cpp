// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "framework_proxy.h"

#pragma comment(lib, "cl_sys_utils.lib")
#pragma comment(lib, "cl_logger.lib")
#pragma comment(lib, "task_executor.lib")

#if defined(USE_GPA)
#pragma comment(lib, "gpasdk_d.lib")
#endif
extern char clFRAMEWORK_CFG_PATH[];

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
		// release the framework proxy object 
		Intel::OpenCL::Framework::FrameworkProxy::Destroy();		
		break;
	}
	return TRUE;
}
