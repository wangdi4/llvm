// Copyright (c) 2006-2007 Intel Corporation
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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  fe_compiler.cpp
//  Implementation of the front-end compiler class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "fe_compiler.h"
#include "observer.h"

#include <cl_sys_defines.h>
#include <task_executor.h>

#if !defined(_WIN32)
#include <malloc.h>
#endif

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::FECompilerAPI;
using namespace Intel::OpenCL::TaskExecutor;

struct FECompileTask : public ITask
{
	cl_device_id					devId;
	FEBuildProgramDescriptor		BuildDesc;
	IFrontendBuildDoneObserver *	pFEObserver;
	LoggerClient *					m_pLoggerClient;
	IOCLFECompiler*					pFECompiler;

	void	Execute()
	{
		IOCLFEBuildProgramResult*	pResult;
		int err = pFECompiler->BuildProgram(&BuildDesc, &pResult);
		if ( 0 != err )
		{
			LOG_ERROR(TEXT("Front-End compilation failed = %x"), err);
		}
		pFEObserver->NotifyFEBuildDone(devId, pResult->GetIRSize(), pResult->GetIR(), pResult->GetErrorLog());
		pResult->Release();
	};

	void	Release()
	{
		delete []BuildDesc.pInput;
		delete this;
	}
};

FrontEndCompiler::FrontEndCompiler() : OCLObject<_cl_object>("FrontEndCompiler"), m_pszModuleName(NULL), m_pFECompiler(NULL), m_pLoggerClient(NULL)
{
}

FrontEndCompiler::~FrontEndCompiler()
{
	FreeResources();
}

cl_err_code FrontEndCompiler::Initialize(const char * psModuleName, const void *pDeviceInfo, size_t stDevInfoSize)
{
	FreeResources();

	INIT_LOGGER_CLIENT(L"FrontEndCompiler", LL_DEBUG);

	if ( !m_dlModule.Load(psModuleName) )
	{
		LOG_ERROR(TEXT("Can't find compiler module %s)"), psModuleName);
		return CL_COMPILER_NOT_AVAILABLE;
	}

	m_pfnCreateInstance = (fnCreateFECompilerInstance*)m_dlModule.GetFunctionPtrByName("CreateFrontEndInstance");
	if ( NULL == m_pfnCreateInstance )
	{
		LOG_ERROR(TEXT("%S"), TEXT("Can't find entry function"));
		return CL_COMPILER_NOT_AVAILABLE;
	}

	m_pszModuleName = STRDUP(psModuleName);

	cl_err_code err = m_pfnCreateInstance(pDeviceInfo, stDevInfoSize, &m_pFECompiler);
	if ( CL_FAILED(err) )
	{
		LOG_ERROR(TEXT("FECompiler::CreateInstance() failed with %x"), err);
	}

	return err;
}

void FrontEndCompiler::FreeResources()
{
	RELEASE_LOGGER_CLIENT;

	if ( NULL != m_pFECompiler )
	{
		m_pFECompiler->Release();
		m_pFECompiler = NULL;
	}

	if ( NULL != m_pszModuleName )
	{
		free((void*)m_pszModuleName);
		m_pszModuleName = NULL;
		m_dlModule.Close();
		m_pfnCreateInstance = NULL;
	}
}

cl_err_code FrontEndCompiler::BuildProgram(	cl_device_id			devId,
										cl_uint						uiStrCount,
										const char **				ppcSrcStrArr,
										size_t *					pszSrcStrLengths,
										const char *				psOptions,
										IFrontendBuildDoneObserver*	pBuildDoneObserver)
{
	//cl_start;
	LOG_DEBUG(TEXT("Enter BuildProgram(devId=%d, uiStcStrCount=%d, ppcSrcStrArr=%d, pszSrcStrLengths=%d, psOptions=%d, pBuildDoneObserver=%d)"),
		devId, uiStrCount, ppcSrcStrArr, pszSrcStrLengths, psOptions, pBuildDoneObserver);
	
	FECompileTask* pTask = new FECompileTask;
	if ( NULL == pTask )
	{
		LOG_ERROR(TEXT("%S"), TEXT("FECompileTask* pTask = new FECompileTask = NULL"));
		return CL_OUT_OF_HOST_MEMORY;
	}

	pTask->devId = devId;
	pTask->pFEObserver = pBuildDoneObserver;
	pTask->m_pLoggerClient = GET_LOGGER_CLIENT;
	pTask->pFECompiler = m_pFECompiler;

	// Fill build descriptor
	pTask->BuildDesc.pszOptions = psOptions;

	size_t stTotalSize = 0;
	for(unsigned int i=0; i<uiStrCount; ++i)
	{
		stTotalSize += pszSrcStrLengths[i];
	}

	// Prepare input buffer
	pTask->BuildDesc.pInput = new char[stTotalSize+1];	// Allocate another symbol for '\0'
	if ( NULL == pTask->BuildDesc.pInput )
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == pTask->BuildDesc.pInput"));
		delete pTask;
		return CL_OUT_OF_HOST_MEMORY;
	}

	// Copy sources to the new buffer
	char*	pBegin = (char*)pTask->BuildDesc.pInput;
	for(unsigned int i=0; i<uiStrCount; ++i)
	{
		MEMCPY_S(pBegin, stTotalSize, ppcSrcStrArr[i], pszSrcStrLengths[i]);
		stTotalSize -= pszSrcStrLengths[i];
		pBegin += pszSrcStrLengths[i];
	}
	*pBegin = '\0';

	unsigned int teErr =  TaskExecutor::GetTaskExecutor()->Execute(pTask);
	if ( 0 != teErr )
	{
		LOG_ERROR(TEXT("%d == TaskExecutor::GetTaskExecutor()->Execute(pTask)"), teErr);
		pTask->Release();
		return CL_OUT_OF_RESOURCES;
	}

	LOG_DEBUG(TEXT("%S"), TEXT("BuildProgram succeeded!"));
	//cl_return CL_SUCCESS;
	return CL_SUCCESS;
}
