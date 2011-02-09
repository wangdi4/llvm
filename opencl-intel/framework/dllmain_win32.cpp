// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "framework_proxy.h"

#pragma comment(lib, "cl_sys_utils.lib")
#pragma comment(lib, "cl_logger.lib")
#pragma comment(lib, "task_executor.lib")

#if defined(USE_TASKALYZER)
#ifdef _DEBUG
#pragma comment(lib, "tal_dd.lib")
#else
#pragma comment(lib, "tal_dr.lib")
#endif
#endif
extern char clFRAMEWORK_CFG_PATH[];

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	char tBuff[MAX_PATH], *ptCutBuff;
	int iCh = '\\';
	int iPathLength;

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		GetModuleFileNameA(hModule, tBuff, MAX_PATH-1);
		ptCutBuff = strrchr ( tBuff, iCh );
		iPathLength = (int)(ptCutBuff - tBuff + 1);
		tBuff[iPathLength] = 0;
		strcpy_s(clFRAMEWORK_CFG_PATH, MAX_PATH-1, tBuff);
		strcat_s(clFRAMEWORK_CFG_PATH, MAX_PATH-1, "cl.cfg");
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (NULL == lpReserved) //Detach due to FreeLibrary
		{
			// release the framework proxy object 
			Intel::OpenCL::Framework::FrameworkProxy::Destroy();
		}
		//Else, either loading failed or process is terminating, do nothing and let OS reclaim resources
		break;
	}
	return TRUE;
}
