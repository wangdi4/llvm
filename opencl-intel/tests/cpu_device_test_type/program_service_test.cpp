// Copyright (c) 2006-2012 Intel Corporation
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
// program_service_test.cpp
///////////////////////////////////////////////////////////

#include "cpu_dev_test.h"
#include "cl_types.h"
#include "cpu_dev_limits.h"

#include <cl_sys_defines.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <math.h>

#ifdef _WIN32
#define SET_FPOS_T(var, val) (var) = (val)
#define GET_FPOS_T(var) var
#else
#define SET_FPOS_T(var, val) ((var).__pos = (val))
#define GET_FPOS_T(var) ((var).__pos)
#endif

extern volatile bool g_bBuildDone;

bool BuildProgram(const char* szFileName, cl_dev_program* prog)
{
	bool isDLL = false;
	unsigned int uiContSize = sizeof(cl_prog_container_header) + sizeof(cl_llvm_prog_header);

	FILE* pIRfile = NULL;
	FOPEN(pIRfile, szFileName, "rb");
	fpos_t fileSize;
	SET_FPOS_T(fileSize, 0);
	if ( NULL == pIRfile )
	{
		return false;
	}
	fseek(pIRfile, 0, SEEK_END);
	fgetpos(pIRfile, &fileSize);
	uiContSize += (unsigned int)GET_FPOS_T(fileSize);
	fclose(pIRfile);

	// Construct program container
	cl_prog_container_header* pCont = (cl_prog_container_header*)malloc(uiContSize);

	if ( NULL == pCont )
	{
		return false;
	}
	memset(pCont, 0, sizeof(cl_prog_container_header)+ sizeof(cl_llvm_prog_header));

	// Container mask
	memcpy((void*)pCont->mask, _CL_CONTAINER_MASK_, sizeof(pCont->mask));

	pCont->container_type = CL_PROG_CNT_PRIVATE;
	pCont->description.bin_type = CL_PROG_BIN_EXECUTABLE_LLVM;
	pCont->description.bin_ver_major = 1;
	pCont->description.bin_ver_minor = 1;
	pIRfile = NULL;
	FOPEN(pIRfile, szFileName, "rb");
	if ( NULL != pIRfile )
	{
		pCont->container_size = (unsigned int)GET_FPOS_T(fileSize) + sizeof(cl_llvm_prog_header);
		fread(((unsigned char*)pCont)+sizeof(cl_prog_container_header)+ sizeof(cl_llvm_prog_header), 1, (size_t)GET_FPOS_T(fileSize), pIRfile);
		fclose(pIRfile);
	}

	cl_int rc = dev_entry->clDevCreateProgram(uiContSize, pCont, CL_DEV_BINARY_USER, prog);
	g_bBuildDone = false;
    cl_build_status     build_status;
	rc = dev_entry->clDevBuildProgram(*prog, NULL, &build_status);

    printf(">>>>>>> The program %x was built, status%X\n", prog, build_status);
	g_bBuildDone = true;

	free(pCont);

	return CL_DEV_SUCCEEDED(rc);
}

bool CreateKernel(cl_dev_program prog, const char* szKernelName, cl_dev_kernel* kernel_id)
{
	cl_int rc = dev_entry->clDevGetKernelId(prog, szKernelName, kernel_id);
	if ( CL_DEV_FAILED(rc) )
	{
		printf("pclDevGetKernelId failed <%s>:<%X>\n", szKernelName, rc);
		return false;
	}

	return true;
}

