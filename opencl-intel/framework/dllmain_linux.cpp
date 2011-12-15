// dllmain.cpp : Defines the entry point for the DLL application.
#include <cassert>
#include <libgen.h>

#include "stdafx.h"
#include "framework_proxy.h"
#include "cl_secure_string_linux.h"
#include "cl_sys_defines.h"
#include "cl_sys_info.h"

#pragma comment(lib, "libcl_sys_utils.a")
#pragma comment(lib, "libcl_logger.so")
#pragma comment(lib, "libtask_executor.so")

using namespace Intel::OpenCL::Utils;

extern char clFRAMEWORK_CFG_PATH[];

__attribute__ ((constructor)) static void dll_init(void);
__attribute__ ((destructor)) static void dll_fini(void);

void dll_init(void)
{
	char tBuff[PATH_MAX];
	GetModulePathName((void*)(ptrdiff_t)dll_init, tBuff, PATH_MAX-1);
	safeStrCpy(clFRAMEWORK_CFG_PATH, MAX_PATH-1, dirname(tBuff));
	safeStrCat(clFRAMEWORK_CFG_PATH, MAX_PATH-1, "/cl.cfg");
}

void dll_fini(void)
{
	// release the framework proxy object 
	// Commenting out for SDK2012 as data races with terminating threads cause random crashes or deadlocks
	//Intel::OpenCL::Framework::FrameworkProxy::Destroy();
}

