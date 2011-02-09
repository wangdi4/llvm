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
#include <ocl_config.h>
#if defined (_WIN32)
#include "gl_shr_utils.h"
#endif
#include <cl_object_info.h>
#include <cl_objects_map.h>
#include <cl_device_api.h>
#include "Device.h"
#include "fe_compiler.h"
#include "cl_sys_defines.h"
#include <assert.h>
#include <malloc.h>

#define USE_COMPILER
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

const char PlatformModule::m_vPlatformInfoStr[] = "FULL_PROFILE";
const unsigned int PlatformModule::m_uiPlatformInfoStrSize = sizeof(m_vPlatformInfoStr) / sizeof(char);

#if defined (_WIN32)
const char PlatformModule::m_vPlatformVersionStr[] = "OpenCL 1.1 WINDOWS";
const unsigned int PlatformModule::m_uiPlatformVersionStrSize = sizeof(m_vPlatformVersionStr) / sizeof(char);

const char PlatformModule::m_vPlatformNameStr[] = "Intel OpenCL";
const unsigned int PlatformModule::m_uiPlatformNameStrSize = sizeof(m_vPlatformNameStr) / sizeof(char);
#else
const char PlatformModule::m_vPlatformVersionStr[] = "OpenCL 1.1 LINUX";
const unsigned int PlatformModule::m_uiPlatformVersionStrSize = sizeof(m_vPlatformVersionStr) / sizeof(char);

const char PlatformModule::m_vPlatformNameStr[] = "OPENCL_INTEL_LINUX";
const unsigned int PlatformModule::m_uiPlatformNameStrSize = sizeof(m_vPlatformNameStr) / sizeof(char);
#endif
const char PlatformModule::m_vPlatformVendorStr[] = "Intel Corporation";
const unsigned int PlatformModule::m_uiPlatformVendorStrSize = sizeof(m_vPlatformVendorStr) / sizeof(char);

const char PlatformModule::m_vPlatformExtensionsStr[] = "cl_khr_icd";
const unsigned int PlatformModule::m_uiPlatformExtensionsStrSize = sizeof(m_vPlatformExtensionsStr) / sizeof(char);


