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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  fe_compiler.cpp
//  Implementation of the front-end compiler class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "fe_compiler.h"
#include "observer.h"
#include "cl_sys_info.h"

#include <cl_sys_defines.h>
#include <task_executor.h>

#if !defined(_WIN32)
#include <malloc.h>
#endif

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::FECompilerAPI;
using namespace Intel::OpenCL::TaskExecutor;

FrontEndCompiler::FrontEndCompiler() : 
		OCLObject<_cl_object>(NULL, "FrontEndCompiler"),
		m_pfnCreateInstance(NULL),
		m_pszModuleName(NULL),
		m_pFECompiler(NULL),
		m_pLoggerClient(NULL)
{
}

FrontEndCompiler::~FrontEndCompiler()
{
	FreeResources();
}

cl_err_code FrontEndCompiler::Initialize(const char * psModuleName, const void *pDeviceInfo, size_t stDevInfoSize)
{
	FreeResources();

	INIT_LOGGER_CLIENT(TEXT("FrontEndCompiler"), LL_DEBUG);

	if ( !m_dlModule.Load(Intel::OpenCL::Utils::GetFullModuleNameForLoad(psModuleName)) )
	{
		LOG_ERROR(TEXT("Can't find compiler module %s)"), psModuleName);
		return CL_COMPILER_NOT_AVAILABLE;
	}

	m_pfnCreateInstance = (fnCreateFECompilerInstance*)m_dlModule.GetFunctionPtrByName("CreateFrontEndInstance");
	if ( NULL == m_pfnCreateInstance )
	{
		LOG_ERROR(TEXT("%s"), TEXT("Can't find entry function"));
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
        if (!m_bTerminate)
        {
		    m_pFECompiler->Release();
        }
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


cl_err_code FrontEndCompiler::CompileProgram(const char*    szProgramSource, 
                                             unsigned int   uiNumInputHeaders, 
                                             const char**   pszInputHeaders, 
                                             const char**   pszInputHeadersNames, 
                                             const char*    szOptions, 
                                             OUT char**     ppBinary, 
                                             OUT size_t*    puiBinarySize, 
                                             OUT char**     pszCompileLog) const
{
    LOG_DEBUG(TEXT("Enter CompileProgram(szProgramSource=%d, uiNumInputHeaders=%d, pszInputHeaders=%d, pszInputHeadersNames=%d, szOptions=%d, ppBinary=%d, puiBinarySize=%d, pszCompileLog=%d)"),
        szProgramSource, uiNumInputHeaders, pszInputHeaders, pszInputHeadersNames, szOptions, ppBinary, puiBinarySize, pszCompileLog);

    IOCLFEBinaryResult*         pResult;
    FECompileProgramDescriptor  compileDesc;

    compileDesc.pProgramSource = szProgramSource;
    compileDesc.uiNumInputHeaders = uiNumInputHeaders;
    compileDesc.pInputHeaders = pszInputHeaders;
    compileDesc.pszInputHeadersNames = pszInputHeadersNames;
    compileDesc.pszOptions = szOptions;

    int err = m_pFECompiler->CompileProgram(&compileDesc, &pResult);

    if ( 0 != err )
    {
        LOG_ERROR(TEXT("Front-End compilation failed = %x"), err);
        if (CL_OUT_OF_HOST_MEMORY == err) 
        {
            if (NULL != pResult) 
            {
                pResult->Release();
            }

            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    const char* errLog = pResult->GetErrorLog();

    if (NULL != errLog)
    {
        *pszCompileLog = new char[strlen(errLog) + 1];
        if (NULL != *pszCompileLog)
        {
            MEMCPY_S(*pszCompileLog, strlen(errLog) + 1, errLog, strlen(errLog) + 1);
        }
        else
        {
            pResult->Release();
            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    *puiBinarySize = pResult->GetIRSize();

    if (0 != *puiBinarySize)
    {
        assert(pResult->GetIR() != 0);
        *ppBinary = new char[*puiBinarySize];
        if (NULL != *ppBinary)
        {
            MEMCPY_S(*ppBinary, *puiBinarySize, pResult->GetIR(), *puiBinarySize);
        }
        else
        {
            pResult->Release();
            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    pResult->Release();

    return CL_SUCCESS;
}

cl_err_code FrontEndCompiler::LinkProgram(const void**  ppBinaries, 
                                          unsigned int  uiNumInputBinaries, 
                                          const size_t* puiBinariesSizes, 
                                          const char*   szOptions, 
                                          OUT char**    ppBinary, 
                                          OUT size_t*   puiBinarySize, 
                                          OUT char**    pszLinkLog,
                                          OUT bool*     pbIsLibrary) const
{
    LOG_DEBUG(TEXT("Enter CompileProgram(ppBinaries=%d, uiNumInputBinaries=%d, puiBinariesSizes=%d, szOptions=%d, ppBinary=%d, puiBinarySize=%d, pszLinkLog=%d)"),
        ppBinaries, uiNumInputBinaries, puiBinariesSizes, szOptions, ppBinary, puiBinarySize, pszLinkLog);

    IOCLFEBinaryResult*	        pResult;
    FELinkProgramsDescriptor    linkDesc;

    linkDesc.pBinaryContainers = ppBinaries;
    linkDesc.uiNumBinaries = uiNumInputBinaries;
    linkDesc.puiBinariesSizes = puiBinariesSizes;
    linkDesc.pszOptions = szOptions;

    int err = m_pFECompiler->LinkPrograms(&linkDesc, &pResult);

    if ( 0 != err )
    {
        LOG_ERROR(TEXT("Front-End compilation failed = %x"), err);
        if (CL_OUT_OF_HOST_MEMORY == err) 
        {
            if (NULL != pResult) 
            {
                pResult->Release();
            }

            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    const char* errLog = pResult->GetErrorLog();

    if (NULL != errLog)
    {
        *pszLinkLog = new char[strlen(errLog) + 1];
        if (NULL != *pszLinkLog)
        {
            MEMCPY_S(*pszLinkLog, strlen(errLog) + 1, errLog, strlen(errLog) + 1);
        }
        else
        {
            pResult->Release();
            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    *puiBinarySize = pResult->GetIRSize();

    if (0 != *puiBinarySize)
    {
        assert(pResult->GetIR() != 0);
        *ppBinary = new char[*puiBinarySize];
        if (NULL != *ppBinary)
        {
            MEMCPY_S(*ppBinary, *puiBinarySize, pResult->GetIR(), *puiBinarySize);
        }
        else
        {
            pResult->Release();
            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    if (NULL != pbIsLibrary)
    {
        *pbIsLibrary = pResult->IsLibrary();
    }

    pResult->Release();

    return CL_SUCCESS;
}

bool FrontEndCompiler::CheckCompileOptions(const char* szOptions, char** szUnrecognizedOptions) const
{
    return m_pFECompiler->CheckCompileOptions(szOptions, szUnrecognizedOptions);
}

bool FrontEndCompiler::CheckLinkOptions(const char* szOptions, char** szUnrecognizedOptions) const
{
  return m_pFECompiler->CheckLinkOptions(szOptions, szUnrecognizedOptions);
}

cl_err_code FrontEndCompiler::GetKernelArgInfo(const void*        pBin, 
                                               const char*        szKernelName, 
                                               FEKernelArgInfo*   *ppArgInfo) const
{
    LOG_DEBUG(TEXT("Enter GetKernelArgInfo(pBin=%p, szKernelName=<%s>, ppArgInfo=%p)"), (void*)pBin, szKernelName, (void*)ppArgInfo);

    if ( (NULL == pBin) || (NULL == szKernelName) || (NULL==ppArgInfo) )
    {
        return CL_INVALID_VALUE;
    }

    int err = m_pFECompiler->GetKernelArgInfo(pBin, szKernelName, ppArgInfo);

    LOG_DEBUG(TEXT("Exit GetKernelArgInfo(pBin=%p, szKernelName=<%s>, ppArgInfo=%p, err=%d)"), (void*)pBin, szKernelName, (void*)ppArgInfo, err);

    return err;
}