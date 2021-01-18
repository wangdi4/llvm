// INTEL CONFIDENTIAL
//
// Copyright 2012-2019 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "framework_proxy.h"
#include <string>
#include "cl_disable_sys_dialog.h"
#include "cl_dynamic_lib.h"
#include "cl_sys_defines.h"
#include "cl_sys_info.h"

#if defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#pragma comment(lib, "cl_sys_utils.lib")
#ifdef _M_X64
#define TASK_EXECUTOR_LIB_NAME "task_executor64"
#else
#define TASK_EXECUTOR_LIB_NAME "task_executor32"
#endif

namespace {
	Intel::OpenCL::Utils::OclDynamicLib *m_dlTaskExecutor;
}

BOOL LoadTaskExecutor()
{
	std::string tePath = std::string(MAX_PATH, '\0');

        Intel::OpenCL::Utils::ConfigFile config(GetConfigFilePath());
	Intel::OpenCL::Utils::GetModuleDirectory(&tePath[0], MAX_PATH);
	tePath.resize(tePath.find_first_of('\0'));
	tePath += TASK_EXECUTOR_LIB_NAME;

        if (config.Read<std::string>("CL_CONFIG_DEVICES", "cpu") == "fpga-emu") {
            tePath += OUTPUT_EMU_SUFF;
        }
        tePath += ".dll";

	if (m_dlTaskExecutor->Load(tePath.c_str()) != 0) {
		return FALSE;
	}
	return TRUE;
}


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
#if !defined(INTEL_PRODUCT_RELEASE) && !defined(_DEBUG)
        Intel::OpenCL::Utils::DisableSystemDialogsOnCrash();
#endif
#ifdef _DEBUG
	// this is needed to initialize allocated objects DB, which is
	// maintained in only in debug
	InitSharedPtrs();
#endif
		m_dlTaskExecutor = new Intel::OpenCL::Utils::OclDynamicLib();
		return LoadTaskExecutor();
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		// release the framework proxy object
		Intel::OpenCL::Framework::FrameworkProxy::Destroy();
#ifdef _DEBUG
        FiniSharedPts();
#endif
		delete m_dlTaskExecutor;
		break;
	}
	return TRUE;
}
