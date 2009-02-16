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

#include "program_service.h"
#include "cl_logger.h"
#include "cl_dynamic_lib.h"
#include "dll_program.h"
#include "llvm_program.h"

#include <stdlib.h>
#include <limits.h>
#include <assert.h>

using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL;

// Static members
static cl_prog_binary_desc gSupportedBinTypes[] = 
{
	{CL_PROG_DLL_X86, 0, 0},
	{CL_PROG_BIN_LLVM, 0, 0},
};
static	unsigned int	gSupportedBinTypesCount = sizeof(gSupportedBinTypes)/sizeof(cl_prog_binary_desc);

ProgramService::ProgramService(cl_int devId, cl_dev_call_backs *devCallbacks, cl_dev_log_descriptor *logDesc) :
	m_iDevId(devId), m_progIdAlloc(1, UINT_MAX), m_kernelIdAlloc(1, UINT_MAX)
{
	memcpy_s(&m_sCallBacks, sizeof(cl_dev_call_backs), devCallbacks, sizeof(cl_dev_call_backs));
	if ( NULL == logDesc )
	{
		memset(&m_logDescriptor, 0, sizeof(cl_dev_log_descriptor));
	}
	else
	{
		memcpy_s(&m_logDescriptor, sizeof(cl_dev_log_descriptor), logDesc, sizeof(cl_dev_log_descriptor));
	}
	

	cl_int ret = m_logDescriptor.pfnclLogCreateClient(m_iDevId, L"CPU Device: Program Service", &m_iLogHandle);
	if(CL_DEV_SUCCESS != ret)
	{
		m_iLogHandle = 0;
	}

	InfoLog(m_logDescriptor, m_iLogHandle, L"CPUDevice: Program Service - Created");
}

ProgramService::~ProgramService()
{
	TProgramMap::iterator	it;

	// Go throught the map and remove all allocated programs
	for(it = m_mapPrograms.begin(); it != m_mapPrograms.end(); ++it)
	{
		DeleteProgramEntry(it->second);
	}

	m_progIdAlloc.Clear();
	m_kernelIdAlloc.Clear();
	m_mapPrograms.clear();

	InfoLog(m_logDescriptor, m_iLogHandle, L"CPUDevice: Program Service - Distructed");

	cl_int ret = m_logDescriptor.pfnclLogReleaseClient(m_iLogHandle);
	if(CL_DEV_SUCCESS != ret)
	{
		//TBD
	}
}