///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::PlatformModule
///////////////////////////////////////////////////////////////////////////////////////////////////
PlatformModule::PlatformModule()
{
	m_ppDevices = NULL;
	m_uiDevicesCount = 0;
	m_clPlatformIds[0] = NULL;
	m_clPlatformIds[1] = NULL;
	m_pOclEntryPoints = NULL;
	// initialize logger
	INIT_LOGGER_CLIENT(L"PlatformModule", LL_DEBUG);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::~PlatformModule
///////////////////////////////////////////////////////////////////////////////////////////////////
PlatformModule::~PlatformModule()
{
	RELEASE_LOGGER_CLIENT;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::InitDevices
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code PlatformModule::InitDevices(const vector<string>& devices, const string& defaultDevice)
{
	m_uiDevicesCount = devices.size();

	m_ppDevices = new Device * [m_uiDevicesCount];
	if (NULL == m_ppDevices)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

#if defined (_WIN32) || defined (USE_COMPILER)
	cl_uint uiNumFECompilers = m_pFECompilers->Count();
	assert(m_uiDevicesCount == uiNumFECompilers);

    OCLObject<_cl_object>** ppFECompilers = new OCLObject<_cl_object>*[uiNumFECompilers];
	if (!ppFECompilers)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

    m_pFECompilers->GetObjects(uiNumFECompilers, ppFECompilers, NULL);
#endif
	for(unsigned int ui=0; ui<m_uiDevicesCount; ++ui)
	{
		// create new device object
		Device * pDevice = new Device();
		if (!pDevice)
		{
#if defined (_WIN32) || defined (USE_COMPILER)
			delete[] ppFECompilers;
#endif
			return CL_OUT_OF_HOST_MEMORY;
		}

		const string& strDevice = devices[ui];

		cl_err_code clErrRet = pDevice->InitDevice(strDevice.c_str(), m_pOclEntryPoints);

		if (CL_FAILED(clErrRet))
		{
			pDevice->Release();
#if defined (_WIN32) || defined (USE_COMPILER)
			delete[] ppFECompilers;
#endif
			return clErrRet;
		}

		// assign device in the objects map
		m_pDevices->AddObject(pDevice);
		m_ppDevices[ui] = pDevice;

		if (defaultDevice != "" && defaultDevice == strDevice)
		{
			m_pDefaultDevice = pDevice;
		}
#if defined (_WIN32) || defined (USE_COMPILER)
		FECompiler* pFECompiler = dynamic_cast<FECompiler*>(ppFECompilers[ui]);
		pDevice->SetFECompiler(pFECompiler);
#endif
	}

#if defined (_WIN32) || defined (USE_COMPILER)
	delete[] ppFECompilers;
#endif
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::InitFECompilers
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code PlatformModule::InitFECompilers(const vector<string>& compilers, const string& defaultCompiler)
{
	assert(compilers.size() <= CL_MAX_INT32);
	int numCompilers = (int)compilers.size();
	for(int i=0; i<numCompilers; ++i)
	{
		// create new front-end compiler object
		FECompiler * pFECompiler = new FECompiler();
		if (!pFECompiler)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}

		const string& strCompiler = compilers[i];

		cl_err_code clErrRet = pFECompiler->Initialize(strCompiler.c_str());
		if (CL_FAILED(clErrRet))
		{
			pFECompiler->Release();
			return clErrRet;
		}
		// assign compiler in the objects map
		m_pFECompilers->AddObject((OCLObject<_cl_object>*)pFECompiler);
		if (defaultCompiler != "" && defaultCompiler == strCompiler)
		{
			m_pDefaultFECompiler = pFECompiler;
		}
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::ReleaseFECompilers
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code PlatformModule::ReleaseFECompilers()
{
	m_pFECompilers->ReleaseAllObjects();
	m_pDefaultFECompiler = NULL;
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::Initialize
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	PlatformModule::Initialize(ocl_entry_points * pOclEntryPoints, OCLConfig * pConfig)
{
	LOG_INFO(TEXT("%S"), TEXT("Platform module logger initialized"));

	m_pOclEntryPoints = pOclEntryPoints;
	m_clPlatformIds[0] = new _cl_platform_id_int;
	m_clPlatformIds[0]->dispatch = m_pOclEntryPoints;

	// initialize devices
	m_pDevices = new OCLObjectsMap<_cl_device_id_int>();
	m_pDefaultDevice = NULL;

	// get device agents dll names from configuration file
	string strDefaultDevice;
	vector<string> strDevices = pConfig->GetDevices(strDefaultDevice);
	if (strDevices.size() == 0)
	{
		return CL_ERR_DEVICE_INIT_FAIL;
	}

	// initialise front-end compilers
	m_pFECompilers = new OCLObjectsMap<_cl_object>();
	m_pDefaultFECompiler = NULL;

	// get front-end compilers dll names from configuration file
	string strDefaultFeCompiler;
	vector<string> strFeCompilers;
#if defined (_WIN32) || defined (USE_COMPILER)
	strFeCompilers = pConfig->GetFeCompilers(strDefaultFeCompiler);
#endif
	if (0 != strFeCompilers.size())
	{
	    cl_err_code clErr = InitFECompilers(strFeCompilers, strDefaultFeCompiler);
	    if (CL_FAILED(clErr))
	    {
			LOG_CRITICAL(TEXT("%S"), TEXT("Failed to initialize front-end compilers"));
		    //return clErr;
	    }
    }
	cl_err_code clErr = InitDevices(strDevices, strDefaultDevice);
	return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	PlatformModule::Release()
{
	LOG_INFO(TEXT("%S"), TEXT("Enter Release"));

	// release front-end compilers
	ReleaseFECompilers();

	// release devices
	m_pDevices->ReleaseAllObjects();
	m_pDefaultDevice = NULL;

	if (NULL != m_ppDevices)
	{
		delete m_ppDevices;
		m_ppDevices = NULL;
	}

	LOG_INFO(TEXT("%S"), TEXT("Platform module logger release"));
	RELEASE_LOGGER_CLIENT;

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetPlatformIDs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code PlatformModule::GetPlatformIDs(cl_uint uiNumEntries,
										   cl_platform_id * pclPlatforms,
										   cl_uint * puiNumPlatforms)
{
	LOG_INFO(TEXT("Enter GetPlatformIDs. (uiNumEntries=%d, pclPlatforms=%d, puiNumPlatforms=%d)"),
		uiNumEntries, pclPlatforms, puiNumPlatforms);

	if ( ((0 == uiNumEntries) && (NULL != pclPlatforms)) ||
		 ((NULL == puiNumPlatforms) && (NULL == pclPlatforms)) )
	{
		LOG_ERROR(TEXT("%S"), TEXT("((0 == uiNumEntries) && (NULL != pclPlatforms)) || ((NULL == puiNumPlatforms) && (NULL != pclPlatforms))"));
		return CL_INVALID_VALUE;
	}

	if ( uiNumEntries > 0 )
	{
		*pclPlatforms = m_clPlatformIds[0];
	}


	if (NULL != puiNumPlatforms)
	{
		*puiNumPlatforms = 1;
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetPlatformInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int	PlatformModule::GetPlatformInfo(cl_platform_id clPlatform,
										cl_platform_info clParamName,
										size_t szParamValueSize,
										void* pParamValue,
										size_t* pszParamValueSizeRet)
{
	LOG_INFO(TEXT("Enter GetPlatformInfo (clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
		clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	// both param_value and param_value_size_ret are null pointers - in this case there is no
	// meaning to do anything
	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == pParamValue || NULL == pszParamValueSizeRet"));
		return CL_INVALID_VALUE;
	}

	if (false == CheckPlatformId(clPlatform))
	{
		LOG_ERROR(TEXT("Current platform id (%d) is not supported"), clPlatform);
		return CL_INVALID_PLATFORM;
	}

	cl_err_code clErr = CL_SUCCESS;
	size_t szParamSize = 0;
	void * pValue = NULL;
	char * pch = NULL,  *pNextToken;
	Device * pDevice = NULL;
	bool bRes = true;
	cl_char pcPlatformExtension[8192] = {0};
	cl_char pcDeviceExtension[8192] = {0};
	cl_char pcOtherDeviceExtension[8192] = {0};
	cl_char pcPlatformICDSuffixKhr[8] = "Intel";

	switch (clParamName)
	{
	case CL_PLATFORM_PROFILE:
		szParamSize = m_uiPlatformInfoStrSize;
		pValue = (void*)m_vPlatformInfoStr;
		break;
	case CL_PLATFORM_VERSION:
		szParamSize = m_uiPlatformVersionStrSize;
		pValue = (void*)m_vPlatformVersionStr;
		break;
	case CL_PLATFORM_NAME:
		szParamSize = m_uiPlatformNameStrSize;
		pValue = (void*)m_vPlatformNameStr;
		break;
	case CL_PLATFORM_VENDOR:
		szParamSize = m_uiPlatformVendorStrSize;
		pValue = (void*)m_vPlatformVendorStr;
		break;
	case CL_PLATFORM_EXTENSIONS:
		assert ((m_uiDevicesCount > 0) && "No devices associated to the platform");
		pDevice = m_ppDevices[0];
		clErr = m_ppDevices[0]->GetInfo(CL_DEVICE_EXTENSIONS, 8192, pcDeviceExtension, NULL);
		if (CL_FAILED(clErr))
		{
			return CL_INVALID_VALUE;
		}
		pch = STRTOK_S((char*)pcDeviceExtension," ", &pNextToken);
		while (pch != NULL)
		{
			bRes = true;
			for (unsigned int ui=1; ui<m_uiDevicesCount; ++ui)
			{
				clErr = m_ppDevices[ui]->GetInfo(CL_DEVICE_EXTENSIONS, 8192, pcOtherDeviceExtension, NULL);
				if (CL_FAILED(clErr))
				{
					return CL_INVALID_VALUE;
				}
				if (NULL == strstr((char*)pcOtherDeviceExtension, pch))
				{
					bRes = false;
					break;
				}
			}
			if (bRes)
			{
				STRCAT_S((char*)pcPlatformExtension, 8192, pch);
				STRCAT_S((char*)pcPlatformExtension, 8192, " ");
			}
			pch = STRTOK_S(NULL, " ", &pNextToken);
		}

		STRCAT_S((char*)pcPlatformExtension, 8192, m_vPlatformExtensionsStr);

		pValue = pcPlatformExtension;
		szParamSize = strlen((char*)pcPlatformExtension) + 1;
		break;
	case CL_PLATFORM_ICD_SUFFIX_KHR:
		pValue = (void*)pcPlatformICDSuffixKhr;
		szParamSize = strlen((char*)pcPlatformICDSuffixKhr) + 1;
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
			LOG_ERROR(TEXT("szParamValueSize (%d) < pszParamValueSizeRet (%d)"), szParamValueSize, szParamSize);
			return CL_INVALID_VALUE;
		}
		memset(pParamValue, 0, szParamValueSize);
		MEMCPY_S(pParamValue, szParamValueSize, pValue, szParamSize);
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetDeviceIDs
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_int	PlatformModule::GetDeviceIDs(cl_platform_id clPlatform,
									 cl_device_type clDeviceType,
									 cl_uint uiNumEntries,
									 cl_device_id* pclDevices,
									 cl_uint* puiNumDevices)
{
	LOG_INFO(TEXT("Enter GetDeviceIDs (device_type=%d, num_entried=%d, pclDevices=%d, puiNumDevices=%d)"),
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
		LOG_ERROR(TEXT("%S"), TEXT("NULL == pclDevices && NULL == puiNumDevices"));
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
		LOG_ERROR(TEXT("%S"), TEXT("can't allocate memory for devices (NULL == ppDevices)"));
		return CL_OUT_OF_HOST_MEMORY;
	}
	clErrRet = m_pDevices->GetObjects(uiNumDevices, (OCLObject<_cl_device_id_int>**)ppDevices, NULL);
	if (CL_FAILED(clErrRet))
	{
		delete[] ppDevices;
		return CL_ERR_OUT(clErrRet);
	}
	pDeviceIds = new cl_device_id[uiNumDevices];
	if (NULL == pDeviceIds)
	{
		LOG_ERROR(TEXT("%S"), TEXT("can't allocate memory for device ids (NULL == pDeviceIds)"));
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
				pDeviceIds[uiRetNumDevices++] = ppDevices[ui]->GetHandle();
				continue;
			}
			if (clDeviceType == CL_DEVICE_TYPE_ALL)
			{
				pDeviceIds[uiRetNumDevices++] = ppDevices[ui]->GetHandle();
			}
			else
			{
				// get device type
				cl_device_type clType;
				cl_int iErrRet = ppDevices[ui]->GetInfo(CL_DEVICE_TYPE, sizeof(cl_device_type), &clType, NULL);
				// check that the current device type satisfactory
				if (iErrRet == 0 && ((clType & clDeviceType) == clType))
				{
					pDeviceIds[uiRetNumDevices++] = ppDevices[ui]->GetHandle();
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
		cl_uint uiNumDevicesToAdd = min(uiRetNumDevices,uiNumEntries);

		for (cl_uint ui=0; ui < uiNumDevicesToAdd; ++ui)
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

	// both param_value and param_value_size_ret are null pointers - in this case there is no
	// meaning to do anything
	if (NULL == pParamValue && NULL == pszParamValueSizeRet)
	{
		LOG_ERROR(TEXT("%S"), TEXT("NULL == pParamValue || NULL == pszParamValueSizeRet"));
		return CL_INVALID_VALUE;
	}

	Device * pDevice = NULL;
	cl_err_code clErrRet = CL_SUCCESS;
	size_t szParamSize = 0;
	void * pValue = NULL;

	switch(clParamName)
	{
	case CL_DEVICE_PLATFORM:
		szParamSize = sizeof(cl_platform_id);
		pValue = &(m_clPlatformIds[0]);
		break;
	default:
		clErrRet = m_pDevices->GetOCLObject((_cl_device_id_int*)clDevice, (OCLObject<_cl_device_id_int>**)(&pDevice));
		if (CL_FAILED(clErrRet))
		{
			return CL_INVALID_DEVICE;
		}
		return pDevice->GetInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
	}

	if (NULL != pszParamValueSizeRet)
	{
		*pszParamValueSizeRet = szParamSize;
	}

	if (NULL != pParamValue)
	{
		if (szParamValueSize < szParamSize)
		{
			LOG_ERROR(TEXT("szParamValueSize (%d) < pszParamValueSizeRet (%d)"), szParamValueSize, szParamSize);
			return CL_INVALID_VALUE;
		}
		MEMCPY_S(pParamValue, szParamValueSize, pValue, szParamSize);
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// PlatformModule::GetDevice
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	PlatformModule::GetDevice(cl_device_id clDeviceId, Device ** ppDevice)
{
	LOG_INFO(TEXT("PlatformModule::GetDevice enter. clDeviceId=%d, ppDevices=%d"),clDeviceId, ppDevice);
	assert( (NULL != ppDevice) && (NULL != m_pDevices) );

	// get the device from the devices list
	return m_pDevices->GetOCLObject((_cl_device_id_int*)clDeviceId, (OCLObject<_cl_device_id_int>**)ppDevice);
}

FECompiler * PlatformModule::GetDefaultFECompiler()
{
	return m_pDefaultFECompiler;
}

//////////////////////////////////////////////////////////////////////////
// PlatformModule::UnloadCompiler
//////////////////////////////////////////////////////////////////////////
cl_int PlatformModule::UnloadCompiler(void)
{
	Device * pDevice = NULL;
	for (cl_uint ui=0; ui<m_pDevices->Count(); ++ui)
	{
		cl_err_code clErr = m_pDevices->GetObjectByIndex(ui, (OCLObject<_cl_device_id_int>**)&pDevice);
		if (CL_SUCCEEDED(clErr) && (NULL != pDevice))
		{
			pDevice->GetDeviceAgent()->clDevUnloadCompiler();
		}
	}
	return CL_SUCCESS;
}

cl_int PlatformModule::GetGLContextInfo(const cl_context_properties * properties, cl_gl_context_info param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
#if defined (_WIN32) //TODO GL support for Linux
	if ( NULL == properties )
	{
		return CL_INVALID_VALUE;
	}

	cl_context_properties hGL, hDC, hDevGL, hDevDC;
	cl_int ret;
	Device * pDevice = NULL;
	cl_device_id	devId = NULL;

	switch(param_name)
	{
	case CL_DEVICES_FOR_GL_CONTEXT_KHR:
	{
		// Return all device in context
		param_value_size /= sizeof(cl_device_id);
		cl_uint uiNumDevices;
		assert(param_value_size <= MAXUINT32);
		ret = GetDeviceIDs(0, CL_DEVICE_TYPE_ALL, (cl_uint)param_value_size, (cl_device_id*)param_value, &uiNumDevices);
		if ( CL_FAILED(ret))
		{
			return ret;
		}

		if ( NULL != param_value_size_ret )
		{
			*param_value_size_ret = ( uiNumDevices * sizeof(cl_device_id) );
		}
		break;
	}

	case CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR:
		// Parse options
		ret = ParseGLContextOptions(properties, &hGL, &hDC);
		if (CL_FAILED(ret))
		{
			return ret;
		}
		// Find appropriate device
		for (cl_uint ui=0; ui<m_pDevices->Count(); ++ui)
		{
			ret = m_pDevices->GetObjectByIndex(ui, (OCLObject<_cl_device_id_int>**)&pDevice);
			if (CL_SUCCEEDED(ret) && (NULL != pDevice))
			{
				pDevice->GetInfo(CL_GL_CONTEXT_KHR, sizeof(cl_context_properties), &hDevGL, NULL);
				pDevice->GetInfo(CL_WGL_HDC_KHR, sizeof(cl_context_properties), &hDevDC, NULL);
				if ( (hDC == hDevDC) && (hGL == hDevGL))
				{
					devId = pDevice->GetHandle();
				}
			}
		}

		// Check parameters
		if ( (NULL == param_value) && (0 == param_value_size) && (NULL != param_value_size_ret) )
		{
			*param_value_size_ret = sizeof(cl_device_id);
			return CL_SUCCESS;
		}

		if ( (NULL == param_value) || (sizeof(cl_device_id) > param_value_size) )
		{
			return CL_INVALID_VALUE;
		}

		*(cl_device_id*)param_value = devId;
		if ( NULL != param_value_size_ret)
		{
			*param_value_size_ret = NULL == devId ? 0 : sizeof(cl_device_id);
		}
		break;

	default:
		return CL_INVALID_VALUE;
	}
	return CL_SUCCESS;
#else
	assert(0 && "GL NOT Implemented on Linux");
#endif
}
