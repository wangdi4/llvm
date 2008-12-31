// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "CudaDevice.h"

#include<stdlib.h>

using namespace Intel::OpenCL;

static cl_dev_entry_points myEntryPoints = {
	CudaDevice::clDevGetDeviceInfo,
	CudaDevice::clDevCreateCommandList,
	CudaDevice::clDevRetainCommandList,
	CudaDevice::clDevReleaseCommandList,
	CudaDevice::clDevCommandListExecute,
	CudaDevice::clDevGetSupportedImageFormats,
	CudaDevice::clDevCreateMemoryObject,
	CudaDevice::clDevDeleteMemoryObject,
	CudaDevice::clDevCreateMappedRegion,
	CudaDevice::clDevReleaseMappedRegion,
	CudaDevice::clDevCheckProgramBinary,
	CudaDevice::clDevBuildProgram,
	CudaDevice::clDevReleaseProgram,
	CudaDevice::clDevUnloadCompiler,
	CudaDevice::clDevGetProgramBinary,
	CudaDevice::clDevGetBuildLog,
	CudaDevice::clDevGetSupportedBinaries,
	CudaDevice::clDevGetKernelId,
	CudaDevice::clDevGetProgramKernels,
	CudaDevice::clDevGetKernelInfo
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
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

cl_int clDevInitDevice(	cl_uint					dev_id,
						cl_dev_entry_points		*dev_entry,
						cl_dev_call_backs		*dev_callbacks,
						cl_dev_log_descriptor	*log_desc
						)
{
	CudaDevice* dev = CudaDevice::CreateDevice(dev_id, dev_callbacks, log_desc);
	if (NULL == dev )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Copy entry points information
	memcpy(dev_entry, &myEntryPoints, sizeof(cl_dev_entry_points));

	return CL_DEV_SUCCESS;
}
