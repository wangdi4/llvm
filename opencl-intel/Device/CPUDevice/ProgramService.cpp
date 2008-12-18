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
//  ProgramService.cpp
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ProgramService.h"

#include<stdlib.h>

using namespace Intel::OpenCL::CPUDevice;

ProgramService::ProgramService(cl_int devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc) :
	m_devId(devId), m_logDesc(logDesc)
{
	m_programId = 0;
	memcpy(&m_frameWorkCallBacks, devCallbacks, sizeof(m_frameWorkCallBacks));
}



ProgramService::~ProgramService()
{
	unsigned int i;
	for(i = 0; i<m_programId; i++)
	{
		ProgramInfo* info= m_programs[i];
		delete info;
	}
	m_programs.erase(m_programs.begin(), m_programs.end());
	m_programId = 0;
}

/****************************************************************************************************************
 checkProgramBinary
	Description
		Performs syntax validation of the intermediate or binary to be built by the device during later stages.
		Call backend compiler to do the check
	Input
		bin_size				Size of the binary buffer
		bin						A pointer to binary buffer that holds program container defined by cl_prog_container.
	Output
		NONE
	Returns
		CL_DEV_SUCCESS			The function is executed successfully.
		CL_DEV_INVALID_VALUE	If bin_size is 0 or bin is NULL.
		CL_DEV_INVALID_BINARY	If the binary is not supported by the device or program container content is invalid.
********************************************************************************************************************/
cl_int ProgramService::checkProgramBinary (size_t IN bin_size, const void* IN bin)
{
	return CL_DEV_INVALID_VALUE;
}
/*******************************************************************************************************************
clDevBuildProgram
	Description
		Builds (compiles & links) a program executable from the program intermediate or binary.
	Input
		bin_size						Size of the binary buffer
		bin								A pointer to binary buffer that holds program container defined by cl_prog_container. 
		options							A pointer to a string that describes the build options to be used for building the program executable.
										The list of supported options is described in section 5.4.3 in OCL spec. document.
		user_data						This value will be passed as an argument when clDevBuildFinished is called. Can be NULL.
		prop							Specifies the origin of the input binary. The values is defined by cl_dev_binary_prop.
	Output
		prog							A handle to created program object.
	Returns
		CL_DEV_SUCCESS					The function is executed successfully.
		CL_DEV_INVALID_BUILD_OPTIONS	If build options for back-end compiler specified by options are invalid.
		CL_DEV_INVALID_BINARY			If the back-end compiler failed to process binary.
		CL_DEV_OUT_OF_MEMORY			If the device failed to allocate memory for the program
***********************************************************************************************************************/

cl_int ProgramService::buildProgram( size_t IN bin_size,
								   const void* IN bin,
								   const cl_char* IN options,
								   void* IN user_data,
								   cl_dev_binary_prop IN prop,
								   cl_dev_program* OUT prog
								   )
{
	if(0 == bin_size || NULL == bin)
	{
		return CL_DEV_INVALID_BUILD_OPTIONS;
	}
	//If the origin of binary is one of the Front-End compilers
	//It is not supported yet
	//Currently spport only binary that was loaded by user
	if(prop != CL_DEV_BINARY_USER)
	{
		return CL_DEV_INVALID_BUILD_OPTIONS;
	}

	//Keep program binary in the list
	ProgramInfo *info = new ProgramInfo;
	if(NULL == info)
	{
		return CL_DEV_OUT_OF_MEMORY;

	}
	info->bin = bin;
	info->binSize = bin_size;

	m_programs[m_programId] = info;
	*(unsigned int*)prog = m_programId;
	m_programId++;
	//When finish call the framework callback that compilation is done
	//TODO: Currently problematic because framework doesnt know ID yet
	//Need to return someway after that from thread
	m_frameWorkCallBacks.pclDevBuildFinished((cl_dev_program)m_programId);
	return CL_DEV_SUCCESS;
}
/********************************************************************************************************************
clDevUnloadCompiler
	Description
		Allows the framework to release the resources allocated by the back-end compiler.
		This is a hint from the framework and does not guarantee that the compiler will not be used in the future
		or that the compiler will actually be unloaded by the device.
	Input
		NONE
	Output
		NONE
	Returns
		CL_DEV_SUCCESS	The function is executed successfully.
********************************************************************************************************************/

cl_int ProgramService::unloadCompiler()
{
	return CL_DEV_SUCCESS;
}
/********************************************************************************************************************
clDevGetProgramBinary
	Description
		Returns the compiled program binary. The output buffer contains program container as defined cl_prog_binary_desc.
	Input
		prog					A handle to created program object.
	Output
		binary					A pointer to buffer wherein program binary will be stored.
		size_ret				The actual size in bytes of the returned buffer. When size is equal to 0 and binary is NULL, returns size in bytes of a program binary. If NULL the parameter is ignored.
	Returns
		CL_DEV_SUCCESS			The function is executed successfully.
		CL_DEV_INVALID_PROGRAM	If program is not valid program object. 
		CL_DEV_INVALID_VALUE	If size is not enough to store the binary or binary is NULL and size is not 0.
********************************************************************************************************************/
cl_int ProgramService::getProgramBinary( cl_dev_program IN prog,
									 const void** OUT binary,
									 size_t* OUT size_ret
									 )
{
	unsigned int programId = (unsigned int)prog;
	if(programId >= m_programId)
	{
		return CL_DEV_INVALID_VALUE;
	}

	ProgramInfo *programInfo = m_programs[programId];
	*binary = (void*)programInfo->bin;
	*size_ret = programInfo->binSize;
	
	return CL_DEV_SUCCESS;
}

cl_int ProgramService::getBuildLog( cl_dev_program IN prog,
								  size_t IN size,
								  char* OUT log,
								  size_t* OUT size_ret
								  )
{
	return CL_DEV_INVALID_VALUE;
}
/************************************************************************************************************
	clDevGetSupportedBinaries (optional)
	Description
		Returns the list of supported binaries.
	Input
		count					Size of the buffer passed to the function in terms of cl_prog_binary_desc. 
	Output
		types					A pointer to buffer wherein binary types will be stored.
		count_ret				The actual size ofthe buffer returned by the function in terms of cl_prog_binary_desc.
								When count is equal to 0 and types is NULL, function returns a size of the list.
								If NULL the parameter is ignored.
	Returns
		CL_DEV_SUCCESS			The function is executed successfully.
		CL_DEV_INVALID_PROGRAM	If program is not valid program object. 
		CL_DEV_INVALID_VALUE	If count is not enough to store the binary or types is NULL and count is not 0.
***************************************************************************************************************/
cl_int ProgramService::getSupportedBinaries( cl_uint IN count,
									   cl_prog_binary_desc* OUT types,
									   size_t* OUT size_ret
									   )
{
	if(count < 1)
	{
		return CL_DEV_INVALID_VALUE;
	}
	//currently support only user binaries

	types[0].bin_type = CL_PROG_BIN_LLVM;
	types[0].bin_ver_major = 0;
	types[0].bin_ver_minor = 0;
	*size_ret = 1;

	return CL_DEV_SUCCESS;
}


cl_int ProgramService::getKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernel_id )
{
		return CL_DEV_INVALID_VALUE;
}



cl_int ProgramService::getProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
						 size_t* OUT num_kernels_ret )
{
		return CL_DEV_INVALID_VALUE;
}



cl_int ProgramService::getKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
					void* OUT value, size_t* OUT value_size_ret )
{
		return CL_DEV_INVALID_VALUE;
}

