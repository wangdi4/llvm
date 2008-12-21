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
	Device * pDevice = new Device();
	m_pDevices->AddObject(pDevice);
	cl_err_code clErrRet = pDevice->InitDevice(L"CPUDevice.dll");
	if (CL_FAILED(clErrRet))
	{
		m_pDevices->RemoveObject(pDevice->GetId());
		delete pDevice;
		return clErrRet;
	}
	return CL_SUCCESS;
}

cl_err_code	PlatformModule::Release()
{
	cl_err_code clRes = CL_SUCCESS;
	OCLObjectInfoParam * pParam = NULL;

	// release devices
	cl_uint uiDevcount = m_pDevices->Count();
	for (cl_uint ui=0; ui<uiDevcount; ++ui)
	{
		Device *pDev = NULL;
		if (CL_SUCCEEDED(m_pDevices->GetObjectByIndex(ui, (OCLObject**)&pDev)))
		{
			pDev->Release();
			delete pDev;
		}
	}
	m_pDevices->Clear();

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
	if (NULL == m_pDevices)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	if (NULL == devices)
	{
		return CL_INVALID_VALUE;
	}
	cl_err_code clErrRet = CL_SUCCESS;
	cl_uint uiNumDevices = m_pDevices->Count();
	cl_uint uiRetNumDevices = 0; // this will be used for the num_devices return value;
	
	// prepare list for all devices
	cl_device_id * pDeviceIds = NULL;
	pDeviceIds = new cl_device_id[uiNumDevices];
	if (NULL == pDeviceIds)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	Device * pDevice = NULL;
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		// get device
		clErrRet = m_pDevices->GetObjectByIndex(ui, (OCLObject**)(&pDevice));
		if (CL_SUCCEEDED(clErrRet) && NULL != pDevice)
		{
			// get device type
			cl_device_type clType;
			cl_int iErrRet = pDevice->GetInfo(CL_DEVICE_TYPE, sizeof(cl_device_type), &clType, NULL);
			// check that the current device type satisfactory 
			if (iErrRet == 0 && ((clType & device_type) == clType))
			{
				pDeviceIds[uiRetNumDevices++] = (cl_device_id)pDevice->GetId();
			}
		}
	}
	if (uiRetNumDevices > num_entries)
	{
		delete[] pDeviceIds;
		return CL_INVALID_VALUE;
	}
	for (cl_uint ui=0; ui<uiRetNumDevices; ++ui)
	{
		devices[ui] = pDeviceIds[ui];
	}
	if (NULL != num_devices)
	{
		*num_devices = uiRetNumDevices;
	}
	
	delete[] pDeviceIds;
	return CL_SUCCESS;
}

cl_err_code	PlatformModule::GetDeviceInfo(cl_device_id device,
										  cl_device_info param_name, 
										  size_t param_value_size, 
										  void* param_value,
										  size_t* param_value_size_ret)
{
	if (NULL == m_pDevices)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	Device * pDevice = NULL;
	cl_err_code clErrRet = m_pDevices->GetOCLObject((cl_int)device, (OCLObject**)(&pDevice));
	if (CL_FAILED(clErrRet))
	{
		return CL_INVALID_DEVICE;
	}
	return pDevice->GetInfo(param_name, param_value_size, param_value, param_value_size_ret);
}