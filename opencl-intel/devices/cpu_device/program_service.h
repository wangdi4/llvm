
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

///////////////////////////////////////////////////////////
//  ProgramService.h
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>

#include "cl_device_api.h"
#include "handle_allocator.h"
#include "cl_dev_backend_api.h"
#include "cl_synch_objects.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::DeviceBackend;

namespace Intel { namespace OpenCL { namespace CPUDevice {

class ProgramService
{

public:
	ProgramService(cl_int devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc);
	virtual ~ProgramService();

	cl_int CheckProgramBinary (size_t IN bin_size, const void* IN bin);
    cl_int CreateProgram( size_t IN binSize,
										const void* IN bin,
										cl_dev_binary_prop IN prop,
										cl_dev_program* OUT prog
									   );
    cl_int BuildProgram( cl_dev_program IN prog,
										const cl_char* IN options,
										void* IN userData
									   );
	cl_int ReleaseProgram( cl_dev_program IN prog );
	cl_int UnloadCompiler();
    cl_int GetProgramBinary( cl_dev_program IN prog,
										size_t IN size,
										void* OUT binary,
										size_t* OUT sizeRet
										);

	cl_int GetBuildLog( cl_dev_program IN prog,
									  size_t IN size,
									  char* OUT log,
									  size_t* OUT sizeRet
									  );
	cl_int GetSupportedBinaries( size_t IN size,
										   cl_prog_binary_desc* OUT types,
										   size_t* OUT sizeRet
										   );

	cl_int GetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId );

	cl_int GetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
						 cl_uint* OUT numKernelsRet );

	cl_int GetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN valueSize,
					void* OUT value, size_t* OUT valueSizeRet ) const;

protected:
	friend class ProgramBuildTask;
	typedef std::map<std::string, cl_dev_kernel>			TName2IdMap;
	struct	TProgramEntry
	{
		ICLDevBackendProgram*	pProgram;
		cl_build_status			clBuildStatus;
		TName2IdMap				mapKernels;
		OclMutex				muMap;
	};
	typedef std::map<cl_dev_program, TProgramEntry*>	TProgramMap;

	void	DeleteProgramEntry(TProgramEntry* pEntry);

	cl_int							m_iDevId;
	cl_dev_log_descriptor			m_logDescriptor;
	cl_int							m_iLogHandle;
	HandleAllocator<unsigned int>	m_progIdAlloc;
	TProgramMap						m_mapPrograms;
	OclMutex						m_muProgMap;
	cl_dev_call_backs				m_sCallBacks;
};

}}};