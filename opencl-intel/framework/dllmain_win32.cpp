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

#ifdef _MSC_VER
static int AvoidMessageBoxHook(int ReportType, char *Message, int *Return) {
	// Set *Return to the retry code for the return value of _CrtDbgReport:
	// http://msdn.microsoft.com/en-us/library/8hyw4sy7(v=vs.71).aspx
	// This may also trigger just-in-time debugging via DebugBreak().
	if (Return)
		*Return = 1;
	// Don't call _CrtDbgReport.
	return 1;
}
#endif


static void DisableSystemDialogsOnCrash() {
#ifdef _MSC_VER
	// Helpful text message is printed when a program is abnormally terminated
	_set_abort_behavior(0, _WRITE_ABORT_MSG);
	// Disable Dr. Watson.
	_set_abort_behavior(0, _CALL_REPORTFAULT);
	_CrtSetReportHook(AvoidMessageBoxHook);
#endif

	// Disable standard error dialog box.
	SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX |
		SEM_NOOPENFILEERRORBOX);
	_set_error_mode(_OUT_TO_STDERR);
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#ifdef _DEBUG
	DisableSystemDialogsOnCrash();

	// this is needed to initialize allocated objects DB, which is
	// maintained in only in debug
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