/****************************************************************************************************************
 CheckProgramBinary
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
cl_int ProgramService::CheckProgramBinary (size_t IN binSize, const void* IN bin)
{
	const cl_prog_container*	pProgCont = (cl_prog_container*)bin;

	InfoLog(m_logDescriptor, m_iLogHandle, L"CheckProgramBinary enter");

	// Check container size
	if ( sizeof(cl_prog_container) > binSize )
	{
		InfoLog(m_logDescriptor, m_iLogHandle, L"Invalid Binary Size was provided");
		return CL_DEV_INVALID_BINARY;
	}

	// Check container mask
	if ( memcmp(_CL_CONTAINER_MASK_, pProgCont->mask, sizeof(pProgCont->mask)) )
	{
		InfoLog(m_logDescriptor, m_iLogHandle, L"Invalid Container Mask was provided");
		return CL_DEV_INVALID_BINARY;
	}

	// Check suppoerted container type
	switch ( pProgCont->container_type )
	{
	// Supported containers
	case CL_PROG_CNT_PRIVATE:
		break;

	default:
		InfoLog(m_logDescriptor, m_iLogHandle, L"Invalid Container Type was provided");
		return CL_DEV_INVALID_BINARY;
	}

	// Check supported binary types
	switch ( pProgCont->description.bin_type )
	{
	// Supported program binaries
	case CL_PROG_DLL_X86:			// The container should contain a full path name to DLL file to load
	case CL_PROG_BIN_LLVM:			// The container should contain valid LLVM-IR
		break;

	case CL_PROG_OBJ_X86:			// The container should contain binary biffer of object file
	default:
		InfoLog(m_logDescriptor, m_iLogHandle, L"Invalid Container Type was provided<%0X>", pProgCont->description.bin_type);
		return CL_DEV_INVALID_BINARY;
	}

	return CL_DEV_SUCCESS;
}

/*******************************************************************************************************************
clDevCreateProgram
	Description
		Creates a device specific program entity (no build is performed).
	Input
		bin_size						Size of the binary buffer
		bin								A pointer to binary buffer that holds program container defined by cl_prog_container. 
		prop							Specifies the origin of the input binary. The values is defined by cl_dev_binary_prop.
	Output
		prog							A handle to created program object.
	Returns
		CL_DEV_SUCCESS					The function is executed successfully.
		CL_DEV_INVALID_BINARY			If the back-end compiler failed to process binary.
		CL_DEV_OUT_OF_MEMORY			If the device failed to allocate memory for the program
***********************************************************************************************************************/
cl_int ProgramService::CreateProgram( size_t IN binSize,
								   const void* IN bin,
								   cl_dev_binary_prop IN prop,
								   cl_dev_program* OUT prog
								   )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"CreateProgram enter");

	// Input parameters validation
	if(0 == binSize || NULL == bin)
	{
		InfoLog(m_logDescriptor, m_iLogHandle, L"Invalid binSize or bin parameters");
		return CL_DEV_INVALID_VALUE;
	}

	if ( NULL == prog )
	{
		InfoLog(m_logDescriptor, m_iLogHandle, L"Invalid prog parameter");
		return CL_DEV_INVALID_VALUE;
	}

	// If the origin of binary is user loaded
	// check for rightness
	if(prop == CL_DEV_BINARY_USER)
	{
		cl_int rc = CheckProgramBinary(binSize, bin);
		if ( CL_DEV_FAILED(rc) )
		{
			InfoLog(m_logDescriptor, m_iLogHandle, L"Check program binary failed");
			return rc;
		}
	}

	// Create new program
	const cl_prog_container*	pProgCont = (cl_prog_container*)bin;
	TProgramEntry*	pEntry		= new TProgramEntry;
	if ( NULL == pEntry )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Cann't allocate program entry");
		return CL_DEV_OUT_OF_MEMORY;
	}
	pEntry->pProgram = NULL;

	switch(pProgCont->description.bin_type)
	{
	case CL_PROG_DLL_X86:
		pEntry->pProgram = new DLLProgram;
		break;
	case CL_PROG_BIN_LLVM:
		pEntry->pProgram = new LLVMProgram;
	}

	if ( NULL == pEntry->pProgram )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to find approproiate program for type<%d>", pProgCont->description.bin_type);
		delete pEntry;
		return CL_DEV_INVALID_BINARY;
	}

	cl_int ret = pEntry->pProgram->CreateProgram(pProgCont);
	if ( CL_DEV_FAILED(ret) )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to create program from given container<%0X>", ret);
		delete pEntry;
		return CL_DEV_INVALID_BINARY;
	}

	// Allocate new program id
	unsigned int newProgId;
	if ( !m_progIdAlloc.AllocateHandle(&newProgId) )
	{
		delete pEntry;
		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to allocate new handle");
		return CL_DEV_OUT_OF_MEMORY;
	}

	m_muProgMap.Lock();
	m_mapPrograms[(cl_dev_program)newProgId] = pEntry;
	m_muProgMap.Unlock();

	*prog = (cl_dev_program)newProgId;

	return CL_DEV_SUCCESS;
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
cl_int ProgramService::BuildProgram( cl_dev_program OUT prog,
									const cl_char* IN options,
								    void* IN userData
								   )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"BuildProgram enter");

	TProgramMap::iterator	it;

	m_muProgMap.Lock();
	it = m_mapPrograms.find(prog);
	if( m_mapPrograms.end() == it )
	{
		m_muProgMap.Unlock();
		InfoLog(m_logDescriptor, m_iLogHandle, L"Requested program not found (%0X)", (unsigned int)prog);
		return CL_DEV_INVALID_PROGRAM;
	}

	TProgramEntry* pEntry = it->second;
	m_muProgMap.Unlock();

	cl_int iRet;

	iRet = pEntry->pProgram->BuildProgram(m_sCallBacks.pclDevBuildStatusUpdate, prog, userData);
	if ( CL_DEV_FAILED(iRet) )
	{
		InfoLog(m_logDescriptor, m_iLogHandle, L"Program(%d) build failed, Ret:%0X", (unsigned int)prog, iRet);
		return iRet;
	}

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
cl_int ProgramService::ReleaseProgram( cl_dev_program IN prog )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"ReleaseProgram enter");

	TProgramMap::iterator	it;

	m_muProgMap.Lock();
	it = m_mapPrograms.find(prog);
	if( m_mapPrograms.end() == it )
	{
		m_muProgMap.Unlock();
		InfoLog(m_logDescriptor, m_iLogHandle, L"Requested program not found (%0X)", (unsigned int)prog);
		return CL_DEV_INVALID_PROGRAM;
	}

	TProgramEntry* pEntry = it->second;
	m_mapPrograms.erase(it);
	m_muProgMap.Unlock();

	DeleteProgramEntry(pEntry);

	m_progIdAlloc.FreeHandle((unsigned int)prog);

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
cl_int ProgramService::UnloadCompiler()
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"UnloadCompiler enter");
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
cl_int ProgramService::GetProgramBinary( cl_dev_program IN prog,
										size_t IN size,
										void* OUT binary,
										size_t* OUT sizeRet
										)
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"GetProgramBinary enter");

	TProgramMap::iterator	it;

	// Access program map
	m_muProgMap.Lock();
	it = m_mapPrograms.find(prog);
	if( it == m_mapPrograms.end())
	{
		m_muProgMap.Unlock();
		InfoLog(m_logDescriptor, m_iLogHandle, L"Requested program not found (%0X)", (unsigned int)prog);
		return CL_DEV_INVALID_PROGRAM;
	}
	ICLDevProgram *pProg = it->second->pProgram;
	m_muProgMap.Unlock();

	if ( NULL != sizeRet )
	{
		*sizeRet = pProg->GetContainerSize();
	}

	if ( (0 == size) && (NULL == binary) )
	{
		return CL_DEV_SUCCESS;
	}

	if ( (NULL == binary) || (size < pProg->GetContainerSize()) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	return pProg->CopyContainer(binary, size);
}

