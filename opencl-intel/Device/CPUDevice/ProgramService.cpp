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

#include <stdlib.h>
#include <limits.h>
#include <assert.h>

using namespace Intel::OpenCL::CPUDevice;

// Static members
static cl_prog_binary_desc gSupportedBinTypes[] = 
{
	{CL_PROG_BIN_X86, 0, 0}
};
static	unsigned int	gSupportedBinTypesCount = sizeof(gSupportedBinTypes)/sizeof(cl_prog_binary_desc);

ProgramService::ProgramService(cl_int devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc) :
	m_devId(devId), m_progIdAlloc(1, UINT_MAX)
{
	memcpy_s(&m_frameWorkCallBacks, sizeof(cl_dev_call_backs), devCallbacks, sizeof(cl_dev_call_backs));
	if ( NULL == logDesc )
	{
		memset(&m_logDesc, 0, sizeof(cl_dev_log_descriptor));
	}
	else
	{
		memcpy_s(&m_logDesc, sizeof(cl_dev_log_descriptor), logDesc, sizeof(cl_dev_log_descriptor));
	}
}

ProgramService::~ProgramService()
{
	ProgramMap_t::iterator	it;

	// Go throught the map and remove all allocated programs
	for(it = m_programs.begin(); it != m_programs.end(); ++it)
	{
		ProgramInfo_t* info= it->second;
		// Clean allocated memory
		assert(info);
		assert(info->bin);
		free(info->bin);
		delete info;
	}

	m_progIdAlloc.Clear();
	m_programs.clear();
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
		// TODO: ADD log
		return CL_DEV_INVALID_VALUE;
	}
	//If the origin of binary is one of the Front-End compilers
	//It is not supported yet
	//Currently spport only binary that was loaded by user
	if(prop != CL_DEV_BINARY_USER)
	{
		// TODO: ADD log
		return CL_DEV_INVALID_BUILD_OPTIONS;
	}

	if ( NULL == prog )
	{
		// TODO: ADD log
		return CL_DEV_INVALID_VALUE;
	}

	// Allocate new program id
	unsigned int newProgId;
	if ( !m_progIdAlloc.AllocateHandle(&newProgId) )
	{
		// TODO: ADD log
		return CL_DEV_OUT_OF_MEMORY;
	}

	//Keep program binary in the list
	ProgramInfo_t *info = new ProgramInfo_t;
	if(NULL == info)
	{
		m_progIdAlloc.FreeHandle(newProgId);
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Allocate memory to store the binary
	info->bin = malloc(bin_size);
	if ( NULL == info->bin )
	{
		m_progIdAlloc.FreeHandle(newProgId);
		delete info;
		return CL_DEV_OUT_OF_MEMORY;
	}

	info->binSize = bin_size;
	memcpy_s(info->bin, info->binSize, bin, bin_size);

	m_programs[newProgId] = info;
	*prog = (cl_dev_program)newProgId;

	//When finish call the framework callback that compilation is done
	//TODO: Currently problematic because framework doesnt know ID yet
	//Need to return someway after that from thread
	m_frameWorkCallBacks.pclDevBuildFinished((cl_dev_program)newProgId);

	return CL_DEV_SUCCESS;
}

/********************************************************************************************************************
clDevReleaseProgram
	Description
		Deletes previously created program object and releases all related resources.
	Input
		prog							A handle to program object to be deleted
	Output
		NONE
	Returns
		CL_DEV_SUCCESS					The function is executed successfully.
		CL_DEV_INVALID_PROGRAM			Invalid program object was specified.
********************************************************************************************************************/
cl_int ProgramService::releaseProgram( cl_dev_program IN prog )
{
	unsigned int progId = (unsigned int)prog;
	ProgramMap_t::iterator	it;

	it = m_programs.find(progId);
	if( it == m_programs.end())
	{
		return CL_DEV_INVALID_PROGRAM;
	}

	ProgramInfo_t* info= it->second;
	// Clean allocated memory
	assert(info);
	assert(info->bin);
	free(info->bin);
	delete info;
	m_programs.erase(it);

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
		size					Size in bytes of the buffer passed to the function. 
	Output
		binary					A pointer to buffer wherein program binary will be stored.
		size_ret				The actual size in bytes of the returned buffer. When size is equal to 0 and binary is NULL, returns size in bytes of a program binary. If NULL the parameter is ignored.
	Returns
		CL_DEV_SUCCESS			The function is executed successfully.
		CL_DEV_INVALID_PROGRAM	If program is not valid program object. 
		CL_DEV_INVALID_VALUE	If size is not enough to store the binary or binary is NULL and size is not 0.
********************************************************************************************************************/
cl_int ProgramService::getProgramBinary( cl_dev_program IN prog,
										size_t IN size,
										void* OUT binary,
										size_t* OUT size_ret
										)
{
	unsigned int progId = (unsigned int)prog;
	ProgramMap_t::iterator	it;

	it = m_programs.find(progId);
	if( it == m_programs.end())
	{
		return CL_DEV_INVALID_PROGRAM;
	}

	ProgramInfo_t *programInfo = it->second;
	if ( NULL != size_ret )
	{
		*size_ret = programInfo->binSize;
	}

	if ( (0 == size) && (NULL == binary) )
	{
		return CL_DEV_SUCCESS;
	}

	if ( (NULL == binary) || (size < programInfo->binSize) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	memcpy_s(binary, size, programInfo->bin, programInfo->binSize);
	
	return CL_DEV_SUCCESS;
}

cl_int ProgramService::getBuildLog( cl_dev_program IN prog,
								  size_t IN size,
								  char* OUT log,
								  size_t* OUT size_ret
								  )
{
	return CL_DEV_INVALID_OPERATION;
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
cl_int ProgramService::getSupportedBinaries( size_t IN size,
									   cl_prog_binary_desc* OUT types,
									   size_t* OUT size_ret
									   )
{
	if ( NULL != size_ret )
	{
		// TODO: Create supported list
		*size_ret = sizeof(gSupportedBinTypes);
	}

	if ( (0 == size) && (NULL == types) )
	{
		return CL_DEV_SUCCESS;
	}

	if( (NULL == types) || (size < sizeof(gSupportedBinTypes)))
	{
		return CL_DEV_INVALID_VALUE;
	}

	//currently support only user binaries
	memcpy_s(types, size, gSupportedBinTypes, sizeof(gSupportedBinTypes));

	return CL_DEV_SUCCESS;
}


cl_int ProgramService::getKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernel_id )
{
		return CL_DEV_INVALID_OPERATION;
}



cl_int ProgramService::getProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
						 size_t* OUT num_kernels_ret )
{
		return CL_DEV_INVALID_OPERATION;
}



cl_int ProgramService::getKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
					void* OUT value, size_t* OUT value_size_ret )
{
		return CL_DEV_INVALID_OPERATION;
}

