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

#include "platform_module.h"
#include <cl_object_info.h>
#include <cl_objects_map.h>
#include <cl_device_api.h>
#include "device.h"
#include <cl_config.h>
#include <assert.h>
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

const char PlatformModule::m_vPlatformInfoStr[] = "FULL_PROFILE";
const unsigned int PlatformModule::m_uiPlatformInfoStrSize = sizeof(m_vPlatformInfoStr) / sizeof(char);

const char PlatformModule::m_vPlatformVersionStr[] = "OpenCL 1.0 WINDOWS";
const unsigned int PlatformModule::m_uiPlatformVersionStrSize = sizeof(m_vPlatformVersionStr) / sizeof(char);

///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::PlatformModule
///////////////////////////////////////////////////////////////////////////////////////////////////
PlatformModule::PlatformModule()
{
	m_pLoggerClient = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::~PlatformModule
///////////////////////////////////////////////////////////////////////////////////////////////////
PlatformModule::~PlatformModule()
{

}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::InitDevices
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code PlatformModule::InitDevices(vector<string> devices, string defaultDevice)
{
	int numDevices = devices.size();
	for(int i=0; i<numDevices; ++i)
	{
		// create new device object
		Device * pDevice = new Device();
		// assign device in the objects map
		m_pDevices->AddObject(pDevice);
		string strDevice = devices[i];
		// get wchar_t from string
		size_t needed;
		::mbstowcs_s(&needed, NULL, 0, &strDevice[0], strDevice.length());
		std::wstring wstr;
		wstr.resize(needed);
		::mbstowcs_s(&needed, &wstr[0], wstr.length(), &strDevice[0], strDevice.length());
		const wchar_t *pout = wstr.c_str();
		// initialize device
		cl_err_code clErrRet = pDevice->InitDevice(pout);
		if (CL_FAILED(clErrRet))
		{
			m_pDevices->RemoveObject(pDevice->GetId(), NULL);
			delete pDevice;
			return clErrRet;
		}
		if (defaultDevice != "" && defaultDevice == strDevice)
		{
			m_pDefaultDevice = pDevice;
		}
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::Initialize
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	PlatformModule::Initialize(ConfigFile * pConfigFile)
{
	// initialize logger
	m_pLoggerClient = new LoggerClient(L"PlatformModule",LL_DEBUG);
	InfoLog(m_pLoggerClient, L"Platform module logger initialized");

	// initialize devices
	m_pDevices = new OCLObjectsMap();
	m_pDefaultDevice = NULL;
	string strDevices = pConfigFile->Read<string>(CL_CONFIG_DEVICES, "");
	vector<string> vectDevices;
	int numDevices = ConfigFile::tokenize(strDevices, vectDevices);
	if (numDevices == 0)
	{
		return CL_ERR_DEVICE_INIT_FAIL;
	}
	string strDefaultDevice = pConfigFile->Read<string>(CL_CONFIG_DEFAULT_DEVICE, "");
	return InitDevices(vectDevices, strDefaultDevice);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	PlatformModule::Release()
{
	InfoLog(m_pLoggerClient, L"Enter Release");

	cl_err_code clRes = CL_SUCCESS;
	OCLObjectInfoParam * pParam = NULL;
	Device *pDev = NULL;

	// release devices
	cl_uint uiDevcount = m_pDevices->Count();
	for (cl_uint ui=0; ui<uiDevcount; ++ui)
	{
		if (CL_SUCCEEDED(m_pDevices->GetObjectByIndex(ui, (OCLObject**)&pDev)))
		{
			pDev->Release();
			delete pDev;
		}
	}
	m_pDevices->Clear();

	InfoLog(m_pLoggerClient, L"Platform module logger release");
	delete m_pLoggerClient;

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetPlatformInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int	PlatformModule::GetPlatformInfo(cl_platform_info clParamName, 
										size_t szParamValueSize, 
										void* pParamValue, 
										size_t* pszParamValueSizeRet)
{
	InfoLog(m_pLoggerClient, L"Enter GetPlatformInfo (clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)", 
		clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	// both param_value and param_value_size_ret are null pointers - in this case there is no 
	// meaning to do anything
	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		ErrLog(m_pLoggerClient, L"NULL == pParamValue || NULL == pszParamValueSizeRet")
		return CL_INVALID_VALUE;
	}

	size_t szParamSize = 0;
	void * pValue = NULL;

	switch (clParamName)
	{
	case CL_PLATFORM_PROFILE:
		szParamSize = m_uiPlatformInfoStrSize + 1;
		pValue = (void*)m_vPlatformInfoStr;
		break;
	case CL_PLATFORM_VERSION:
		szParamSize = m_uiPlatformVersionStrSize + 1;
		pValue = (void*)m_vPlatformVersionStr;
		break;
	default:
		return CL_INVALID_VALUE;
	}

	if (NULL != pszParamValueSizeRet)
	{
		*pszParamValueSizeRet = szParamSize;
	}

	if (NULL != pParamValue)
	{
		if (szParamValueSize < szParamSize)
		{
			ErrLog(m_pLoggerClient, L"szParamValueSize (%d) < pszParamValueSizeRet (%d)", szParamValueSize, szParamSize)
			return CL_INVALID_VALUE;
		}
		memset(pParamValue, 0, szParamValueSize);
		memcpy_s(pParamValue, szParamValueSize, pValue, szParamValueSize - 1);
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetDeviceIDs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int	PlatformModule::GetDeviceIDs(cl_device_type clDeviceType,
									 cl_uint uiNumEntries, 
									 cl_device_id* pclDevices, 
									 cl_uint* puiNumDevices)
{
	InfoLog(m_pLoggerClient, L"Enter GetDeviceIDs (device_type=%d, num_entried=%d, pclDevices=%d, puiNumDevices=%d)", 
		clDeviceType, uiNumEntries, pclDevices, puiNumDevices);

	assert (NULL != m_pDevices);

	if (!(clDeviceType & CL_DEVICE_TYPE_DEFAULT)		&&
		!(clDeviceType & CL_DEVICE_TYPE_CPU)			&&
		!(clDeviceType & CL_DEVICE_TYPE_GPU)			&&
		!(clDeviceType & CL_DEVICE_TYPE_ACCELERATOR)	&&
		!(clDeviceType & CL_DEVICE_TYPE_ALL))
	{
		return CL_INVALID_DEVICE_TYPE;
	}

	if ((NULL == pclDevices && NULL == puiNumDevices) ||
		(NULL == pclDevices && uiNumEntries > 0))
	{
		InfoLog(m_pLoggerClient, L"NULL == pclDevices && NULL == puiNumDevices")
		return CL_INVALID_VALUE;
	}

	cl_err_code clErrRet = CL_SUCCESS;
	cl_uint uiNumDevices = m_pDevices->Count();
	cl_uint uiRetNumDevices = 0; // this will be used for the num_devices return value;
	Device ** ppDevices = NULL;
	cl_device_id * pDeviceIds = NULL;

	if (uiNumDevices == 0)
	{
		return CL_DEVICE_NOT_FOUND;
	}
	if (clDeviceType == CL_DEVICE_TYPE_DEFAULT && m_pDefaultDevice == NULL)
	{
		return CL_DEVICE_NOT_FOUND;
	}
	
	// prepare list for all devices
	ppDevices = new Device * [uiNumDevices];
	if (NULL == ppDevices)
	{
		ErrLog(m_pLoggerClient, L"can't allocate memory for devices (NULL == ppDevices)")
		return CL_OUT_OF_HOST_MEMORY;
	}
	clErrRet = m_pDevices->GetObjects(uiNumDevices, (OCLObject**)ppDevices, NULL);
	if (CL_FAILED(clErrRet))
	{
		return CL_ERR_OUT(clErrRet);
		delete[] ppDevices;
	}
	pDeviceIds = new cl_device_id[uiNumDevices];
	if (NULL == pDeviceIds)
	{
		ErrLog(m_pLoggerClient, L"can't allocate memory for device ids (NULL == pDeviceIds)")
		return CL_OUT_OF_HOST_MEMORY;
	}

	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		if (NULL != ppDevices[ui])
		{
			if ((clDeviceType & CL_DEVICE_TYPE_DEFAULT) && 
				ppDevices[ui]->GetId() == m_pDefaultDevice->GetId())
			{
				//found the default device
				pDeviceIds[uiRetNumDevices++] = (cl_device_id)ppDevices[ui]->GetId();
				continue;
			}
			if (clDeviceType == CL_DEVICE_TYPE_ALL)
			{
				pDeviceIds[uiRetNumDevices++] = (cl_device_id)ppDevices[ui]->GetId();
			}
			else
			{
				// get device type
				cl_device_type clType;
				cl_int iErrRet = ppDevices[ui]->GetInfo(CL_DEVICE_TYPE, sizeof(cl_device_type), &clType, NULL);
				// check that the current device type satisfactory 
				if (iErrRet == 0 && ((clType & clDeviceType) == clType))
				{
					pDeviceIds[uiRetNumDevices++] = (cl_device_id)ppDevices[ui]->GetId();
				}
			}
		}
	}
	delete[] ppDevices;

	if (uiRetNumDevices == 0)
	{
		delete[] pDeviceIds;
		return CL_DEVICE_NOT_FOUND;
	}

	if (NULL != pclDevices)
	{
		if (uiRetNumDevices > uiNumEntries)
		{
			delete[] pDeviceIds;
			return CL_INVALID_VALUE;
		}
		for (cl_uint ui=0; ui<uiRetNumDevices; ++ui)
		{
			pclDevices[ui] = pDeviceIds[ui];
		}
	}
	
	if (NULL != puiNumDevices)
	{
		*puiNumDevices = uiRetNumDevices;
	}
	
	delete[] pDeviceIds;
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetDeviceInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int	PlatformModule::GetDeviceInfo(cl_device_id clDevice,
									  cl_device_info clParamName, 
									  size_t szParamValueSize, 
									  void* pParamValue,
									  size_t* pszParamValueSizeRet)
{
	if (NULL == m_pDevices)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	Device * pDevice = NULL;
	cl_err_code clErrRet = m_pDevices->GetOCLObject((cl_int)clDevice, (OCLObject**)(&pDevice));
	if (CL_FAILED(clErrRet))
	{
		return CL_INVALID_DEVICE;
	}
	return pDevice->GetInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetDevice
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	PlatformModule::GetDevice(cl_device_id clDeviceId, Device ** ppDevice)
{
	InfoLog(m_pLoggerClient, L"PlatformModule::GetDevice enter. clDeviceId=%d, ppDevices=%d",clDeviceId, ppDevice);
	// check input parameters
	if (NULL == ppDevice)
	{
		ErrLog(m_pLoggerClient, L"ppDevices==NULL; return CL_INVALID_VALUE");
		return CL_INVALID_VALUE;
	}
	if (NULL == m_pDevices)
	{
		ErrLog(m_pLoggerClient, L"m_pDevices==NULL; return CL_ERR_INITILIZATION_FAILED");
		CL_ERR_INITILIZATION_FAILED;
	}
	// get the device from the devices list
	cl_err_code clErrRet = m_pDevices->GetOCLObject((cl_int)clDeviceId, (OCLObject**)ppDevice);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"m_pDevices->GetOCLObject(%d,%d) = %d", clDeviceId, ppDevice, clErrRet);
		return clErrRet;
	}
	if (NULL == *ppDevice)
	{
		ErrLog(m_pLoggerClient, L"*ppDevice==NULL; return CL_INVALID_VALUE");
		return CL_INVALID_VALUE;
	}
	return CL_SUCCESS;
}