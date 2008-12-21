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

///////////////////////////////////////////////////////////
//  PlatformModule.cpp
//  Implementation of the Class PlatformModule
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "PlatformModule.h"
using namespace Intel::OpenCL::Framework;

const char PlatformModule::m_vPlatformInfoStr[] = "FULL_PROFILE";
const unsigned int PlatformModule::m_uiPlatformInfoStrSize = sizeof(m_vPlatformInfoStr) / sizeof(char);

const char PlatformModule::m_vPlatformVersionStr[] = "OpenCL 1.0 WINDOWS";
const unsigned int PlatformModule::m_uiPlatformVersionStrSize = sizeof(m_vPlatformVersionStr) / sizeof(char);


PlatformModule::PlatformModule()
{
	m_pPlatformLoggerClient = NULL;
}


PlatformModule::~PlatformModule()
{

}

cl_err_code	PlatformModule::Initialize()
{
	// initialize logger
	m_pPlatformLoggerClient = new LoggerClient(L"PlatformModule",LL_DEBUG);
	InfoLog(m_pPlatformLoggerClient,L"Platform module logger initialized");

	// initialize paltform info
	m_pObjectInfo = new OCLObjectInfo();
	
	if (m_pObjectInfo != NULL)
	{
		//m_pObjectInfo->SetString(CL_PLATFORM_PROFILE, m_uiPlatformInfoStrSize, m_vPlatformInfoStr);
		OCLObjectInfoParam * pParam = new OCLObjectInfoParam(CL_PLATFORM_PROFILE, m_uiPlatformInfoStrSize, (void*) m_vPlatformInfoStr);
		m_pObjectInfo->SetParam(CL_PLATFORM_PROFILE, pParam);
		//m_pObjectInfo->SetString(CL_PLATFORM_VERSION, m_uiPlatformVersionStrSize, m_vPlatformVersionStr);
		pParam = new OCLObjectInfoParam(CL_PLATFORM_VERSION, m_uiPlatformVersionStrSize, (void*) m_vPlatformVersionStr);
		m_pObjectInfo->SetParam(CL_PLATFORM_VERSION, pParam);
	}

	// initialize devices
	m_pDevices = new OCLObjectsMap();

	return CL_SUCCESS;
}

cl_err_code	PlatformModule::Release()
{
	cl_err_code clRes = CL_SUCCESS;
	OCLObjectInfoParam * pParam = NULL;
	
	if (NULL != m_pObjectInfo)
	{
		clRes = m_pObjectInfo->GetParam(CL_PLATFORM_PROFILE, &pParam);
		if (CL_SUCCEEDED(clRes))
		{
			delete pParam;
		}
		clRes = m_pObjectInfo->GetParam(CL_PLATFORM_VERSION, &pParam);
		if (CL_SUCCEEDED(clRes))
		{
			delete pParam;
		}
	}

	InfoLog(m_pPlatformLoggerClient,L"Platform module logger release");
	delete m_pPlatformLoggerClient;

	return CL_SUCCESS;
}

cl_err_code	PlatformModule::GetPlatformInfo(cl_platform_info param_name,
											size_t param_value_size, 
											void* param_value, 
											size_t* param_value_size_ret)
{
	if (NULL == m_pObjectInfo)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	if (NULL == param_value || NULL == param_value_size_ret)
	{
		return CL_INVALID_VALUE;
	}
	
	OCLObjectInfoParam *pParam = NULL;
	cl_err_code clRes = m_pObjectInfo->GetParam(param_name, &pParam);
	if (CL_SUCCEEDED(clRes))
	{
		if (param_value_size < pParam->GetSize())
		{
			return CL_INVALID_VALUE;
		}
		memcpy_s(param_value, param_value_size, pParam->GetValue(), pParam->GetSize());
		*param_value_size_ret = pParam->GetSize();
		return CL_SUCCESS;
	}
	return CL_INVALID_VALUE;
}

cl_err_code	PlatformModule::GetDeviceIDs(cl_device_type device_type,
										 cl_uint num_entries, 
										 cl_device_id* devices, 
										 cl_uint* num_devices)
{
	return CL_ERR_NOT_IMPLEMENTED;
}

cl_err_code	PlatformModule::clGetDeviceInfo(cl_device_id device,
											cl_device_info param_name, 
											size_t param_value_size, 
											void* param_value,
											size_t* param_value_size_ret)
{
	return CL_ERR_NOT_IMPLEMENTED;
}