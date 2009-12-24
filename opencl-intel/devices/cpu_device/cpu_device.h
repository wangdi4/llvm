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
* File cpu_device.h
* declares C++ interface between the device and the Open CL frame work.
*
*/
#pragma once

#include "cl_device_api.h"
#include "program_service.h"
#include "memory_allocator.h"
#include "task_dispatcher.h"
#include "cl_dynamic_lib.h"
#include "cpu_dev_limits.h"
#include "cpu_config.h"

// OCL RT library initialization routing, imported from cl_builtin_functions.dll
extern "C" int __declspec(dllimport) clRTLibInitLibrary();

namespace Intel { namespace OpenCL { namespace CPUDevice {

extern const char* CPU_STRING;
extern const cl_image_format suportedImageFormats[];
extern const unsigned int NUM_OF_SUPPORTED_IMAGE_FORMATS;

class CPUDevice
{
private:
	ProgramService*			m_pProgramService;
	MemoryAllocator*		m_pMemoryAllocator;
	TaskDispatcher*			m_pTaskDispatcher;
	CPUDeviceConfig*		m_pCPUDeviceConfig;
	cl_dev_call_backs		m_frameWorkCallBacks;
	cl_uint					m_uiCpuId; 
	cl_dev_log_descriptor	m_logDescriptor;
	cl_int					m_iLogHandle;
	OclDynamicLib			m_dlRunTime;

protected:
	CPUDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc);
	~CPUDevice();
	static CPUDevice*	m_pDevInstance;
public:
	static CPUDevice*			CreateDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc);
	static inline CPUDevice*	GetInstance();
    static void                 Destroy();
	static cl_int               clDevGetDeviceInfo(cl_device_info IN param, size_t IN val_size, void* OUT paramVal, size_t* OUT param_val_size_ret);

	// Device entry points
	static cl_int clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list);
	static cl_int clDevFlushCommandList( cl_dev_cmd_list IN list);
	static cl_int clDevRetainCommandList( cl_dev_cmd_list IN list);
	static cl_int clDevReleaseCommandList( cl_dev_cmd_list IN list );
	static cl_int clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count);
	static cl_int clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN imageType,
							cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet);
	static cl_int clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
									cl_uint	IN dim_count, const size_t* IN dim_size, void*	IN buffer_ptr, const size_t* IN pitch,
									cl_dev_host_ptr_flags IN host_flags, cl_dev_mem* OUT memObj);
	static cl_int clDevDeleteMemoryObject( cl_dev_mem IN memObj );
	static cl_int clDevCreateMappedRegion( cl_dev_cmd_param_map* INOUT pMapParams);
	static cl_int clDevReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams );
	static cl_int clDevCheckProgramBinary( size_t IN binSize, const void* IN bin );
	static cl_int clDevCreateProgram( size_t IN binSize, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog );
	static cl_int clDevBuildProgram( cl_dev_program IN prog, const cl_char* IN options, void* IN userData);
	static cl_int clDevReleaseProgram( cl_dev_program IN prog );
	static cl_int clDevUnloadCompiler();
	static cl_int clDevGetProgramBinary( cl_dev_program IN prog, size_t IN size, void* OUT binary, size_t* OUT sizeRet );
	static cl_int clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT size_ret);
	static cl_int clDevGetSupportedBinaries( cl_uint IN count, cl_prog_binary_desc* OUT types, size_t* OUT sizeRet );
	static cl_int clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId );
	static cl_int clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN numKernels, cl_dev_kernel* OUT kernels,
									 cl_uint* OUT numKernelsRet );
	static cl_int clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
								void* OUT value, size_t* OUT valueSizeRet );
	static void	  clDevCloseDevice(void);
};

}}}
