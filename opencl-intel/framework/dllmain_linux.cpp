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

__attribute__ ((constructor)) static void dll_init(void);
__attribute__ ((destructor)) static void dll_fini(void);

void dll_init(void)
{
}

void dll_fini(void)
{
	// release the framework proxy object 
	Intel::OpenCL::Framework::FrameworkProxy::Destroy();
}