cl_int ProgramService::GetBuildLog( cl_dev_program IN prog,
								  size_t IN size,
								  char* OUT log,
								  size_t* OUT sizeRet
								  )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"GetBuildLog enter");

	TProgramMap::iterator	it;

	// Access program map
	m_muProgMap.Lock();
	it = m_mapPrograms.find(prog);
	if( it == m_mapPrograms.end())
	{
		m_muProgMap.Unlock();
		InfoLog(m_logDescriptor, m_iLogHandle, L"Requested program not found (%0X)", (unsigned int)prog);
		return CL_DEV_INVALID_PROGRAM;
	}
	ICLDevProgram *pProg = it->second->pProgram;
	m_muProgMap.Unlock();

	size_t	stLogSize = strlen(pProg->GetBuildLog())+1;
	if ( (0 == size) && (NULL == log) )
	{
		if ( NULL == sizeRet )
		{
			return CL_DEV_INVALID_VALUE;
		}
		*sizeRet = stLogSize;
		return CL_DEV_SUCCESS;
	}

	if ( (NULL == log) || (size < stLogSize) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	memcpy(log, pProg->GetBuildLog(), stLogSize);
	if ( NULL != sizeRet )
	{
		*sizeRet = stLogSize;
	}
	
	return CL_DEV_SUCCESS;
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
cl_int ProgramService::GetSupportedBinaries( size_t IN size,
									   cl_prog_binary_desc* OUT types,
									   size_t* OUT sizeRet
									   )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"GetSupportedBinaries enter");
	if ( NULL != sizeRet )
	{
		// TODO: Create supported list
		*sizeRet = sizeof(gSupportedBinTypes);
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
	memcpy(types, gSupportedBinTypes, sizeof(gSupportedBinTypes));

	return CL_DEV_SUCCESS;
}


