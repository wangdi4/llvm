// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
// 
// WARRANTY DISCLAIMER
// 
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  dllmain.cpp
///////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma comment(lib, "cl_sys_utils.lib")
#pragma comment(lib, "cl_logger.lib")
#pragma comment(lib, "task_executor.lib")

#if defined(USE_GPA)
#pragma comment(lib, "gpasdk_d.lib")
#endif

#include "cpu_device.h"
#include "backend_wrapper.h"
#include<stdlib.h>

using namespace Intel::OpenCL::CPUDevice;


extern char clCPUDEVICE_CFG_PATH[];

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
        strcpy_s(clCPUDEVICE_CFG_PATH, MAX_PATH-1, tBuff);
        strcat_s(clCPUDEVICE_CFG_PATH, MAX_PATH-1, "cl.cfg");
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

/************************************************************************************************************************
   clDevGetDeviceInfo
**************************************************************************************************************************/
extern "C" cl_dev_err_code clDevGetDeviceInfo(  cl_device_info  param, 
                            size_t          valSize, 
                            void*           paramVal,
                            size_t*         paramValSizeRet
                            )
{
    return CPUDevice::clDevGetDeviceInfo(param, valSize, paramVal, paramValSizeRet);
}

