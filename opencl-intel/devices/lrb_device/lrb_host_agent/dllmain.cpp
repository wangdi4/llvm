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
#include "lrb_agent.h"
#include <windows.h>

using namespace Intel::OpenCL::LRBAgent;

/******************************************************************
 *
 ******************************************************************/
static cl_dev_entry_points myEntryPoints = {
	LrbAgent::clDevCreateCommandList,
    LrbAgent::clDevFlushCommandList,
	LrbAgent::clDevRetainCommandList,
	LrbAgent::clDevReleaseCommandList,
	LrbAgent::clDevCommandListExecute,
	LrbAgent::clDevGetSupportedImageFormats,
	LrbAgent::clDevCreateMemoryObject,
	LrbAgent::clDevDeleteMemoryObject,
	LrbAgent::clDevCreateMappedRegion,
	LrbAgent::clDevReleaseMappedRegion,
	LrbAgent::clDevCheckProgramBinary,
	LrbAgent::clDevCreateProgram,
	LrbAgent::clDevBuildProgram,
	LrbAgent::clDevReleaseProgram,
	LrbAgent::clDevUnloadCompiler,
	LrbAgent::clDevGetProgramBinary,
	LrbAgent::clDevGetBuildLog,
	LrbAgent::clDevGetSupportedBinaries,
	LrbAgent::clDevGetKernelId,
	LrbAgent::clDevGetProgramKernels,
	LrbAgent::clDevGetKernelInfo,
	LrbAgent::clDevCloseDevice
};

/******************************************************************
 *
 ******************************************************************/
BOOL APIENTRY DllMain( 
    HMODULE hModule,
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
        // Release agent if havn't done before
        LrbAgent::Destroy();
		break;
	}
	return TRUE;
}

/******************************************************************
 * clDevInitDevice: General entry point for all OpenCL agents.
 * This function will be called first immediately after the library was loaded.
 ******************************************************************/
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
	
    LrbAgent* agent = LrbAgent::GetInstance();
	if (NULL == agent )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}

    agent->Initialize(dev_id, dev_callbacks, log_desc);

	// Copy entry points information
	memcpy(dev_entry, &myEntryPoints, sizeof(cl_dev_entry_points));
   
	return CL_DEV_SUCCESS;
}


/******************************************************************
 * clDevGetDeviceInfo: Use to get LRB device info
 * 
 ******************************************************************/
cl_int clDevGetDeviceInfo(  cl_device_info  param, 
                            size_t          valSize, 
                            void*           paramVal,
				            size_t*         paramValSizeRet
                            )
{
    return LrbAgent::clDevGetDeviceInfo(param, valSize, paramVal, paramValSizeRet);
}
