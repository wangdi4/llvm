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
#pragma comment(lib, "cl_builtin_functions.lib")
#pragma comment(lib, "task_executor.lib")
#pragma comment(lib, "llvm_backend.lib")
#pragma comment(lib, "tal_s.lib")

#pragma warning(push)
  #pragma warning(disable:4391)
  #include <intrin.h>
#pragma warning(pop)
#include "cpu_device.h"

#include<stdlib.h>

using namespace Intel::OpenCL::CPUDevice;

static cl_dev_entry_points myEntryPoints = {
	CPUDevice::clDevCreateCommandList,
	CPUDevice::clDevFlushCommandList,
	CPUDevice::clDevRetainCommandList,
	CPUDevice::clDevReleaseCommandList,
	CPUDevice::clDevCommandListExecute,
	CPUDevice::clDevGetSupportedImageFormats,
	CPUDevice::clDevCreateMemoryObject,
	CPUDevice::clDevDeleteMemoryObject,
	CPUDevice::clDevCreateMappedRegion,
	CPUDevice::clDevReleaseMappedRegion,
	CPUDevice::clDevCheckProgramBinary,
	CPUDevice::clDevCreateProgram,
	CPUDevice::clDevBuildProgram,
	CPUDevice::clDevReleaseProgram,
	CPUDevice::clDevUnloadCompiler,
	CPUDevice::clDevGetProgramBinary,
	CPUDevice::clDevGetBuildLog,
	CPUDevice::clDevGetSupportedBinaries,
	CPUDevice::clDevGetKernelId,
	CPUDevice::clDevGetProgramKernels,
	CPUDevice::clDevGetKernelInfo,
	CPUDevice::clDevCloseDevice
};

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
        CPUDevice::Destroy();
		break;
	}
	return TRUE;
}

bool isIntelCpu()
{
	char vcCPUString[0x20] = {0};
    int viCPUInfo[4] = {-1};

    // get the CPU string and the number of valid of valid IDs
    __cpuid(viCPUInfo, 0);
    int iValidIDs = viCPUInfo[0];
    *((int*)vcCPUString) = viCPUInfo[1];
    *((int*)(vcCPUString+4)) = viCPUInfo[3];
    *((int*)(vcCPUString+8)) = viCPUInfo[2];

    if (strcmp(vcCPUString, CPU_STRING) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }

}
cl_int clDevCreateDeviceInstance(	
                        cl_uint					dev_id,
						cl_dev_entry_points		*dev_entry,
						cl_dev_call_backs		*dev_callbacks,
						cl_dev_log_descriptor	*log_desc
						)
{
	
	if(NULL == dev_callbacks || NULL == dev_entry)
	{
		return CL_DEV_INVALID_OPERATION;
	}
	// If not Intel CPU return error
	if(!isIntelCpu())
	{
		return CL_DEV_ERROR_FAIL;
	}
	
	CPUDevice* dev = CPUDevice::CreateDevice(dev_id, dev_callbacks, log_desc);
	if (NULL == dev )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Copy entry points information
	memcpy(dev_entry, &myEntryPoints, sizeof(cl_dev_entry_points));

	return CL_DEV_SUCCESS;
}

/************************************************************************************************************************
   clDevGetDeviceInfo
**************************************************************************************************************************/
cl_int clDevGetDeviceInfo(  cl_device_info  param, 
                            size_t          valSize, 
                            void*           paramVal,
				            size_t*         paramValSizeRet
                            )
{
    return CPUDevice::clDevGetDeviceInfo(param, valSize, paramVal, paramValSizeRet);
}

