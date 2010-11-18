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

/*
*
* File DummyDevice.h
* declares C++ interface between the device and the Open CL frame work.
*
*/
#pragma once

#include "cl_device_api.h"

namespace Intel { namespace OpenCL {

class DummyDevice
{
protected:
	DummyDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc);

	static DummyDevice*	m_pDevInstance;
public:
	static DummyDevice*			CreateDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc);
	static inline DummyDevice*	GetInstance();
	void						Release();

	// Device entry points
	static cl_int clDevGetDeviceInfo(cl_device_info IN param, size_t IN val_size, void* OUT param_val,
							size_t* OUT param_val_size_ret);
	static cl_int clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list);
	static cl_int clDevRetainCommandList( cl_dev_cmd_list IN list);
	static cl_int clDevReleaseCommandList( cl_dev_cmd_list IN list );
	static cl_int clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count);
	static cl_int clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN image_type,
							cl_uint IN num_entries, cl_image_format* OUT formats, cl_uint* OUT num_entries_ret);
	static cl_int clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
									cl_uint	IN dim_count, const size_t* dim, void*	buffer_ptr, const size_t* pitch,
									cl_dev_host_ptr_flags host_flags, cl_dev_mem* OUT memObj);
	static cl_int clDevDeleteMemoryObject( cl_dev_mem IN memObj );
	static cl_int clDevCreateMappedRegion( cl_dev_cmd_param_map* INOUT pMapParams);
	static cl_int clDevReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams);
	static cl_int clDevCheckProgramBinary( size_t IN bin_size, const void* IN bin );
	static cl_int clDevCreateProgram( size_t IN bin_size, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog );
	static cl_int clDevBuildProgram( cl_dev_program IN prog , const cl_char* IN options, void* IN user_data);
	static cl_int clDevReleaseProgram( cl_dev_program IN prog );

	static cl_int clDevUnloadCompiler();
	static cl_int clDevGetProgramBinary( cl_dev_program IN prog,
										 size_t IN size,
										 void* OUT binary,
										 size_t* OUT size_ret);
	static cl_int clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT size_ret);
	static cl_int clDevGetSupportedBinaries( cl_uint IN count, cl_prog_binary_desc* OUT types, size_t* OUT size_ret );
	static cl_int clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernel_id );
	static cl_int clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
									 size_t* OUT num_kernels_ret );
	static cl_int clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
								void* OUT value, size_t* OUT value_size_ret );
};
}}
