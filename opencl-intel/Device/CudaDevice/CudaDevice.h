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
* File CudaDevice.h
* declares C++ interface between the device and the Open CL frame work.
*
*/
#pragma once


#include "cl_device_api.h"
#include "cl_Logger.h"
#include "cl_thread.h"
#include "cl_synch_objects.h"
#include <cuda.h>
#include <map>
#include <vector>
#include <string>
#include <queue>
using namespace std;
using namespace Intel::OpenCL::Utils;


namespace Intel { namespace OpenCL { namespace CudaDevice {


#define N_MAX_STRING_SIZE 100
#define N_MAX_ARGUMENTS 20


typedef struct _KERNEL_ID
{
	cl_int ProgramID;
	string KernelName;
}KERNEL_ID;


class cCudaKernel
{
public:
	cCudaKernel();
	~cCudaKernel();
	CUfunction m_function;
	string m_OriginalName;
	KERNEL_ID m_KernelID;
	cl_kernel_arg_type m_Arguments[N_MAX_ARGUMENTS];
	cl_int m_NumberOfArguments;
};

typedef map<string, cCudaKernel*> KERNELS;

class cCudaProgram
{
public:
	cCudaProgram();
	~cCudaProgram();
	CUmodule m_module;
	KERNELS m_kernels;
	cl_uint m_ProgramID;
	cl_prog_container m_ProgContainer;
	string m_ProgramName;
};

typedef vector< cCudaProgram* > PROGRAMS;

typedef queue< cl_dev_cmd_desc* > COMMANDS;

class cCudaCommandList;

class cCudaCommandExecuteThread : public OclThread
{
public:
	cCudaCommandList *CommandList;
protected:
	int Run();
};

class cCudaDevice;

class cCudaCommandList
{
public:
	cCudaCommandList();
	~cCudaCommandList();
	cl_int m_ID;
	COMMANDS m_commands;
	cCudaCommandExecuteThread commands_execute;
	OclMutex m_QueueLock;
	OclMutex m_CondLock;
	OclCondition m_CommandEnqueued;
	cCudaDevice* m_device;
	void RunCommand(cl_dev_cmd_desc* cmd);
	CUdevice cuDevice;
	CUcontext cuContext;
};

typedef vector< cCudaCommandList* > COMMAND_LISTS;

class cCudaDevice
{
protected:

	friend cCudaCommandList;
	friend cCudaCommandExecuteThread;
	cCudaDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc);
	~cCudaDevice();

	static cCudaDevice*	m_pDevInstance;

	cl_int m_ParseCubin( const char* stCubinName , cCudaProgram OUT *Prog );

	PROGRAMS m_Programs;
	CUcontext m_context;
	CUdevice m_device;
	cl_uint m_id;
	cl_dev_call_backs m_CallBacks;
	COMMAND_LISTS m_CommandLists;


public:
	static cCudaDevice*			CreateDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc);
	static inline cCudaDevice*	GetInstance();
	void						Release();

	// Device entry points
	static cl_int clDevGetDeviceInfo(cl_device_info IN param, size_t IN val_size, void* OUT param_val,
							size_t* OUT param_val_size_ret);
	static cl_int clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list);
	static cl_int clDevRetainCommandList( cl_dev_cmd_list IN list);
	static cl_int clDevReleaseCommandList( cl_dev_cmd_list IN list );
	static cl_int clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN cmds, cl_uint IN count);
	static cl_int clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN image_type,
							cl_uint IN num_entries, cl_image_format* OUT formats, cl_uint* OUT num_entries_ret);
	static cl_int clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
									size_t IN width, size_t IN height, size_t IN depth, cl_dev_mem* OUT memObj);
	static cl_int clDevDeleteMemoryObject( cl_dev_mem IN memObj );
	static cl_int clDevCreateMappedRegion( cl_dev_mem IN memObj, const size_t IN origin[3], const size_t IN region[3],
									 void** OUT ptr, size_t* OUT row_pitch, size_t* OUT slice_pitch);
	static cl_int clDevReleaseMappedRegion( cl_dev_mem IN memObj, void* IN ptr);
	static cl_int clDevCheckProgramBinary( size_t IN bin_size, const void* IN bin );

	static cl_int clDevCreateProgram( size_t IN bin_size, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog );
	static cl_int clDevBuildProgram( cl_dev_program IN prog, const cl_char* IN options, void* IN user_data );

	static cl_int clDevReleaseProgram( cl_dev_program IN prog );
	static cl_int clDevUnloadCompiler();
	static cl_int clDevGetProgramBinary( cl_dev_program IN prog, size_t	IN size, void* OUT binary, size_t* OUT size_ret );
	static cl_int clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT size_ret);
	static cl_int clDevGetSupportedBinaries( cl_uint IN count, cl_prog_binary_desc* OUT types, size_t* OUT size_ret );
	static cl_int clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernel_id );
	static cl_int clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
									 cl_uint* OUT num_kernels_ret );
	static cl_int clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
								void* OUT value, size_t* OUT value_size_ret );
};

}}}
