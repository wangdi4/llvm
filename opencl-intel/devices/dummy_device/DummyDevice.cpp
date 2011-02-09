// Copyright (c) 2006-2008 Intel Corporation
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

// DummyDevice.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "DummyDevice.h"

using namespace Intel::OpenCL;

// Static members initialization
DummyDevice* DummyDevice::m_pDevInstance = NULL;

DummyDevice::DummyDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
{
}


// ---------------------------------------
// Public functions / Device entry points

DummyDevice* DummyDevice::CreateDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc)
{
	// TODO : ADD log
	if ( NULL == m_pDevInstance )
	{
		m_pDevInstance = new DummyDevice(devId, devCallbacks, logDesc);
	}

	return m_pDevInstance;
}

DummyDevice* DummyDevice::GetInstance()
{
	return m_pDevInstance;
}

// Device entry points
cl_int DummyDevice::clDevGetDeviceInfo(cl_device_info IN param, size_t IN val_size, void* OUT param_val,
				size_t* OUT param_val_size_ret)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevRetainCommandList( cl_dev_cmd_list IN list)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevReleaseCommandList( cl_dev_cmd_list IN list )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN image_type,
				cl_uint IN num_entries, cl_image_format* OUT formats, cl_uint* OUT num_entries_ret)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
									cl_uint	IN dim_count, const size_t* dim, void*	buffer_ptr, const size_t* pitch,
									cl_dev_host_ptr_flags host_flags, cl_dev_mem* OUT memObj)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevDeleteMemoryObject( cl_dev_mem IN memObj )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevCreateMappedRegion( cl_dev_cmd_param_map* INOUT pMapParams )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevReleaseMappedRegion( cl_dev_cmd_param_map* INOUT pMapParams )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevCheckProgramBinary( size_t IN bin_size, const void* IN bin )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevCreateProgram( size_t IN bin_size, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevBuildProgram( cl_dev_program IN prog , const cl_char* IN options, void* IN user_data)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevReleaseProgram( cl_dev_program IN prog )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevUnloadCompiler()
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevGetProgramBinary(  cl_dev_program IN prog,
											size_t IN size,
											void* OUT binary,
											size_t* OUT size_ret)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT size_ret)
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevGetSupportedBinaries( cl_uint IN count, cl_prog_binary_desc* OUT types, size_t* OUT size_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernel_id )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
						 size_t* OUT num_kernels_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

cl_int DummyDevice::clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
					void* OUT value, size_t* OUT value_size_ret )
{
	// TODO : ADD log
	return CL_DEV_INVALID_OPERATION;
}