cl_int ProgramService::GetKernelId( cl_dev_program IN prog, const char* IN name, cl_dev_kernel* OUT kernelId )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"GetKernelId enter");

	if ( (NULL == name) || (NULL == kernelId) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	TProgramMap::const_iterator	it;

	// Access program map
	m_muProgMap.Lock();
	it = m_mapPrograms.find(prog);
	if( it == m_mapPrograms.end())
	{
		m_muProgMap.Unlock();
		InfoLog(m_logDescriptor, m_iLogHandle, L"Requested program not found (%0X)", (unsigned int)prog);
		return CL_DEV_INVALID_PROGRAM;
	}

	TProgramEntry *pEntry = it->second;
	m_muProgMap.Unlock();

	// Find if ID is already allocated
	TName2IdMap::const_iterator	nameIt;
	pEntry->muMap.Lock();
	nameIt = pEntry->mapKernels.find(name);
	if ( pEntry->mapKernels.end() != nameIt )
	{
		*kernelId = nameIt->second;
		pEntry->muMap.Unlock();
		return CL_DEV_SUCCESS;
	}

	// Retrive kernel from program
	cl_int iRet;
	const ICLDevKernel*	pKernel;

	iRet = pEntry->pProgram->GetKernel(name, &pKernel);
	if ( CL_DEV_FAILED(iRet) )
	{
		InfoLog(m_logDescriptor, m_iLogHandle, L"Requested kernel not found");
		return iRet;
	}

	// Allocate new global ID
	unsigned int	uiNewId = 0;
	m_kernelIdAlloc.AllocateHandle(&uiNewId);
	if ( 0 == uiNewId )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Cann't allocate kernel id");
		return CL_DEV_ERROR_FAIL;
	}

	// Add to program entry map
	pEntry->muMap.Lock();
	pEntry->mapKernels[name] = (cl_dev_kernel)uiNewId;
	pEntry->muMap.Unlock();

	// Add to global ID to Kernel map
	m_muKernelMap.Lock();
	m_mapKernels[(cl_dev_kernel)uiNewId] = pKernel;
	m_muKernelMap.Unlock();

	*kernelId = (cl_dev_kernel)uiNewId;

	return CL_DEV_SUCCESS;
}