bool BuildFromBinary_test(const char* szDLLName, unsigned int uiTotal, const char* szKernelName, unsigned int uiParams)
{

	// Start kernels test
	cl_dev_program	prog;
	cl_dev_kernel id;

	if ( !BuildProgram(szDLLName,  &prog) )
	{
		return false;
	}

	if ( !CreateKernel(prog, szKernelName,  &id) )
	{
		return false;
	}

	size_t stParamSize;

	// Test Kernel information
	cl_int rc = dev_entry->clDevGetKernelInfo(id, CL_DEV_KERNEL_PROTOTYPE, 0, NULL, &stParamSize);
	if ( CL_DEV_FAILED(rc) )
	{
		printf("pclDevGetKernelInfo failed[CL_DEV_KERNEL_PROTOTYPE] <%X>\n", rc);
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}
	uiParams *= sizeof(cl_kernel_argument);
	if ( stParamSize != uiParams )
	{
		printf("pclDevGetKernelInfo invalid parameters %u <-> %u\n", stParamSize, uiParams);
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}

	// Get required size
	size_t stReqdWGSize[3];
	rc = dev_entry->clDevGetKernelInfo(id, CL_DEV_KERNEL_WG_SIZE_REQUIRED,
		sizeof(stReqdWGSize), stReqdWGSize, NULL);
	if ( CL_DEV_FAILED(rc) )
	{
		printf("pclDevGetKernelInfo[CL_DEV_KERNEL_COMPILE_WG_SIZE] failed <%X>\n", rc);
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}
	for(int i=0;i<3;++i)
	{
		if ( 0 != stReqdWGSize[i] )
		{
			dev_entry->clDevReleaseProgram(prog);
			return false;
		}
	}

	// Get optimal size
	rc = dev_entry->clDevGetKernelInfo(id, CL_DEV_KERNEL_WG_SIZE,
		sizeof(stParamSize), &stParamSize, NULL);
	if ( CL_DEV_FAILED(rc) )
	{
		printf("pclDevGetKernelInfo[CL_DEV_KERNEL_WG_SIZE] failed <%X>\n", rc);
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}
	if ( stParamSize != 128 )
	{
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}

	// Implicit local memory size
	cl_ulong ullLocalSize;
	rc = dev_entry->clDevGetKernelInfo(id, CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE,
		sizeof(cl_ulong), &ullLocalSize, NULL);
	if ( CL_DEV_FAILED(rc) )
	{
		printf("pclDevGetKernelInfo[CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE] failed <%X>\n", rc);
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}
	if ( ullLocalSize != 0 )
	{
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}

	// private memory size
	ullLocalSize = 0;
	rc = dev_entry->clDevGetKernelInfo(id, CL_DEV_KERNEL_PRIVATE_SIZE,
		sizeof(cl_ulong), &ullLocalSize, NULL);
	if ( CL_DEV_FAILED(rc) )
	{
		printf("pclDevGetKernelInfo[CL_DEV_KERNEL_IMPLICIT_LOCAL_SIZE] failed <%X>\n", rc);
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}
	if ( ullLocalSize == 0 )
	{
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}

	// Get maximum size
	rc = dev_entry->clDevGetKernelInfo(id, CL_DEV_KERNEL_MAX_WG_SIZE,
		sizeof(stParamSize), &stParamSize, NULL);
	if ( CL_DEV_FAILED(rc) )
	{
		printf("pclDevGetKernelInfo[CL_DEV_KERNEL_WG_SIZE] failed <%X>\n", rc);
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}
	size_t maxWGSize = (size_t) MIN(CPU_MAX_WORK_GROUP_SIZE, (CPU_DEV_MAX_WG_PRIVATE_SIZE /ullLocalSize) );
	maxWGSize = (size_t)1 << (size_t)(logf((float)maxWGSize)/logf(2.f));
	if ( stParamSize != maxWGSize )
	{
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}

	// Get all kernels
	cl_uint uiKernels;
	rc = dev_entry->clDevGetProgramKernels(prog, 0, NULL, &uiKernels);
	if ( CL_DEV_FAILED(rc) )
	{
		printf("pclDevGetProgramKernels failed <%X>\n", rc);
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}
	if ( uiTotal != uiKernels )
	{
		printf("pclDevGetProgramKernels invalid amount of kernels %u <-> %u\n", uiTotal, uiKernels);
		dev_entry->clDevReleaseProgram(prog);
		return false;
	}

	cl_dev_kernel* pKernels = new cl_dev_kernel[uiKernels];
	memset(pKernels, 0, sizeof(cl_dev_kernel)*uiKernels);
	rc = dev_entry->clDevGetProgramKernels(prog, uiKernels, pKernels, &uiKernels);
	for(unsigned int i=1; i<=uiTotal; ++i)
	{
		if ( 0 == (unsigned int)(size_t)pKernels[i-1] )
		{
			printf("pclDevGetProgramKernels failed to fill kernel ID array", uiTotal, uiKernels);
			delete []pKernels;
			dev_entry->clDevReleaseProgram(prog);
			return false;
		}
	}

	delete []pKernels;
	dev_entry->clDevReleaseProgram(prog);

	return true;
}
