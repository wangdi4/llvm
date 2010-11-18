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
#include "CL\cl.h"
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

//classes declarations
class cCudaKernel;
class cCudaProgram;
class cCudaCommandExecuteThread;
class cCudaCommandList;
class cCudaDevice;
class cCudaMemObject;

//enums
typedef enum CudaCommandType
{
	CUDA_BUILD_PROGRAM,
	CUDA_RUN_KERNEL,
	CUDA_MEM_READ,
	CUDA_MEM_WRITE,
	CUDA_OTHER,								
};

//structs
typedef struct _CUDA_BUILD_PROGRAM_CONTAINER
{
	cl_uint prog;
	cl_char *options;
	void *user_data;
}CUDA_BUILD_PROGRAM_CONTAINER;
typedef struct _COMMAND_CONTAINER
{
	~_COMMAND_CONTAINER();
	CudaCommandType CommandType;
	void* Command;
}COMMAND_CONTAINER;
typedef struct _KERNEL_ID
{
	cl_int ProgramID;
	string KernelName;
}KERNEL_ID;

//typedefs
typedef map<string, cCudaKernel*> KERNELS;
typedef vector< cCudaProgram* > PROGRAMS;
typedef queue< COMMAND_CONTAINER* > COMMANDS;
typedef vector< cCudaCommandList* > COMMAND_LISTS;

//classes implementation
class cCudaMemObject
{
public:
	cCudaMemObject( cl_dev_mem_flags flags, 
					const cl_image_format* format, 
					cl_uint dim_count, 
					const size_t* dim,
					void* buffer_ptr,
					const size_t* pitch);
	~cCudaMemObject();
	CUdeviceptr GetPtr();
	int GetRW();
	int Write(cl_uint dim_count, size_t* origin, size_t* region, void* ptr, size_t* pitch);
	int Read(cl_uint dim_count, size_t* origin, size_t* region, void* ptr, size_t* pitch);

private:
	cl_uint m_RW;
	cl_uint m_OnHost;
	cl_uint m_DimCount;
	size_t* m_dim;
	CUdeviceptr m_DevPtr;
	void * m_HostPtr;
	size_t* m_pitch;
};

class cCudaKernel
{
public:
	cCudaKernel();
	~cCudaKernel();
	CUfunction m_function;
	string m_OriginalName;
	KERNEL_ID m_KernelID;
	cl_kernel_argument m_Arguments[N_MAX_ARGUMENTS];
	cl_uint m_MemFlags[N_MAX_ARGUMENTS];
	cl_int m_NumberOfArguments;
};



class cCudaProgram
{
public:
	//functions
	cCudaProgram( cCudaDevice* device );
	cCudaProgram( cCudaDevice* device, cl_prog_container *ProgContainer, cl_uint ID );
	~cCudaProgram();
	cl_int GetBinary(size_t size, void *binary, size_t *size_ret);
	cl_int GetKernelID(const char* IN name, cl_dev_kernel* OUT kernel_id );
	cl_int GetKernels(cl_uint IN num_kernels, KERNEL_ID** OUT kernels, cl_uint* OUT num_kernels_ret );
	cl_int GetKernelArguments( string KernelName, 
							  size_t IN value_size, 
							  cl_kernel_argument* OUT value, 
							  size_t* OUT value_size_ret );
	cl_int Build(const cl_char *options, void *user_data);
	cl_int RunKernel( cl_dev_cmd_param_kernel* pKernelParam, cl_dev_cmd_id	id, void* data );

	//members
	cl_uint m_ProgramID;

private:
	//functions
	cl_int ParseCubin( const char* stCubinName );

	//members
	CUmodule m_module;
	KERNELS m_kernels;
	cl_prog_container m_ProgContainer;
	string m_ProgramName;
	cCudaDevice* m_device;
};

class cCudaCommandExecuteThread : public OclThread
{
public:
	cCudaCommandExecuteThread(cCudaDevice* pDevice, cCudaCommandList* pCommandList);
	~cCudaCommandExecuteThread();

protected:
	//members

	CUdevice m_cuDevice;
	CUcontext m_cuContext;
	cCudaDevice* m_device;
	cCudaCommandList* m_CommandList;

	//functions
	int Run();
	cl_int BuildProgram( CUDA_BUILD_PROGRAM_CONTAINER BuildData );
	cl_int RunKernel( cl_dev_cmd_desc cmd );
	cl_int RunRead( cl_dev_cmd_desc cmd );
	cl_int RunWrite( cl_dev_cmd_desc cmd );
};