cl_int ProgramService::GetProgramKernels( cl_dev_program IN prog, cl_uint IN num_kernels, cl_dev_kernel* OUT kernels,
						 cl_uint* OUT numKernelsRet )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"GetProgramKernels enter");

	TProgramMap::const_iterator	it;
	// Access program map
	m_muProgMap.Lock();
	it = m_mapPrograms.find(prog);
	if( it == m_mapPrograms.end())
	{
		m_muProgMap.Unlock();
		InfoLog(m_logDescriptor, m_iLogHandle, L"Requested program not found (%0X)", (unsigned int)prog);
		return CL_DEV_INVALID_PROGRAM;
	}

	TProgramEntry *pEntry = it->second;
	m_muProgMap.Unlock();

	unsigned int	uiNumProgKernels;
	cl_int			iRet;

	iRet = pEntry->pProgram->GetAllKernels(NULL, 0, &uiNumProgKernels);
	if ( CL_DEV_FAILED(iRet) )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to retrive number of kernels");
		return iRet;
	}

	// Check input parameters
	if ( (0==num_kernels) && (NULL==kernels) )
	{
		if ( NULL == numKernelsRet )
		{
			return CL_DEV_INVALID_VALUE;
		}

		*numKernelsRet = uiNumProgKernels;
		return CL_DEV_SUCCESS;
	}

	if ( (NULL==kernels) || (num_kernels < uiNumProgKernels) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	// Allocate buffer to store all kernels from program
	const ICLDevKernel*	*pKernels;
	pKernels = (const ICLDevKernel*	*)malloc(sizeof(ICLDevKernel*)*uiNumProgKernels);
	if ( NULL == pKernels )
	{
		ErrLog(m_logDescriptor, m_iLogHandle, L"Can't allocate memory for kernels");
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Retrieve kernels from program and store internally
	iRet = pEntry->pProgram->GetAllKernels(pKernels, uiNumProgKernels, &uiNumProgKernels);
	if ( CL_DEV_FAILED(iRet) )
	{
		free(pKernels);
		ErrLog(m_logDescriptor, m_iLogHandle, L"Failed to retrive kernels");
		return CL_DEV_ERROR_FAIL;
	}

	for(unsigned int i=0; i<uiNumProgKernels; ++i)
	{
		unsigned int uiKernelId = 0;

		// Check if kernel is already retrieved
		const char*	szKernelName = pKernels[i]->GetKernelName();
		TName2IdMap::const_iterator nameIt;
		pEntry->muMap.Lock();
		nameIt = pEntry->mapKernels.find(szKernelName);
		pEntry->muMap.Unlock();

		// Kernel already has ID ?
		if ( pEntry->mapKernels.end() != nameIt )
		{
			kernels[i] = nameIt->second;
			continue;
		}
		
		// Allocate new ID
		m_kernelIdAlloc.AllocateHandle(&uiKernelId);
		if ( 0 == uiKernelId )
		{
			free(pKernels);
			ErrLog(m_logDescriptor, m_iLogHandle, L"Failed allocate ID for kernel");
			if ( NULL != numKernelsRet )
			{
				*numKernelsRet = i;
			}
			return CL_DEV_ERROR_FAIL;
		}

		// Add new ID to program's Name2ID map
		pEntry->muMap.Lock();
		pEntry->mapKernels[szKernelName] = (cl_dev_kernel)uiKernelId;
		pEntry->muMap.Unlock();

		// Add to global ID2Kernel map
		m_muKernelMap.Lock();
		m_mapKernels[(cl_dev_kernel)uiKernelId] = pKernels[i];
		m_muKernelMap.Unlock();

		kernels[i] = (cl_dev_kernel)uiKernelId;
	}

	if ( NULL != numKernelsRet )
	{
		*numKernelsRet = uiNumProgKernels;
	}

	return CL_DEV_SUCCESS;
}



cl_int ProgramService::GetKernelInfo( cl_dev_kernel IN kernel, cl_dev_kernel_info IN param, size_t IN value_size,
					void* OUT value, size_t* OUT valueSizeRet )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"GetKernelInfo enter");

	// Find appropriate kernel
	TKernelMap::const_iterator	it;
	m_muKernelMap.Lock();
	it = m_mapKernels.find(kernel);
	if ( m_mapKernels.end() == it )
	{
		m_muKernelMap.Unlock();
		InfoLog(m_logDescriptor, m_iLogHandle, L"Requested kernel not found (%0X)", (unsigned int)kernel);
		return CL_DEV_INVALID_KERNEL;
	}
	const ICLDevKernel* pKernel = it->second;
	m_muKernelMap.Unlock();

	// Set value parameters
	size_t	stValSize;
	const void*	pValue;

	switch (param)
	{
	case CL_DEV_KERNEL_NAME:
		pValue = pKernel->GetKernelName();
		stValSize = strlen((const char*)pValue)+1;
		break;

	case CL_DEV_KERNEL_PROTOTYPE:
		pValue = pKernel->GetKernelArgs();
		stValSize = it->second->GetNumArgs()*sizeof(cl_kernel_argument);
		break;

	case CL_DEV_KERNEL_COMPILE_WG_SIZE:
		pValue = NULL;
		stValSize = 0;
		break;

	default:
		return CL_DEV_INVALID_VALUE;
	}

	if ( NULL != valueSizeRet )
	{
		*valueSizeRet = stValSize;
	}

	if ( (0 == value_size) && (NULL == value) )
	{
		if ( NULL == valueSizeRet )
		{
			return CL_DEV_INVALID_VALUE;
		}
		return CL_DEV_SUCCESS;
	}

	if ( (NULL == value) || (value_size < stValSize) )
	{
			return CL_DEV_INVALID_VALUE;
	}

	memcpy(value, pValue, stValSize);

	return CL_DEV_SUCCESS;
}

cl_int ProgramService::GetKernelObject( cl_dev_kernel IN kernel, const ICLDevKernel* OUT *pKernel )
{
	InfoLog(m_logDescriptor, m_iLogHandle, L"GetKernelInfo enter");

	// Find appropriate kernel
	TKernelMap::const_iterator	it;
	m_muKernelMap.Lock();
	it = m_mapKernels.find(kernel);
	if ( m_mapKernels.end() == it )
	{
		m_muKernelMap.Unlock();
		InfoLog(m_logDescriptor, m_iLogHandle, L"Requested kernel not found (%0X)", (unsigned int)kernel);
		return CL_DEV_INVALID_KERNEL;
	}
	if ( NULL != pKernel )
	{
		*pKernel = it->second;
	}

	m_muKernelMap.Unlock();

	return CL_DEV_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////////////
//	Private methods
void ProgramService::DeleteProgramEntry(TProgramEntry* pEntry)
{
	// Finnaly release the object
	delete pEntry->pProgram;

	// Free allocated kernel ids
	TName2IdMap::const_iterator lstIt;
	for (lstIt=pEntry->mapKernels.begin();lstIt!=pEntry->mapKernels.end();++lstIt)
	{
		m_kernelIdAlloc.FreeHandle((unsigned int)(lstIt->second));
	}
	pEntry->mapKernels.clear();

	delete pEntry;
}

