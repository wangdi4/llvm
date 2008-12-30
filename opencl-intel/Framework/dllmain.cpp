// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

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
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

