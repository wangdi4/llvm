// INTEL CONFIDENTIAL
//
// Copyright 2006-2019 Intel Corporation.
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

///////////////////////////////////////////////////////////
//  dllmain.cpp
///////////////////////////////////////////////////////////

#pragma comment(lib, "cl_sys_utils.lib")
#pragma comment(lib, "cl_logger.lib")
#ifdef _M_X64
#define TASK_EXECUTOR_LIB_NAME "task_executor64.dll"
#else
#define TASK_EXECUTOR_LIB_NAME "task_executor32.dll"
#endif

#include "cpu_device.h"
#include "backend_wrapper.h"
#include <stdlib.h>
#include <string>
#include <cl_dynamic_lib.h>
#include <cl_sys_defines.h>
#include <cl_sys_info.h>

#if defined (_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

using namespace Intel::OpenCL::CPUDevice;

namespace {
    Intel::OpenCL::Utils::OclDynamicLib *m_dlTaskExecutor;
}

#ifdef _DEBUG
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
#endif // _DEBUG

BOOL LoadTaskExecutor()
{
    std::string tePath = std::string(MAX_PATH, '\0');

    Intel::OpenCL::Utils::GetModuleDirectory(&tePath[0], MAX_PATH);
    tePath.resize(tePath.find_first_of('\0'));
    tePath += TASK_EXECUTOR_LIB_NAME;

    if (!m_dlTaskExecutor->Load(tePath.c_str())) {
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
#ifdef _DEBUG
    DisableSystemDialogsOnCrash();
#endif
        m_dlTaskExecutor = new Intel::OpenCL::Utils::OclDynamicLib();
        return LoadTaskExecutor();
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        delete m_dlTaskExecutor;
        break;
    }
    return TRUE;
}
