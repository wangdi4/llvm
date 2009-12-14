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

#include "framework_proxy.h"
#include "program.h"
#include "program_binary.h"
#include "context.h"
#include "kernel.h"

#include <string.h>
#include <device.h>
#include <fe_compiler.h>
#include <cl_utils.h>
#include <assert.h>
using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Program::Program(Context * pContext)
{
	::OCLObject();

#ifdef _DEBUG
	assert (pContext != NULL);
#endif

	m_pContext = pContext;
	m_uiStcStrCount = 0;
	m_ppcSrcStrArr = NULL;
	m_pszSrcStrLengths = NULL;
	m_eProgramState = PST_NONE;
	m_clSourceBuildStatus = CL_BUILD_NONE;

	m_pfnNotify = NULL;
	m_pUserData = NULL;
	m_pBuildOptions = NULL;

	m_uiFeBuildLogLength = 0;
	m_pcFeBuildLog = NULL;

	m_pKernels = new OCLObjectsMap();

	m_mapIntermidiates.clear();

	m_ppDevices = pContext->GetDevices(NULL);
	m_pDeviceIds = pContext->GetDeviceIds(&m_uiDevicesCount);

    // Sign to be dependent on the context, ensure the context will be delated only after the object was
    m_pContext->AddPendency();

	m_bBuildFinished = false;

	m_pHandle = new _cl_program;
	m_pHandle->object = this;

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
	delete[] m_pBuildOptions;

	// release binaries resources
	map<cl_device_id, ProgramBinary*>::iterator it = m_mapIntermidiates.begin();
	while (it != m_mapIntermidiates.end())
	{
		ProgramBinary * pProgBin = it->second;
		delete pProgBin;
		it++;
	}
	m_mapIntermidiates.clear();
	m_mapDevices.clear();

	m_pKernels->Clear();
    delete m_pKernels;

    m_pContext->RemovePendency();

	// Release log info
	if ( NULL != m_pcFeBuildLog )
	{
		delete []m_pcFeBuildLog;
	}

	if (NULL != m_pHandle)
	{
		delete m_pHandle;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::Release()
{
	cl_err_code clErrRet = OCLObject::Release();
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	LOG_DEBUG(L"Program::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d", 
		param_name, param_value_size, param_value, param_value_size_ret);
	
	cl_err_code clErrRet = CL_SUCCESS;
	if (NULL == param_value && NULL == param_value_size_ret)
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamValueSize = 0;
	void * pValue = NULL;

	cl_context clContextParam = 0;
	cl_uint uiParam = 0;
	map<cl_device_id, Device*>::iterator it;
	map<cl_device_id, ProgramBinary*>::iterator it_bin;
	char * pSourceCode = NULL;
	
	switch ( (cl_program_info)param_name )
	{
	case CL_PROGRAM_REFERENCE_COUNT:
		szParamValueSize = sizeof(cl_uint);
		pValue = &m_uiRefCount;
		break;

	case CL_PROGRAM_CONTEXT:
		szParamValueSize = sizeof(cl_context);
		clContextParam = (cl_context)m_pContext->GetId();
		pValue = &clContextParam;
		break;

	case CL_PROGRAM_NUM_DEVICES:
		szParamValueSize = sizeof(cl_uint);
		pValue = &m_uiDevicesCount;
		break;

	case CL_PROGRAM_DEVICES:
		szParamValueSize = sizeof(cl_uint) * m_uiDevicesCount;
		pValue = m_pDeviceIds;
		break;
	
	case CL_PROGRAM_SOURCE:
		szParamValueSize = 1;
		for (cl_uint ui=0; ui<m_uiStcStrCount; ++ui)
		{
			szParamValueSize += m_pszSrcStrLengths[ui];
		}
		if (NULL != param_value)
		{
			pSourceCode = new char[szParamValueSize];
			if (NULL == pSourceCode)
			{
				return CL_ERR_INITILIZATION_FAILED;
			}
			memcpy_s(pSourceCode, m_pszSrcStrLengths[0], m_ppcSrcStrArr[0], m_pszSrcStrLengths[0]);
			for (cl_uint ui=1; ui<m_uiStcStrCount; ++ui)
			{
				memcpy_s(pSourceCode + m_pszSrcStrLengths[ui-1], m_pszSrcStrLengths[ui], m_ppcSrcStrArr[ui], m_pszSrcStrLengths[ui]);
			}
			// Note: according to spec section 5.4.5, the length returned should include the null terminator
			pSourceCode[szParamValueSize-1] = NULL;
			pValue = pSourceCode;
		}
		break;
	
	case CL_PROGRAM_BINARY_SIZES:
		
		szParamValueSize = sizeof(size_t) * m_mapIntermidiates.size();
		if (NULL != param_value)
		{
			if (param_value_size < szParamValueSize)
			{
				return CL_INVALID_VALUE;
			}
			it_bin = m_mapIntermidiates.begin();
			for (cl_uint ui=0; ui<m_mapIntermidiates.size() && it_bin != m_mapIntermidiates.end(); ++ui)
			{
				(it_bin->second)->GetBinary(0, NULL, &((size_t*)param_value)[ui]);
				it_bin++;
			}
		}
		if (NULL != param_value_size_ret)
		{
			*param_value_size_ret = szParamValueSize;
		}
		return CL_SUCCESS;

	case CL_PROGRAM_BINARIES:

		szParamValueSize = sizeof(char *) * m_mapIntermidiates.size();
		// get  data
		if (NULL != param_value)
		{
			if (param_value_size < szParamValueSize)
			{
				return CL_INVALID_VALUE;
			}
			it_bin = m_mapIntermidiates.begin();
			for ( cl_uint ui=0; ui<m_mapDevices.size() && it_bin != m_mapIntermidiates.end(); ui++)
			{
				(it_bin->second)->GetBinary(0, NULL, &uiParam);
				(it_bin->second)->GetBinary(uiParam, ((char**)param_value)[ui], &uiParam);
				it_bin++;
			}
		}
		// get  size
		if (NULL != param_value_size_ret)
		{
			*param_value_size_ret = szParamValueSize;
		}
		return CL_SUCCESS;
	default:
		LOG_ERROR(L"param_name (=%d) isn't valid", param_name);
		return CL_INVALID_VALUE;
	}

	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (NULL != param_value && param_value_size < szParamValueSize)
	{
		LOG_ERROR(L"param_value_size (=%d) < szParamValueSize (=%d)", param_value_size, szParamValueSize);
		delete[] pSourceCode;
		return CL_INVALID_VALUE;
	}

	// if param_value == NULL return only param value size
	if (NULL != param_value_size_ret)
	{
		*param_value_size_ret = szParamValueSize;

	}

	if (NULL != param_value && szParamValueSize > 0)
	{
		memcpy_s(param_value, szParamValueSize, pValue, szParamValueSize);
	}
	delete[] pSourceCode;

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::GetBuildInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::GetBuildInfo(	cl_device_id clDevice, 
									cl_program_build_info clParamName, 
									size_t szParamValueSize, 
									void * pParamValue, 
									size_t * pzsParamValueSizeRet)
{
	LOG_DEBUG(L"Enter GetBuildInfo (clDevice=%d, clParamName=%d, szParamValueSize=%d, pParamValue=%d, pzsParamValueSizeRet=%d)", 
		clDevice, clParamName, szParamValueSize, pParamValue, pzsParamValueSizeRet);

	cl_err_code clErr = CL_SUCCESS;
	if ((NULL == pParamValue && NULL == pzsParamValueSizeRet)	||
		(NULL == pParamValue && szParamValueSize > 0))
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamSize = 0;
	void * pValue = NULL;
	ProgramBinary * pProgBin = NULL;
	Device * pDevice = NULL;
	cl_build_status clBuildStatus;

	map<cl_device_id, Device*>::iterator it_device = m_mapDevices.find(clDevice);
	if (it_device == m_mapDevices.end())
	{
		return CL_INVALID_DEVICE;
	}
	pDevice = it_device->second;

	map<cl_device_id, ProgramBinary*>::iterator it_bin = m_mapIntermidiates.find(clDevice);
	pProgBin = (it_bin == m_mapIntermidiates.end()) ? NULL : it_bin->second;

	switch (clParamName)
	{
	case CL_PROGRAM_BUILD_STATUS:
		szParamSize = sizeof(cl_build_status);
		if (NULL == pProgBin)
		{
			clBuildStatus = m_clSourceBuildStatus;
			//// if the initial state of the program was source code, then NULL value means that the
			//// build process was failed
			//if (m_eProgramState == PST_INTERMIDIATE_FROM_SOURCE)
			//{
			//	clBuildStatus = CL_BUILD_ERROR;
			//}
			//else
			//{
			//	clBuildStatus = CL_BUILD_NONE;
			//}
		}
		else
		{
			clBuildStatus = (cl_build_status)pProgBin->GetStatus();
		}
		pValue = &clBuildStatus;
		break;
	
	case CL_PROGRAM_BUILD_OPTIONS:
		if (NULL != m_pBuildOptions)
		{
			szParamSize = strlen(m_pBuildOptions) + 1;
		}
		pValue = m_pBuildOptions;
		break;

	case CL_PROGRAM_BUILD_LOG:
		if (NULL == pProgBin)
		{
			szParamSize = m_uiFeBuildLogLength;
			pValue = m_pcFeBuildLog;
		}
		else
		{
			clErr = pDevice->GetBuildLog(pProgBin->GetId(), szParamValueSize, (char*)pParamValue, pzsParamValueSizeRet);
			return clErr;
		}
		break;
	
	default:
		LOG_ERROR(L"clParamName (=%d) isn't valid", clParamName);
		return CL_INVALID_VALUE;
	}

	if (NULL != pParamValue && szParamSize > szParamValueSize)
	{
		return CL_INVALID_VALUE;
	}

	// if pParamValue == NULL return only param value size
	if (NULL != pzsParamValueSizeRet)
	{
		*pzsParamValueSizeRet = szParamSize;
	}

	if (NULL != pParamValue && szParamSize > 0)
	{
		memcpy_s(pParamValue, szParamValueSize, pValue, szParamSize);
	}

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// AddSource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::AddSource(cl_uint uiCount, const char ** ppcStrings, const size_t * pszLengths)
{
	LOG_DEBUG(L"AddSource enter. uiCount=%d, ppcStrings=%d, pszLengths=%d",uiCount, ppcStrings, pszLengths);
	
	// check if the program wasn't initialized yet
	if (m_eProgramState != PST_NONE)
	{
		LOG_ERROR(L"m_eProgramState != PST_NONE");
		return CL_ERR_INITILIZATION_FAILED;
	}

	// check inputarguments
	if (m_uiStcStrCount > 0 || NULL != m_ppcSrcStrArr)
	{
		// source has allready added to this program
		LOG_ERROR(L"m_uiStcStrCount > 0 || NULL != m_ppcSrcStrArr; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}

	// add context's devices to the devices list and the binaries list;
	for (cl_uint ui=0; ui<m_uiDevicesCount; ++ui)
	{
		m_mapDevices[(cl_device_id)m_ppDevices[ui]->GetId()] = m_ppDevices[ui];
	}

	// check if count is zero or if strings or any entry in strings is NULL
	if (0 == uiCount || NULL == ppcStrings )
	{
		LOG_ERROR(L"0 == uiCount || NULL == ppcStrings; return CL_INVALID_VALUE");
		return CL_INVALID_VALUE;
	}

	for(cl_uint ui=0; ui<uiCount; ++ui)
	{
		if ( NULL == ppcStrings[ui] )
		{
			LOG_ERROR(L"NULL == ppcStrings[%d]; return CL_INVALID_VALUE", ui);
			return CL_INVALID_VALUE;
		}
	}

	// COPY THE SOURCE CODE
	// --------------------

	m_uiStcStrCount = uiCount;
	// allocate host memory for source code
	LOG_DEBUG(L"m_pszSrcStrLengths = new size_t[%d]",m_uiStcStrCount);
	m_pszSrcStrLengths = new size_t[m_uiStcStrCount];
	if (NULL == m_pszSrcStrLengths)
	{
		LOG_ERROR(L"NULL == m_pszSrcStrLengths");
		return CL_OUT_OF_HOST_MEMORY;
	}
	for(cl_uint ui=0; ui<m_uiStcStrCount; ++ui)
	{
		if ( (NULL == pszLengths) || (0==pszLengths[ui]) )
		{
			m_pszSrcStrLengths[ui] = strlen(ppcStrings[ui]);
		}
		else
		{
			m_pszSrcStrLengths[ui] = pszLengths[ui];
		}
	}
	// allocate host memory for source code
	LOG_DEBUG(L"m_ppcSrcStrArr = new char * [%d]",m_uiStcStrCount);
	m_ppcSrcStrArr = new const char * [m_uiStcStrCount];
	if (NULL == m_ppcSrcStrArr)
	{
		LOG_ERROR(L"NULL == m_ppcSrcStrArr");
		delete[] m_pszSrcStrLengths;
		m_pszSrcStrLengths = NULL;
		return CL_OUT_OF_HOST_MEMORY;
	}
	for (cl_uint ui=0; ui<m_uiStcStrCount; ++ui)
	{
		//if ( 0 == m_pszSrcStrLengths[ui] )
		//{
		//	continue; // Skip empty string
		//}
		LOG_DEBUG(L"m_ppcSrcStrArr[ui] = new char[%d] + 1]",m_pszSrcStrLengths[ui]);
		m_ppcSrcStrArr[ui] = new char[m_pszSrcStrLengths[ui] + 1];
		if (NULL == m_ppcSrcStrArr[ui])
		{
			// free all previouse strings
			LOG_ERROR(L"NULL == m_ppcSrcStrArr");
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
		memset((void*)m_ppcSrcStrArr[ui], 0, m_pszSrcStrLengths[ui] + 1);
		if (0 != m_pszSrcStrLengths[ui])
		{
			memcpy_s((void*)m_ppcSrcStrArr[ui], m_pszSrcStrLengths[ui], ppcStrings[ui], m_pszSrcStrLengths[ui]);
		}
		//strcpy_s((char*)(m_ppcSrcStrArr[ui]), m_pszSrcStrLengths[ui] + 1, ppcStrings[ui]);
	}

	m_eProgramState = PST_SOURCE_CODE;

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// AddBinary
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::AddBinaries(cl_uint uiNumDevices, 
								 Device ** ppDevices, 
								 const size_t * pszBinariesSize, 
								 const unsigned char ** ppBinaries,
								 cl_int * piBinaryStatus)
{
	LOG_DEBUG(L"AddBinary enter. uiNumDevices=%d, ppDevices=%d, pszBinariesSize=%d, ppBinaries=%d, piBinaryStatus=%d", 
		uiNumDevices, ppDevices, pszBinariesSize, ppBinaries, piBinaryStatus);

	cl_err_code clErrRet = CL_SUCCESS;
	cl_err_code clFinalErrRet = CL_SUCCESS;
	
	// check if the program wasn't initialized yet
	if (m_eProgramState != PST_NONE)
	{
		LOG_ERROR(L"m_eProgramState != PST_NONE");
		return CL_ERR_INITILIZATION_FAILED;
	}

	// check input parameters
	if (0 == uiNumDevices || NULL == ppDevices || NULL == pszBinariesSize || NULL == ppBinaries)
	{
		LOG_ERROR(L"0 == uiNumDevices || NULL == ppDevices || NULL == pszBinariesSize || NULL == ppBinariesData");
		return CL_INVALID_VALUE;
	}

	// check binaries and add them to the program object
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		// get device and the device is valid device
		Device * pDevice = ppDevices[ui];
		if (NULL == pDevice)
		{
			LOG_ERROR(L"device number %d is not valid", ui);
			return CL_INVALID_DEVICE;
		}

		// check if the current device allready have binary assign for it
		map<cl_device_id,ProgramBinary*>::iterator it = m_mapIntermidiates.find((cl_device_id)pDevice->GetId());
		if (it != m_mapIntermidiates.end())
		{
			return CL_ERR_KEY_ALLREADY_EXISTS;
		}

		// set the device in the program's devices list
		m_mapDevices[(cl_device_id)pDevice->GetId()] = pDevice;

		// check binary
		clErrRet = pDevice->CheckProgramBinary(pszBinariesSize[ui], ppBinaries[ui]);

		// update binary status
		if (NULL != piBinaryStatus)
		{
			piBinaryStatus[ui] = CL_SUCCEEDED(clErrRet) ? CL_SUCCESS : CL_INVALID_BINARY;
		}

		// if binary is valid binary create program binary object and add it to the program object
		if (CL_SUCCEEDED(clErrRet))
		{
			ProgramBinary * pProgBin = new ProgramBinary(	pszBinariesSize[ui], 
															ppBinaries[ui], 
															CL_DEV_BINARY_USER, 
															pDevice, 
															GET_LOGGER_CLIENT,
															&clErrRet);
			if (CL_FAILED(clErrRet))
			{
				return clErrRet;
			}
			m_mapIntermidiates[(cl_device_id)pDevice->GetId()] = pProgBin;
		}
		else
		{
			LOG_ERROR(L"binary ppBinaries[%d] isn't valid for device %d", ui, pDevice->GetId());
			m_mapIntermidiates[(cl_device_id)pDevice->GetId()] = NULL;
			clFinalErrRet = CL_INVALID_BINARY;
		}
	}

	m_eProgramState = PST_INTERMIDIATE;

	return clFinalErrRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CheckBinaries
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::CheckBinaries(cl_uint uiNumDevices, const cl_device_id * pclDevices)
{
	LOG_DEBUG(L"CheckBinaries enter. uiNumDevices=%d, pclDevices=%d", uiNumDevices, pclDevices);
	// check input parameters
	if (0 == uiNumDevices || NULL == pclDevices)
	{
		LOG_ERROR(L"0 == uiNumDevices || NULL == pclDevices");
		return CL_INVALID_DEVICE;
	}
	// for each device in uiNumDevices: check that the binaries exists and ready (not being built by the backend)
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		map<cl_device_id,ProgramBinary*>::iterator it = m_mapIntermidiates.find(pclDevices[ui]);
		if (it == m_mapIntermidiates.end())
		{
			LOG_ERROR(L"device id %d not found in m_mapIntermidiates", pclDevices[ui]);
			return CL_INVALID_DEVICE;
		}
		ProgramBinary * pProgBin = it->second;
		if (NULL == pProgBin)
		{
			LOG_ERROR(L"NULL == pProgBin");
			return CL_INVALID_BINARY;
		}
		Device * pDevice = (Device*)pProgBin->GetDevice();
		if (NULL == pDevice)
		{
			LOG_ERROR(L"device id %d not attached to program binary", pclDevices[ui]);
			return CL_INVALID_DEVICE;
		}
		// check binary status
		cl_build_status clBuildStatus = pProgBin->GetStatus();
		if (CL_BUILD_IN_PROGRESS == clBuildStatus)
		{
			LOG_ERROR(L"program binarie's status is CL_BUILD_IN_PROGRESS");
			return CL_INVALID_OPERATION;
		}
		cl_uint uiBinsize = 0;
		char * pBinData = NULL;
		cl_err_code clErrRet = pProgBin->GetBinary(0, NULL, &uiBinsize);
		if (CL_SUCCEEDED(clErrRet))
		{
			pBinData = new char[uiBinsize];
			if (NULL == pBinData)
			{
				return CL_OUT_OF_HOST_MEMORY;
			}
			pProgBin->GetBinary(uiBinsize, pBinData, NULL);
		}
		clErrRet = pDevice->CheckProgramBinary(uiBinsize, pBinData);
		delete[] pBinData;
		if (CL_FAILED(clErrRet))
		{
			LOG_ERROR(L"binary of device %d isn't valid binary", pclDevices[ui]);
			return CL_INVALID_BINARY;
		}
	}
	return CL_SUCCESS;
}
cl_err_code Program::BuildBinaries(cl_uint uiNumDevices, const cl_device_id * pclDevices, const char * pcOptions)
{
	//cl_start;
	LOG_DEBUG(L"BuildBinarys enter. uiNumDevices=%d, pclDevices=%d", uiNumDevices, pclDevices);
	// check input parameters

	// Lock intermidiates map, on the first phase start all build process
	// prevent callback execution
	OclAutoMutex CS(&m_csIntermidateMap);
	// for each device in uiNumDevices: build binary
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		map<cl_device_id,ProgramBinary*>::iterator it = m_mapIntermidiates.find(pclDevices[ui]);
		if (it == m_mapIntermidiates.end())
		{
			LOG_ERROR(L"device id %d not found in m_mapIntermidiates", pclDevices[ui]);
			return CL_INVALID_DEVICE;
		}
		ProgramBinary * pProgBin = it->second;
		if (NULL == pProgBin)
		{
			LOG_ERROR(L"NULL == pProgBin");
			return CL_INVALID_BINARY;
		}
		IBuildDoneObserver * pBuildDoneObserver = dynamic_cast<IBuildDoneObserver*>(this);
		pProgBin->Build(pcOptions, pBuildDoneObserver);
	}
	//cl_return CL_SUCCESS;
	return CL_SUCCESS;
}

cl_err_code Program::BuildSource(cl_uint uiNumDevices, const cl_device_id * pclDevices, const char * pcOptions)
{
	//cl_start;
	LOG_DEBUG(L"Enter BuildSource(uiNumDevices=%d, pclDevices=%d, pcOptions=%d)", uiNumDevices, pclDevices, pcOptions);

	m_clSourceBuildStatus = CL_BUILD_IN_PROGRESS;

	// check that the compilers available for all devices
	FECompiler *pFeCompiler = NULL;
	cl_err_code clErr = CheckFECompilers(uiNumDevices, pclDevices);
	if (CL_FAILED(clErr))
	{
		LOG_ERROR(L"Front-End compiler isn't available for all devices");
		m_clSourceBuildStatus = CL_BUILD_ERROR;
		return clErr;
	}

	// Lock intermidiates map, on the first phase start all build process
	// prevent callback execution
	OclAutoMutex CS(&m_csIntermidateMap);

	// for each device in uiNumDevices: check if program binary exists
	// if yes, skip build
	// if not build source and create new program binary
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		LOG_DEBUG(L"Build source for device %d", pclDevices[ui]);
		map<cl_device_id,ProgramBinary*>::iterator it = m_mapIntermidiates.find(pclDevices[ui]);
		if (it != m_mapIntermidiates.end())
		{
			continue;
		}
		map<cl_device_id,Device*>::iterator it_device = m_mapDevices.find(pclDevices[ui]);
		if (it_device == m_mapDevices.end())
		{
			LOG_ERROR(L"device id %d not found in m_mapDevices", pclDevices[ui]);
			m_clSourceBuildStatus = CL_BUILD_ERROR;
			return CL_INVALID_DEVICE;
		}
		Device * pDevice = it_device->second;
		FECompiler * pFeCompiler = (FECompiler*) pDevice->GetFECompiler();

		// if front-end compiler isn't availble try defualt one
		if (NULL == pFeCompiler)
		{
			pFeCompiler = FrameworkProxy::Instance()->GetPlatformModule()->GetDefaultFECompiler();
			if ( NULL == pFeCompiler )
			{
				LOG_DEBUG(L"Front-End compiler is not available");
				m_mapIntermidiates[pclDevices[ui]] = NULL;
				continue;
			}
		}
		LOG_DEBUG(L"Get FE compiler (compiler = %d)", pFeCompiler);

		IFrontendBuildDoneObserver * pFrontendBuildDoneObserver = dynamic_cast<IFrontendBuildDoneObserver*>(this);
		cl_err_code clErr = pFeCompiler->BuildProgram(pclDevices[ui], m_uiStcStrCount, m_ppcSrcStrArr, m_pszSrcStrLengths, pcOptions,
														pFrontendBuildDoneObserver);
		if (CL_FAILED(clErr))
		{
			LOG_ERROR(L"pFeCompiler->BuildProgram(...) = %ws", ClErrTxt(clErr));
			m_mapIntermidiates[pclDevices[ui]] = NULL;
			continue;
		}
	}
	//cl_return CL_SUCCESS;
	return CL_SUCCESS;
}

cl_err_code Program::Build(cl_uint uiNumDevices,
						   const cl_device_id *	pclDeviceList,
						   const char *	pcOptions,
						   void (*pfnNotify)(cl_program clProgram, void * pUserData),
						   void * pUserData)
{
	//cl_start;
	LOG_DEBUG(L"Build enter. uiNumDevices=%d, pclDeviceList=%d, pcOptions=%d, pUserData=%d", 
		uiNumDevices, pclDeviceList, pcOptions, pUserData);

	cl_err_code clErrRet = CL_SUCCESS;

    // check invalid input
	if (uiNumDevices > 0 && NULL == pclDeviceList	||
		uiNumDevices == 0 && NULL != pclDeviceList )
	{
		LOG_ERROR(L"uiNumDevices > 0 && NULL == pclDeviceList || uiNumDevices == 0 && NULL != pclDeviceList");
		return CL_INVALID_VALUE;
	}
	
	cl_uint uiAllDevices = 0;
	cl_device_id * pclAllDeviceList = NULL;
	
	// check input parameters
	// when devices is NULL, apply on all devices
	if (0 == uiNumDevices && NULL == pclDeviceList)
	{
		pclAllDeviceList = m_pContext->GetDeviceIds(&uiAllDevices);
		if ( 0 == uiAllDevices )
		{
			return CL_INVALID_PROGRAM;
		}
	}

	m_pUserData = pUserData;
	m_pfnNotify = pfnNotify;

	// update build options: delete previouse options and allocate memory for new options
	if (NULL != m_pBuildOptions)
	{
		delete[] m_pBuildOptions;
	}

	if (NULL != pcOptions)
	{
		m_pBuildOptions = new char[strlen(pcOptions) + 1];
		if (NULL == m_pBuildOptions)
		{
			if (NULL != pclAllDeviceList)
			{
				delete []pclAllDeviceList;
			}

			return CL_OUT_OF_HOST_MEMORY;
		}
		strcpy_s(m_pBuildOptions, strlen(pcOptions) + 1, pcOptions);
	}

	// check devices and binaries
	if (m_eProgramState == PST_SOURCE_CODE)
	{
		// if the program created with source, build source first and then build binaries
		clErrRet = (pclAllDeviceList == NULL) ?
			BuildSource(uiNumDevices, pclDeviceList, m_pBuildOptions) :
			BuildSource(uiAllDevices, pclAllDeviceList, m_pBuildOptions);

	}
    else
    {
		if (NULL == pclAllDeviceList)
		{
			// Build binaries.
			clErrRet = CheckBinaries(uiNumDevices, pclDeviceList);
			if (CL_SUCCEEDED(clErrRet))
			{
				clErrRet = BuildBinaries(uiNumDevices, pclDeviceList, m_pBuildOptions);
			}
			else
			{
				// Log error
				LOG_ERROR(L"CheckBinaries(%d, %d)=%d", uiNumDevices, pclDeviceList, clErrRet);
			}
		}
		else
		{
			// Build binaries.
			clErrRet = CheckBinaries(uiAllDevices, pclAllDeviceList);
			if (CL_SUCCEEDED(clErrRet))
			{
				clErrRet = BuildBinaries(uiAllDevices, pclAllDeviceList, m_pBuildOptions);
			}
			else
			{
				// Log error
				LOG_ERROR(L"CheckBinaries(%d, %d)=%d", uiAllDevices, pclAllDeviceList, clErrRet);
			}
		}
    }
	
	if (CL_SUCCEEDED(clErrRet) && (NULL == m_pfnNotify))
	{
        // Build started, wait for build done event in case that there isn't a notification functino
		{
			OclAutoMutex CS(&m_BuildNotifyCS);
			while (!m_bBuildFinished) { m_BuildWaitCond.Wait(&m_BuildNotifyCS); }
			m_bBuildFinished = false;
		}
		
		// if build falied - need to return error value
		if ((m_eProgramState != PST_BINARY) && (m_eProgramState != PST_BINARY_FROM_SOURCE))
		{
			clErrRet = CL_BUILD_PROGRAM_FAILURE;
		}
		// scan binaries and check status
		else
		{
			map<cl_device_id, ProgramBinary*>::iterator it = m_mapIntermidiates.begin();
			while (it != m_mapIntermidiates.end())
			{
				ProgramBinary * pProgBin = it->second;
				if (NULL == pProgBin || pProgBin->GetStatus() == CL_BUILD_ERROR)
				{
					clErrRet = CL_BUILD_PROGRAM_FAILURE;
				}
				it++;
			}
		}
	}

	// TODO: check if kernels attached to the program
	//cl_return clErrRet;
	return clErrRet;
}

cl_err_code Program::NotifyBuildDone(cl_device_id device, cl_build_status build_status)
{
	//cl_start;
	bool bFinished = true;
	{ // Lock
		OclAutoMutex CS(&m_csIntermidateMap);
		map<cl_device_id, ProgramBinary*>::iterator it = m_mapIntermidiates.begin();
		while (it != m_mapIntermidiates.end())
		{
			if ( NULL == it->second )
				continue;

			cl_build_status status = (it->second)->GetStatus();
			if (status == CL_BUILD_NONE || status == CL_BUILD_IN_PROGRESS)
			{
				bFinished = false;
			}
			it++;
		}
	} // Unlock

	if ( bFinished )
	{
		map<cl_device_id, ProgramBinary*>::iterator it = m_mapIntermidiates.find(device);
		m_eProgramState = ((it->second)->GetBinaryProp() == CL_DEV_BINARY_COMPILER) ? PST_BINARY_FROM_SOURCE : PST_BINARY;

		if ( NULL != m_pfnNotify )
		{
            m_pfnNotify((cl_program)m_iId, m_pUserData);
        }
        else
        {
            // Free blocking
            m_BuildNotifyCS.Lock();
			m_bBuildFinished = true;
			m_BuildNotifyCS.Unlock();
			m_BuildWaitCond.Signal();
        }
    }	

	//cl_return CL_SUCCESS;
	return CL_SUCCESS;
}

cl_err_code Program::NotifyFEBuildDone(cl_device_id device, size_t szBinSize, void * pBinData, const char* pLogStr)
{
	//cl_start;
	LOG_DEBUG(L"Enter NotifyFEBuildDone (device=%d, szBinSize=%d, pBinData=%d, pLogStr=%d", 
		device, szBinSize, pBinData, pLogStr);

	if (NULL != pLogStr)
	{
		m_uiFeBuildLogLength = strlen(pLogStr) + 1;
		m_pcFeBuildLog = new char[m_uiFeBuildLogLength];
		if (NULL == m_pcFeBuildLog)
		{
			m_uiFeBuildLogLength = 0;
		}
		else
		{
			memcpy_s(m_pcFeBuildLog, m_uiFeBuildLogLength, pLogStr, m_uiFeBuildLogLength);
		}
	}

	cl_err_code clErr = CL_SUCCESS;
	map<cl_device_id, Device*>::iterator it = m_mapDevices.find(device);
	Device * pDevice = it->second;
	unsigned int uiDevCount = m_mapDevices.size();

	ProgramBinary *pProgBin = NULL;
	
	// front-end compilation succeeded, create new program binary object
	if (szBinSize != 0 && pBinData != NULL)
	{
		// create new program binary and assign it to the program
		m_clSourceBuildStatus = CL_BUILD_SUCCESS;
		pProgBin = new ProgramBinary(szBinSize, 
									 pBinData, 
									 CL_DEV_BINARY_COMPILER, 
									 pDevice, 
									 GET_LOGGER_CLIENT,
									 &clErr);
		if (CL_FAILED(clErr))
		{
			LOG_ERROR(L"Failed to create new program binary object (return %ws)", ClErrTxt(clErr));
			delete pProgBin;
			pProgBin = NULL;
			m_clSourceBuildStatus = CL_BUILD_ERROR;
		}
	}
	else
	{
		m_clSourceBuildStatus = CL_BUILD_ERROR;
	}

	// check if front-end built source code for all devices
	bool bFinished = false;
	{ // Lock
		OclAutoMutex CS(&m_csIntermidateMap);
		m_mapIntermidiates[device] = pProgBin;
		bFinished = m_mapIntermidiates.size() == uiDevCount;
	} // Unlock

	// next step - build binaries
	if ( bFinished )
	{
		// update program's state - now it contains intermediates
		m_eProgramState = PST_INTERMIDIATE_FROM_SOURCE;

		// build binaries only if front-end succeeded
		if (m_clSourceBuildStatus != CL_BUILD_ERROR)
		{
			cl_uint uiNumDevices = 0;
			cl_device_id * pDevices = m_pContext->GetDeviceIds(&uiNumDevices);
			if (NULL == pDevices)
			{
				m_clSourceBuildStatus = CL_BUILD_ERROR;
				return CL_OUT_OF_HOST_MEMORY;
			}
			clErr = BuildBinaries(uiNumDevices, pDevices, m_pBuildOptions);
			if (CL_FAILED(clErr))
			{
				m_clSourceBuildStatus = CL_BUILD_ERROR;
				if ( NULL != m_pfnNotify )
				{
					m_pfnNotify((cl_program)m_iId, m_pUserData);
				}
				else
				{
					m_BuildNotifyCS.Lock();
					m_bBuildFinished = true;
					m_BuildNotifyCS.Unlock();
					m_BuildWaitCond.Signal();
				}
			}
		}
		else
		{
			if ( NULL != m_pfnNotify )
			{
				m_pfnNotify((cl_program)m_iId, m_pUserData);
			}
			else
			{
				m_BuildNotifyCS.Lock();
				m_bBuildFinished = true;
				m_BuildNotifyCS.Unlock();
				m_BuildWaitCond.Signal();
			}
		}
	}
	//cl_return clErr;
	return clErr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CreateKernel
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::CreateKernel(const char * psKernelName, Kernel ** ppKernel)
{
	//cl_start;

	LOG_DEBUG(L"CreateKernel enter. pscKernelName=%s, ppKernel=%d", psKernelName, ppKernel);
	
	// check if program binary already built
	if ( (m_eProgramState != PST_BINARY) && (m_eProgramState != PST_BINARY_FROM_SOURCE))
	{
		return CL_INVALID_PROGRAM_EXECUTABLE;
	}

	// check invalid input
	if (NULL == psKernelName || NULL == ppKernel)
	{
		return CL_INVALID_VALUE;
	}

	// check if the current kernel allready available in the program
	//bool bResult = IsKernelExists(psKernelName, ppKernel);
	//if (true == bResult)
	//{
	//	return CL_SUCCESS;
	//}
	
	// get program binaries from the program - this will be used to create the device kernels
	// in the kernel object
	ProgramBinary ** ppBinaries = new ProgramBinary * [m_mapIntermidiates.size()];
	if (NULL == ppBinaries)
	{
		LOG_ERROR(L"new ProgramBinary * [%d] == NULL", m_mapIntermidiates.size());
		return CL_OUT_OF_HOST_MEMORY;
	}
	map<cl_device_id, ProgramBinary*>::iterator it = m_mapIntermidiates.begin();
	for (cl_uint ui=0; ui<m_mapIntermidiates.size() && it != m_mapIntermidiates.end(); ++ui)
	{
		ppBinaries[ui] = it->second;
		it++;
	}
	
	// create new kernel object - this is an empty kernel as long as there are no associated device
	// kernel to it.
	Kernel * pKernel = new Kernel(this, psKernelName);
	pKernel->SetLoggerClient(GET_LOGGER_CLIENT);

	// next step - for each device that has the kernel, create device kernel 
	cl_err_code clErrRet = pKernel->CreateDeviceKernels(m_mapIntermidiates.size(), ppBinaries);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(L"pKernel->CreateDeviceKernels(%d, ppBinaries) = %ws", m_mapIntermidiates.size(), ClErrTxt(clErrRet));
		pKernel->Release();
		delete pKernel;
		delete[] ppBinaries;
		return clErrRet;
	}

	// add the kernel object and assing new key for it
	m_pKernels->AddObject((OCLObject*)pKernel);
	*ppKernel = pKernel;
	delete[] ppBinaries;

	AddPendency();

	//cl_return CL_SUCCESS;
	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CreateAllKernels
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::CreateAllKernels(cl_uint uiNumKernels, cl_kernel * pclKernels, cl_uint * puiNumKernelsRet)
{
	LOG_DEBUG(L"Enter CreateAllKernels (uiNumKernels=%d, pclKernels=%d, puiNumKernelsRet=%d", 
		uiNumKernels, pclKernels, puiNumKernelsRet);

	// check if program binary already built
	if ( (m_eProgramState != PST_BINARY) && (m_eProgramState != PST_BINARY_FROM_SOURCE) )
	{
		return CL_INVALID_PROGRAM_EXECUTABLE;
	}

	// check input arguments
	//if (NULL == pclKernels && NULL == puiNumKernelsRet)
	//{
	//	LOG_ERROR(L"NULL == pclKernels && NULL == puiNumKernelsRet")
	//	return CL_INVALID_VALUE;
	//}

	cl_uint uiNewKernels = 0; // count the number of new kernels that added to the program
	cl_err_code clErrRet = CL_SUCCESS;

	// scan program binaries and for each binary get the available kernels
	// for each kernel - create new Kernel object and assign it to the program object
	map<cl_device_id, ProgramBinary*>::iterator it = m_mapIntermidiates.begin();
	while (it != m_mapIntermidiates.end())
	{
		ProgramBinary * pProgBin = it->second;
		if (NULL == pProgBin)
		{
			// not valid binary, skip and move to the next one
			LOG_DEBUG(L"program binary number %d is NULL object", it->first);
			continue;
		}
		Device * pDevice = (Device*)pProgBin->GetDevice();
		if (NULL == pDevice)
		{
			// the current binary hasn't valid device, skip and move to the next one
			LOG_DEBUG(L"device of program binary number %d is NULL object", it->first);
			continue;
		}

		// for each binary, get the attached kernels from the device
		cl_uint uiDevNumKernels = 0;
		clErrRet = pDevice->GetProgramKernels(pProgBin->GetId(), 0, NULL, &uiDevNumKernels);
		if (CL_FAILED(clErrRet))
		{
			LOG_ERROR(L"GetProgramKernels(%d, 0, NULL, %d) = %ws", pProgBin->GetId(), &uiDevNumKernels, ClErrTxt(clErrRet));
			return clErrRet;
		}

		cl_dev_kernel * pKernels = new cl_dev_kernel[uiDevNumKernels];
		if (NULL == pKernels)
		{
			LOG_ERROR(L"new cl_dev_kernel[%d] = NULL", uiDevNumKernels);
			return CL_OUT_OF_HOST_MEMORY;
		}
		clErrRet = pDevice->GetProgramKernels(pProgBin->GetId(), uiDevNumKernels, pKernels, NULL);
		if (CL_FAILED(clErrRet))
		{
			LOG_ERROR(L"GetProgramKernels(%d, %d, %d, NULL) = %ws", pProgBin->GetId(), uiDevNumKernels, pKernels, ClErrTxt(clErrRet));
			delete[] pKernels;
			return clErrRet;
		}

		// for each kernel, get its name and create the kernel object
		Kernel * pKernel = NULL;
		for (cl_uint ui=0; ui<uiDevNumKernels; ++ui)
		{
			// get kernel's name
			size_t szKernelNameSize = 0;
			clErrRet = pDevice->GetKernelInfo(pKernels[ui], CL_DEV_KERNEL_NAME, 0 , NULL, &szKernelNameSize);
			if (CL_FAILED(clErrRet))
			{
				LOG_DEBUG(L"GetKernelInfo of kerenl %d failed (returned %ws)", pKernels[ui], ClErrTxt(clErrRet));
				continue;
			}
			char * psKernelName = new char[szKernelNameSize];
			if (NULL == psKernelName)
			{
				return CL_OUT_OF_HOST_MEMORY;
			}
			clErrRet = pDevice->GetKernelInfo(pKernels[ui], CL_DEV_KERNEL_NAME, szKernelNameSize , psKernelName, NULL);
			if (CL_FAILED(clErrRet))
			{
				LOG_DEBUG(L"GetKernelInfo of kerenl %d failed (returned %ws)", pKernels[ui], ClErrTxt(clErrRet));
				delete[] psKernelName;
				continue;
			}
			
			// once we got the name of the kernel - we need to check if the kernel already exists in the program
			bool bKernelExists = IsKernelExists(psKernelName, &pKernel);
			if (false == bKernelExists)
			{
				pKernel = new Kernel(this, psKernelName);
				pKernel->SetLoggerClient(GET_LOGGER_CLIENT);
			}
			clErrRet = pKernel->AddDeviceKernel(pKernels[ui], pProgBin);
			if (CL_FAILED(clErrRet))
			{
				// if it is a new kernel - delete it
				if (false == bKernelExists)
				{
					delete pKernel;
				}
				delete[] psKernelName;
				continue;
			}
			delete[] psKernelName;
			if (false == bKernelExists)
			{
				m_pKernels->AddObject(pKernel);
				uiNewKernels++;
			}
		}
		it++;
	}

	if (NULL != puiNumKernelsRet)
	{
		*puiNumKernelsRet = m_pKernels->Count();
	}
	
	if ( NULL != pclKernels )
	{
		// Check if passed buffer is big enough
		if (uiNumKernels >= m_pKernels->Count())
		{
			// get the kernels Ids
			clErrRet = m_pKernels->GetIDs(m_pKernels->Count(), (cl_int*)pclKernels, puiNumKernelsRet);
		}
		else
		{
			clErrRet = CL_INVALID_VALUE;
		}

		if (CL_FAILED(clErrRet))
		{
			// remove all kernels from the kernels list
			Kernel * pKernel = NULL;
			for (cl_uint ui=0; ui<m_pKernels->Count(); ++ui)
			{
				cl_int clErr = m_pKernels->GetObjectByIndex(ui, (OCLObject**)&pKernel);
				if (CL_SUCCEEDED(clErr) && NULL != pKernel)
				{
					m_pKernels->RemoveObject((cl_int)pKernel->GetId(), NULL);
					pKernel->Release();
					delete pKernel;
				}
			}
			m_pKernels->Clear();
			for(cl_uint ui=0; ui<uiNewKernels; ++ui)
			{
				RemovePendency();
			}
			return clErrRet;
		}			
	}
	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// IsKernelExists
///////////////////////////////////////////////////////////////////////////////////////////////////
bool Program::IsKernelExists(const char * psKernelName, Kernel ** ppKernelRet)
{
	if (NULL == psKernelName)
	{
		return false;
	}
	
	cl_err_code clErr = CL_SUCCESS;
	Kernel * pKernel = NULL;
	
	// move through the list of kernels and check if there is a kernel match to the name
	for (cl_uint ui=0; ui<m_pKernels->Count(); ++ui)
	{
		clErr = m_pKernels->GetObjectByIndex(ui, (OCLObject**)&pKernel);
		if (CL_SUCCEEDED(clErr))
		{
			if (0 == strcmp(psKernelName, pKernel->GetName()))
			{
				if (NULL != ppKernelRet)
				{
					*ppKernelRet = pKernel;
				}
				return true;
			}
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// IsKernelExists
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::GetKernels(cl_uint uiNumKernels, Kernel ** ppKernels, cl_uint * puiNumKernelsRet)
{
	LOG_DEBUG(L"Enter GetKernels (uiNumKernels=%d, ppKernels=%d, puiNumKernelsRet=%d", 
		uiNumKernels, ppKernels, puiNumKernelsRet);

#ifdef _DEBUG
	assert ( m_pKernels != NULL );
#endif
	
	// check invalid input
	if (NULL == ppKernels && NULL == puiNumKernelsRet)
	{
		return CL_INVALID_VALUE;
	}
	if (NULL != ppKernels && uiNumKernels < m_pKernels->Count())
	{
		return CL_INVALID_VALUE;
	}
	// return number of kernels
	if (NULL != puiNumKernelsRet)
	{
		*puiNumKernelsRet = m_pKernels->Count();
	}

	if (NULL != ppKernels)
	{
		cl_err_code clErr = CL_SUCCESS;
		Kernel * pKernel = NULL;
		for (cl_uint ui=0; ui<m_pKernels->Count(); ++ui)
		{
			clErr = m_pKernels->GetObjectByIndex(ui, (OCLObject**)&pKernel);
			if (CL_FAILED(clErr))
			{
				return clErr;
			}
			ppKernels[ui] = pKernel;
		}
	}

	return CL_SUCCESS;

}
///////////////////////////////////////////////////////////////////////////////////////////////////
// RemoveKernel
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::RemoveKernel(cl_kernel clKernel)
{
	LOG_DEBUG(L"Enter RemoveKernel (clKernel=%d", clKernel);

#ifdef _DEBUG
	assert ( NULL != m_pKernels );
#endif

	bool bResult = m_pKernels->IsExists((cl_int)clKernel);
	if (false == bResult)
	{
		return CL_INVALID_KERNEL;
	}
	cl_err_code clErr = m_pKernels->RemoveObject((cl_int)clKernel, NULL);
	if (CL_FAILED(clErr))
	{
		return clErr;
	}

	RemovePendency();

	return CL_SUCCESS;
}
cl_err_code Program::CheckFECompilers(cl_uint uiNumDevices, const cl_device_id * pclDevices)
{
	LOG_DEBUG(L"Enter CheckFECompilers (uiNumDevices=%d, pclDevices=%d)", 
		uiNumDevices, pclDevices);

	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		map<cl_device_id, Device*>::iterator it = m_mapDevices.find(pclDevices[ui]);

		//assert( "device available in program" && (it != m_mapDevices.end()) && (NULL != it->second) );

		if ( (it == m_mapDevices.end()) || (NULL == it->second) )
		{
			return CL_INVALID_DEVICE;
		}

		Device * pDevice = it->second;
		FECompiler * pFeCompiler = (FECompiler*)pDevice->GetFECompiler();
		if (NULL == pFeCompiler)
		{
			pFeCompiler = FrameworkProxy::Instance()->GetPlatformModule()->GetDefaultFECompiler();
			if (NULL == pFeCompiler)
			{
				//return CL_COMPILER_NOT_AVAILABLE;
				return CL_INVALID_DEVICE;
			}
		}
	}
	return CL_SUCCESS;
}