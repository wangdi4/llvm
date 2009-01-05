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
//  program.cpp
//  Implementation of the Class program
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "program.h"
#include <string.h>
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Program::Program(Context * pContext)
{
	::OCLObject();

	m_pLoggerClient = new LoggerClient(L"program",LL_DEBUG);
	m_pContext = pContext;
	m_uiStcStrCount = 0;
	m_ppcSrcStrArr = NULL;
	m_pszSrcStrLengths = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Program::~Program()
{
	if (m_uiStcStrCount > 0 && NULL != m_ppcSrcStrArr)
	{
		for (cl_uint ui=0; ui<m_uiStcStrCount; ++ui)
		{
			delete[] m_ppcSrcStrArr[ui];
		}
	}
	delete[] m_ppcSrcStrArr;
	delete[] m_pszSrcStrLengths;

	delete m_pLoggerClient;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// AddSource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::AddSource(cl_uint uiCount, const char ** ppcStrings, const size_t * pszLengths)
{
	if (m_uiStcStrCount > 0 || NULL != m_ppcSrcStrArr || NULL != pszLengths)
	{
		// source has allready added to this program
		return CL_ERR_INITILIZATION_FAILED;
	}
	// check if count is zero or if strings or any entry in strings is NULL
	if (0 == uiCount || NULL == ppcStrings || NULL == pszLengths)
	{
		ErrLog(m_pLoggerClient, L"0 == uiCount || NULL == ppcStrings || NULL == szLengths");
		return CL_INVALID_VALUE;
	}
	for (cl_uint ui=0; ui<uiCount; ++ui)
	{
		if (NULL == ppcStrings[ui] || NULL == pszLengths[ui])
		{
			ErrLog(m_pLoggerClient, L"ppcStrings[%d] == NULL || szLengths[%d] == NULL", ui, ui);
			return CL_INVALID_VALUE;
		}
	}
	m_uiStcStrCount = uiCount;
	// allocate host memory for source code
	m_pszSrcStrLengths = new size_t[m_uiStcStrCount];
	if (NULL == m_uiStcStrCount)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	for(cl_uint ui=0; ui<m_uiStcStrCount; ++ui)
	{
		m_pszSrcStrLengths[ui] = pszLengths[ui];
	}
	// allocate host memory for source code
	m_ppcSrcStrArr = new char * [m_uiStcStrCount];
	if (NULL == m_ppcSrcStrArr)
	{
		delete[] m_pszSrcStrLengths;
		m_pszSrcStrLengths = NULL;
		return CL_OUT_OF_HOST_MEMORY;
	}
	for (cl_uint ui=0; ui<m_uiStcStrCount; ++ui)
	{
		m_ppcSrcStrArr[ui] = new char[pszLengths[ui] + 1];
		if (NULL == m_ppcSrcStrArr)
		{
			// free all previouse strings
			for (cl_uint uj=0; uj<ui; ++uj)
			{
				delete[] m_ppcSrcStrArr[uj];
			}
			delete[] m_ppcSrcStrArr;
			m_ppcSrcStrArr = NULL;
			delete[] m_pszSrcStrLengths;
			m_pszSrcStrLengths = NULL;
			return CL_OUT_OF_HOST_MEMORY;
		}
		strcpy_s(m_ppcSrcStrArr[ui], pszLengths[ui] + 1, ppcStrings[ui]);
	}
	return CL_SUCCESS;
}