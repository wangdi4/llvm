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
//  Context.cpp
//  Implementation of the Class Context
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Context.h"
using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Context C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Context::Context(cl_context_properties clProperties, cl_uint uiNumDevices, Device **ppDevices, logging_fn pfnNotify, void *pUserData)
{
	::OCLObject();
	m_pfnNotify = NULL;
	m_pUserData = NULL;

	m_pLoggerClient = new LoggerClient(L"Context", LL_DEBUG);
	InfoLog(m_pLoggerClient, L"Context constructor enter");

	if (NULL == ppDevices || uiNumDevices <= 0)
	{
		return;
	}
	m_pDeviceIds = new cl_device_id[uiNumDevices];
	if (NULL == m_pDeviceIds)
	{
		return;
	}
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		cl_device_id iDeviceId = (cl_device_id)(*ppDevices)->GetId();
		m_pDeviceIds[ui] = iDeviceId;
		m_mapDevices[iDeviceId] = *ppDevices;
	}

	m_clContextProperties = clProperties;
	m_pfnNotify = pfnNotify;
	m_pUserData = pUserData;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Context D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Context::~Context()
{
	InfoLog(m_pLoggerClient, L"Context destructor enter");
	delete[] m_pDeviceIds;
	m_mapDevices.clear();

	delete m_pLoggerClient;
	m_pLoggerClient = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	InfoLog(m_pLoggerClient, L"Context::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d", param_name, param_value_size, param_value, param_value_size_ret);
	
	if (NULL == param_value && NULL == param_value_size_ret)
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamValueSize = 0;
	void * pValue = NULL;
	
	switch ( (cl_context_info)param_name )
	{

	case CL_CONTEXT_REFERENCE_COUNT:
		szParamValueSize = sizeof(cl_uint);
		pValue = &m_uiRefCount;
		break;
	case CL_CONTEXT_DEVICES:
		szParamValueSize = sizeof(cl_device_id) * m_mapDevices.size();
		pValue = m_pDeviceIds;
		break;
	case CL_CONTEXT_PROPERTIES:
		szParamValueSize = sizeof(cl_context_properties);
		pValue = &m_clContextProperties;
		break;
	default:
		ErrLog(m_pLoggerClient, L"param_name (=%d) isn't valid", param_name);
		return CL_INVALID_VALUE;
	}
	// if param_value == NULL return only param value size
	if (NULL == param_value)
	{
		*param_value_size_ret = szParamValueSize;
		return CL_SUCCESS;
	}
	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (param_value_size < szParamValueSize)
	{
		ErrLog(m_pLoggerClient, L"param_value_size (=%d) < szParamValueSize (=%d)", param_value_size, szParamValueSize);
		return CL_INVALID_VALUE;
	}
	memcpy_s(param_value, szParamValueSize, pValue, param_value_size);
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Context::Release()
{
	cl_err_code clErr = OCLObject::Release();
	if (CL_FAILED(clErr))
	{
		return clErr;
	}
	if (0 == m_uiRefCount)
	{
		// TODO check resources
	}
	return CL_SUCCESS;

}