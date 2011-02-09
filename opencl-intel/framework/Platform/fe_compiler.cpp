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

#if !defined(_WIN32)
#include <malloc.h>
#endif

#if defined (_WIN32)
#define STRDUP(src) (_strdup(src))
#else
#define STRDUP(src) (strdup(src))
#endif

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

struct FECompileTask
{
	cl_device_id					devId;
	FEBuildProgramDesc*				pBuildDesc;
	IFrontendBuildDoneObserver *	pFEObserver;
	LoggerClient *					pLoggerClient;
};

FECompiler::FECompiler() : m_pszModuleName(NULL), m_fnBuild(NULL), m_pLoggerClient(NULL)
{
}

FECompiler::~FECompiler()
{
	FreeResources();
}

cl_err_code FECompiler::Initialize(const char * psModuleName)
{
	FreeResources();

	INIT_LOGGER_CLIENT(L"FECompiler", LL_DEBUG);

	if ( !m_dlModule.Load(psModuleName) )
	{
		return CL_COMPILER_NOT_AVAILABLE;
	}

	m_pszModuleName = STRDUP(psModuleName);
	m_fnBuild = (fn_FEBuildProgram*)m_dlModule.GetFunctionPtrByName("clFEBuildProgram");
	if ( NULL == m_fnBuild )
	{
		Release();
		return CL_COMPILER_NOT_AVAILABLE;
	}

	return CL_SUCCESS;
}

void FECompiler::FreeResources()
{
	RELEASE_LOGGER_CLIENT;

	if ( NULL != m_pszModuleName )
	{
		free((void*)m_pszModuleName);
		m_pszModuleName = NULL;
		m_dlModule.Close();
		m_fnBuild = NULL;
	}
}

cl_err_code FECompiler::BuildProgram(	cl_device_id				devId,
										cl_uint						uiStcStrCount,
										const char **				ppcSrcStrArr,
										size_t *					pszSrcStrLengths,
										const char *				psOptions,
										IFrontendBuildDoneObserver*	pBuildDoneObserver)
{
	//cl_start;
	LOG_DEBUG(TEXT("Enter BuildProgram(devId=%d, uiStcStrCount=%d, ppcSrcStrArr=%d, pszSrcStrLengths=%d, psOptions=%d, pBuildDoneObserver=%d)"),
		devId, uiStcStrCount, ppcSrcStrArr, pszSrcStrLengths, psOptions, pBuildDoneObserver);
	
	FECompileTask* pTask = new FECompileTask;
	if ( NULL == pTask )
	{
		LOG_ERROR(TEXT("%S"), TEXT("FECompileTask* pTask = new FECompileTask = NULL"));
		return CL_OUT_OF_HOST_MEMORY;
	}

	FEBuildProgramDesc*	pBuildDesc = new FEBuildProgramDesc;
	if ( NULL == pBuildDesc )
	{
		LOG_ERROR(TEXT("%S"), TEXT("FEBuildProgramDesc*	pBuildDesc = new FEBuildProgramDesc"));
		delete pTask;
		return CL_OUT_OF_HOST_MEMORY;
	}

	pTask->devId = devId;
	pTask->pFEObserver = pBuildDoneObserver;
	pTask->pLoggerClient = GET_LOGGER_CLIENT;
	pTask->pBuildDesc = pBuildDesc;

	// Fill build descriptor
	pBuildDesc->uiLineCount = uiStcStrCount;
	pBuildDesc->ppsLineArray = ppcSrcStrArr;
	pBuildDesc->pLengths = pszSrcStrLengths;
	pBuildDesc->pszOptions = psOptions;
	pBuildDesc->pCallBack = BuildNotifyCallBack;
	pBuildDesc->pData = pTask;

	int iBuildRes = m_fnBuild(pBuildDesc);
	if ( 0 != iBuildRes )
	{
		LOG_ERROR(TEXT("%S"), TEXT("0 != m_fnBuild(uiStcStrCount, ppcSrcStrArr, pszSrcStrLengths, psOptions, &BuildNotifyCallBack, pTask)"));
		return CL_OUT_OF_RESOURCES;
	}

	LOG_DEBUG(TEXT("%S"), TEXT("BuildProgram succeeded!"));
	//cl_return CL_SUCCESS;
	return CL_SUCCESS;
}

void FECompiler::BuildNotifyCallBack(void* pData, void* pBuffer, size_t stBufferSize, int iStatus, const char* szErrLog)
{
	//cl_start;
	FECompileTask* pTask = (FECompileTask*)pData;

	//DbgLogA(pTask->pLoggerClient,
	//	"Enter BuildNotifyCallBack (pData (void*)=%d, pBuffer (void*)=%d, stBufferSize (size_t)=%d, iStatus (int) = %d, szErrLog (const char*)=%s)", 
	//	pData, pBuffer, stBufferSize, iStatus, szErrLog);

	pTask->pFEObserver->NotifyFEBuildDone(pTask->devId, stBufferSize, pBuffer, szErrLog);

	// Free memory objects
	delete pTask->pBuildDesc;
	delete pTask;
	//cl_return;
	return;
}