class cCudaCommandList
{
public:
	cCudaCommandList(cl_int ID, cCudaDevice* pDevice);
	~cCudaCommandList();

	void StartThread();
	cl_dev_cmd_list GetID();
	cl_int PushRead(cl_dev_cmd_desc* cmds);
	cl_int PushWrite(cl_dev_cmd_desc* cmds);
	cl_int PushKernel(cl_dev_cmd_desc* cmds);
	cl_int PushBuildProgram(cl_uint prog, const cl_char *options, void *user_data);
	bool IsEmpty();
	COMMAND_CONTAINER* Pop();

private:
	//members
	cl_int m_ID;
	cCudaDevice* m_device;
	COMMANDS m_commands;
	OclMutex m_QueueLock;
	OclCondition m_CommandEnqueued;
	cCudaCommandExecuteThread m_commands_execute;
};




class cCudaDevice
{
protected:

	friend cCudaCommandList;
	friend cCudaCommandExecuteThread;
	friend cCudaProgram;
	cCudaDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc);
	~cCudaDevice();

	static cCudaDevice*	m_pDevInstance;

	cl_uint m_id;
	cl_dev_call_backs m_CallBacks;

	PROGRAMS m_Programs;
	COMMAND_LISTS m_CommandLists;
	CUdevice m_CuDev;
	CUdevprop m_Prop;


public:
	static cCudaDevice*			CreateDevice(cl_uint devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc);
	static inline cCudaDevice*	GetInstance();
	void						Release();

	// Device entry points
	///////////////////////////////////////not implemented/////////////////////////////////////////////////////////
	static cl_int clDevRetainCommandList( cl_dev_cmd_list IN list);
	static cl_int clDevReleaseCommandList( cl_dev_cmd_list IN list );
	static cl_int clDevGetSupportedImageFormats( cl_dev_mem_flags IN flags, cl_dev_mem_object_type IN image_type,
							cl_uint IN num_entries, cl_image_format* OUT formats, cl_uint* OUT num_entries_ret);
	static cl_int clDevCreateMappedRegion( cl_dev_cmd_param_map* INOUT pMapParams );
	static cl_int clDevReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams );
	static cl_int clDevCheckProgramBinary( size_t IN bin_size, const void* IN bin );
	static cl_int clDevReleaseProgram( cl_dev_program IN prog );
	static cl_int clDevUnloadCompiler();
	static cl_int clDevGetBuildLog( cl_dev_program IN prog, size_t IN size, char* OUT log, size_t* OUT size_ret);
	static cl_int clDevGetSupportedBinaries( cl_uint IN count, cl_prog_binary_desc* OUT types, size_t* OUT size_ret );
	static cl_int clDevGetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
								void* OUT value, size_t* OUT value_size_ret );



	/////////////////////////////////////////implemented///////////////////////////////////////////////////////////
	static cl_int clDevGetDeviceInfo(cl_device_info IN param, size_t IN val_size, void* OUT param_val,
							size_t* OUT param_val_size_ret);
	static cl_int clDevCreateCommandList( cl_dev_cmd_list_props IN props, cl_dev_cmd_list* OUT list);
	static cl_int clDevCommandListExecute( cl_dev_cmd_list IN list, cl_dev_cmd_desc* IN *cmds, cl_uint IN count);
	static cl_int clDevCreateMemoryObject( cl_dev_mem_flags IN flags, const cl_image_format* IN format,
									cl_uint	IN dim_count, const size_t* dim, void* buffer_ptr, const size_t* pitch,
									cl_dev_host_ptr_flags host_flags, cl_dev_mem* OUT memObj);
	static cl_int clDevDeleteMemoryObject( cl_dev_mem IN memObj );
	static cl_int clDevCreateProgram( size_t IN bin_size, const void* IN bin, cl_dev_binary_prop IN prop, cl_dev_program* OUT prog );
	static cl_int clDevBuildProgram( cl_dev_program IN prog, const cl_char* IN options, void* IN user_data );
	static cl_int clDevGetProgramBinary( cl_dev_program IN prog, size_t	IN size, void* OUT binary, size_t* OUT size_ret );
	static cl_int clDevGetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernel_id );
	static cl_int clDevGetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
									 cl_uint* OUT num_kernels_ret );
};

}}}
