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
#include "program_binary.h"
#include <string.h>
#include <device.h>
using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program C'tor
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
// Program D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Program::~Program()
{
	// release source code resources
	if (m_uiStcStrCount > 0 && NULL != m_ppcSrcStrArr)
	{
		for (cl_uint ui=0; ui<m_uiStcStrCount; ++ui)
		{
			delete[] m_ppcSrcStrArr[ui];
		}
	}
	delete[] m_ppcSrcStrArr;
	delete[] m_pszSrcStrLengths;

	// release binaries resources
	map<cl_device_id, ProgramBinary*>::iterator it = m_mapBinaries.begin();
	while (it != m_mapBinaries.end())
	{
		ProgramBinary * pProgBin = it->second;
		delete pProgBin;
	}
	m_mapBinaries.clear();
	delete m_pLoggerClient;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::Release()
{
	return OCLObject::Release();
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
	InfoLog(m_pLoggerClient, L"AddSource enter. uiCount=%d, ppcStrings=%d, pszLengths=%d",uiCount, ppcStrings, pszLengths);
	if (m_uiStcStrCount > 0 || NULL != m_ppcSrcStrArr)
	{
		// source has allready added to this program
		ErrLog(m_pLoggerClient, L"m_uiStcStrCount > 0 || NULL != m_ppcSrcStrArr; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}
	// check if count is zero or if strings or any entry in strings is NULL
	if (0 == uiCount || NULL == ppcStrings || NULL == pszLengths)
	{
		ErrLog(m_pLoggerClient, L"0 == uiCount || NULL == ppcStrings || NULL == szLengths; return CL_INVALID_VALUE");
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
	InfoLog(m_pLoggerClient, L"m_pszSrcStrLengths = new size_t[%d]",m_uiStcStrCount);
	m_pszSrcStrLengths = new size_t[m_uiStcStrCount];
	if (NULL == m_pszSrcStrLengths)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_pszSrcStrLengths");
		return CL_OUT_OF_HOST_MEMORY;
	}
	for(cl_uint ui=0; ui<m_uiStcStrCount; ++ui)
	{
		m_pszSrcStrLengths[ui] = pszLengths[ui];
	}
	// allocate host memory for source code
	InfoLog(m_pLoggerClient, L"m_ppcSrcStrArr = new char * [%d]",m_uiStcStrCount);
	m_ppcSrcStrArr = new char * [m_uiStcStrCount];
	if (NULL == m_ppcSrcStrArr)
	{
		ErrLog(m_pLoggerClient, L"NULL == m_ppcSrcStrArr");
		delete[] m_pszSrcStrLengths;
		m_pszSrcStrLengths = NULL;
		return CL_OUT_OF_HOST_MEMORY;
	}
	for (cl_uint ui=0; ui<m_uiStcStrCount; ++ui)
	{
		InfoLog(m_pLoggerClient, L"m_ppcSrcStrArr[ui] = new char[%d] + 1]",pszLengths[ui]);
		m_ppcSrcStrArr[ui] = new char[pszLengths[ui] + 1];
		if (NULL == m_ppcSrcStrArr)
		{
			// free all previouse strings
			ErrLog(m_pLoggerClient, L"NULL == m_ppcSrcStrArr");
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
///////////////////////////////////////////////////////////////////////////////////////////////////
// AddBinary
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::AddBinary(Device * pDevice, cl_uint uiBinarySize, const void * pBinaryData)
{
	InfoLog(m_pLoggerClient, L"AddBinary enter. pDevice=%d, uiBinarySize=%d, pBinaryData=%d", pDevice, uiBinarySize, pBinaryData);
	cl_err_code clErrRet = CL_SUCCESS;

	if (NULL == pDevice)
	{
		return CL_INVALID_VALUE;
	}

	// check if the current device allready have binary assign for it
	map<cl_device_id,ProgramBinary*>::iterator it = m_mapBinaries.find((cl_device_id)pDevice->GetId());
	if (it != m_mapBinaries.end())
	{
		return CL_ERR_KEY_ALLREADY_EXISTS;
	}
	ProgramBinary * pProgBin = new ProgramBinary(uiBinarySize, pBinaryData, pDevice, &clErrRet);
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	m_mapBinaries[(cl_device_id)pDevice->GetId()] = pProgBin;
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// CheckDevices
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::CheckBinaries(cl_uint uiNumDevices, const cl_device_id * pclDevices)
{
	if (0 == uiNumDevices || NULL == pclDevices)
	{
		return CL_INVALID_DEVICE;
	}
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		map<cl_device_id,ProgramBinary*>::iterator it = m_mapBinaries.find(pclDevices[ui]);
		if (it == m_mapBinaries.end())
		{
			return CL_INVALID_DEVICE;
		}
		ProgramBinary * pProgBin = it->second;
		if (NULL == pProgBin)
		{
			return CL_INVALID_BINARY;
		}
		Device * pDevice = pProgBin->GetDevice();
		if (NULL == pDevice)
		{
			return CL_INVALID_DEVICE;
		}
		cl_err_code clErrRet = pDevice->CheckProgramBinary(pProgBin->GetSize(), pProgBin->GetData());
		if (CL_FAILED(clErrRet))
		{
			return CL_INVALID_BINARY;
		}
	}
	return CL_SUCCESS;
}