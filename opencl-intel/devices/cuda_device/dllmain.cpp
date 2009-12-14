// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "CudaDevice.h"

#include <stdlib.h>

using namespace Intel::OpenCL::CudaDevice;

static cl_dev_entry_points myEntryPoints = {
	cCudaDevice::clDevCreateCommandList,
	cCudaDevice::clDevRetainCommandList,
	cCudaDevice::clDevReleaseCommandList,
	cCudaDevice::clDevCommandListExecute,
	cCudaDevice::clDevGetSupportedImageFormats,
	cCudaDevice::clDevCreateMemoryObject,
	cCudaDevice::clDevDeleteMemoryObject,
	cCudaDevice::clDevCreateMappedRegion,
	cCudaDevice::clDevReleaseMappedRegion,
	cCudaDevice::clDevCheckProgramBinary,
	cCudaDevice::clDevCreateProgram,
	cCudaDevice::clDevBuildProgram,
	cCudaDevice::clDevReleaseProgram,
	cCudaDevice::clDevUnloadCompiler,
	cCudaDevice::clDevGetProgramBinary,
	cCudaDevice::clDevGetBuildLog,
	cCudaDevice::clDevGetSupportedBinaries,
	cCudaDevice::clDevGetKernelId,
	cCudaDevice::clDevGetProgramKernels,
	cCudaDevice::clDevGetKernelInfo
};

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
        break;
	case DLL_PROCESS_DETACH:
        // Here we supposed to destory the device
		break;
	}
	return TRUE;
}

cl_int clDevCreateDeviceInstance(	cl_uint					dev_id,
						cl_dev_entry_points		*dev_entry,
						cl_dev_call_backs		*dev_callbacks,
						cl_dev_log_descriptor	*log_desc
						)
{
	cCudaDevice* dev = cCudaDevice::CreateDevice(dev_id, dev_callbacks, log_desc);
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
    return cCudaDevice::clDevGetDeviceInfo(param, valSize, paramVal, paramValSizeRet);
}
