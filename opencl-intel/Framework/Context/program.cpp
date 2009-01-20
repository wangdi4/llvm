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
#include "context.h"
#include "kernel.h"
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
	m_eProgramSourceType = PST_NONE;

	m_pfnNotify = NULL;
	m_pUserData = NULL;

	m_pKernels = new OCLObjectsMap();

	m_mapBinaries.clear();
	m_mapDevices.clear();
	m_mapBinaryStatus.clear();
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
	m_mapBinaryStatus.clear();
	m_mapDevices.clear();

	delete m_pLoggerClient;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::Release()
{
	return OCLObject::Release();

	Kernel * pKernel = NULL;
	cl_err_code clErrRet = CL_SUCCESS;
	
	if (m_uiRefCount == 0)
	{
		for (cl_uint ui=0; ui<m_pKernels->Count(); ++ui)
		{
			clErrRet = m_pKernels->GetObjectByIndex(ui, (OCLObject**)&pKernel);
			if (CL_FAILED(clErrRet) || NULL == pKernel)
			{
				return clErrRet;
			}
			clErrRet = pKernel->Release();
			if (CL_FAILED(clErrRet))
			{
				return clErrRet;
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	InfoLog(m_pLoggerClient, L"Program::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d", 
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
	cl_device_id * pDevices = NULL;
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
		uiParam = m_mapDevices.size();
		pValue = &uiParam;
		break;

	case CL_PROGRAM_DEVICES:
		szParamValueSize = sizeof(cl_uint) * m_mapDevices.size();
		if (NULL != param_value)
		{
			pDevices = new cl_device_id[m_mapDevices.size()];
			if (NULL == pDevices)
			{
				return CL_ERR_INITILIZATION_FAILED;
			}
			// scan devices and fill array
			it = m_mapDevices.begin();
			for ( cl_uint ui=0; ui<m_mapDevices.size() && it != m_mapDevices.end(); ui++)
			{
				pDevices[ui] = it->first;
				it++;
			}
		}
		pValue = pDevices;
		break;
	
	case CL_PROGRAM_SOURCE:
		szParamValueSize = 0;
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
			pValue = pSourceCode;
		}
		break;
	
	case CL_PROGRAM_BINARY_SIZES:
		
		szParamValueSize = sizeof(size_t) * m_mapBinaries.size();
		if (param_value_size < szParamValueSize)
		{
			return CL_INVALID_VALUE;
		}
		if (NULL != param_value_size_ret)
		{
			*param_value_size_ret = szParamValueSize;
		}
		if (NULL != param_value)
		{
			it_bin = m_mapBinaries.begin();
			for (cl_uint ui=0; ui<m_mapBinaries.size() && it_bin != m_mapBinaries.end(); ++ui)
			{
				(it_bin->second)->GetBinary(0, NULL, &((size_t*)param_value)[ui]);
				it_bin++;
			}
		}
		return CL_SUCCESS;

	case CL_PROGRAM_BINARIES:

		szParamValueSize = sizeof(char *) * m_mapBinaries.size();
		if (param_value_size < szParamValueSize)
		{
			return CL_INVALID_VALUE;
		}
		// get  size
		if (NULL != param_value_size_ret)
		{
			*param_value_size_ret = szParamValueSize;
		}
		// get  data
		if (NULL != param_value)
		{
			it_bin = m_mapBinaries.begin();
			for ( cl_uint ui=0; ui<m_mapDevices.size() && it_bin != m_mapBinaries.end(); ui++)
			{
				(it_bin->second)->GetBinary(0, NULL, &uiParam);
				(it_bin->second)->GetBinary(uiParam, ((char**)param_value)[ui], &uiParam);
				it_bin++;
			}
		}
		return CL_SUCCESS;
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
		delete[] pDevices;
		delete[] pSourceCode;
		return CL_INVALID_VALUE;
	}
	memcpy_s(param_value, szParamValueSize, pValue, param_value_size);
	delete[] pDevices;
	delete[] pSourceCode;

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// AddSource
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::AddSource(cl_uint uiCount, const char ** ppcStrings, const size_t * pszLengths)
{
	InfoLog(m_pLoggerClient, L"AddSource enter. uiCount=%d, ppcStrings=%d, pszLengths=%d",uiCount, ppcStrings, pszLengths);
	// check if the program wasn't initialized yet
	if (m_eProgramSourceType != PST_NONE)
	{
		ErrLog(m_pLoggerClient, L"m_eProgramSourceType != PST_NONE");
		return CL_ERR_INITILIZATION_FAILED;
	}
	m_eProgramSourceType = PST_SOURCE_CODE;

	// check inputarguments
	if (m_uiStcStrCount > 0 || NULL != m_ppcSrcStrArr)
	{
		// source has allready added to this program
		ErrLog(m_pLoggerClient, L"m_uiStcStrCount > 0 || NULL != m_ppcSrcStrArr; return CL_ERR_INITILIZATION_FAILED");
		return CL_ERR_INITILIZATION_FAILED;
	}

	// add context's devices to the devices list and the binaries list;
	cl_uint uiNumDevices = 0;
	Device ** ppDevices = NULL;
	// get devices count
	cl_err_code clErrRet = m_pContext->GetDevices(0, NULL, &uiNumDevices);
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	// allocate memory for devices
	ppDevices = new Device * [uiNumDevices];
	if (NULL == ppDevices)
	{
		return CL_ERR_INITILIZATION_FAILED;
	}
	clErrRet = m_pContext->GetDevices(uiNumDevices, ppDevices, NULL);
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		Device * pDevice = ppDevices[ui];
		m_mapDevices[(cl_device_id)pDevice->GetId()] = pDevice;
		//m_mapBinaries[(cl_device_id)pDevice->GetId()] = NULL;
		//m_mapBinaryStatus[(cl_device_id)pDevice->GetId()] = false;
	}
	delete[] ppDevices;

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
cl_err_code Program::AddBinaries(cl_uint uiNumDevices, 
								 Device ** ppDevices, 
								 const size_t * pszBinariesSize, 
								 const void ** ppBinaries,
								 cl_int * piBinaryStatus)
{
	InfoLog(m_pLoggerClient, L"AddBinary enter. uiNumDevices=%d, ppDevices=%d, pszBinariesSize=%d, ppBinaries=%d, piBinaryStatus=%d", 
		uiNumDevices, ppDevices, pszBinariesSize, ppBinaries, piBinaryStatus);
	cl_err_code clErrRet = CL_SUCCESS;
	cl_err_code clFinalErrRet = CL_SUCCESS;
	
	// check if the program wasn't initialized yet
	if (m_eProgramSourceType != PST_NONE)
	{
		ErrLog(m_pLoggerClient, L"m_eProgramSourceType != PST_NONE");
		return CL_ERR_INITILIZATION_FAILED;
	}
	m_eProgramSourceType = PST_BINARY;

	// check input parameters
	if (0 == uiNumDevices || NULL == ppDevices || NULL == pszBinariesSize || NULL == ppBinaries)
	{
		ErrLog(m_pLoggerClient, L"0 == uiNumDevices || NULL == ppDevices || NULL == pszBinariesSize || NULL == ppBinariesData");
		return CL_INVALID_VALUE;
	}

	// check binaries and add them to the program object
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		// get device and the device is valid device
		Device * pDevice = ppDevices[ui];
		if (NULL == pDevice)
		{
			ErrLog(m_pLoggerClient, L"device number %d is not valid", ui);
			return CL_INVALID_DEVICE;
		}

		// set the device in the program's devices list
		m_mapDevices[(cl_device_id)pDevice->GetId()] = pDevice;

		// check if the current device allready have binary assign for it
		map<cl_device_id,ProgramBinary*>::iterator it = m_mapBinaries.find((cl_device_id)pDevice->GetId());
		if (it != m_mapBinaries.end())
		{
			return CL_ERR_KEY_ALLREADY_EXISTS;
		}

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
			ProgramBinary * pProgBin = new ProgramBinary(pszBinariesSize[ui], ppBinaries[ui], CL_DEV_BINARY_USER, pDevice, &clErrRet);
			if (CL_FAILED(clErrRet))
			{
				return clErrRet;
			}
			m_mapBinaries[(cl_device_id)pDevice->GetId()] = pProgBin;
			m_mapBinaryStatus[(cl_device_id)pDevice->GetId()] = true;
		}
		else
		{
			ErrLog(m_pLoggerClient, L"binary ppBinaries[%d] isn't valid for device %d", ui, pDevice->GetId());
			m_mapBinaries[(cl_device_id)pDevice->GetId()] = NULL;
			m_mapBinaryStatus[(cl_device_id)pDevice->GetId()] = false;
			clFinalErrRet = CL_INVALID_BINARY;
		}
	}
	return clFinalErrRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CheckBinaries
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::CheckBinaries(cl_uint uiNumDevices, const cl_device_id * pclDevices)
{
	InfoLog(m_pLoggerClient, L"CheckBinaries enter. uiNumDevices=%d, pclDevices=%d", uiNumDevices, pclDevices);
	// check input parameters
	if (0 == uiNumDevices || NULL == pclDevices)
	{
		ErrLog(m_pLoggerClient, L"0 == uiNumDevices || NULL == pclDevices");
		return CL_INVALID_DEVICE;
	}
	// for each device in uiNumDevices: check that the binaries exists and ready (not being built by the backend)
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		map<cl_device_id,ProgramBinary*>::iterator it = m_mapBinaries.find(pclDevices[ui]);
		if (it == m_mapBinaries.end())
		{
			ErrLog(m_pLoggerClient, L"device id %d not found in m_mapBinaries", pclDevices[ui]);
			return CL_INVALID_DEVICE;
		}
		ProgramBinary * pProgBin = it->second;
		if (NULL == pProgBin)
		{
			ErrLog(m_pLoggerClient, L"NULL == pProgBin");
			return CL_INVALID_BINARY;
		}
		Device * pDevice = (Device*)pProgBin->GetDevice();
		if (NULL == pDevice)
		{
			ErrLog(m_pLoggerClient, L"device id %d not attached to program binary", pclDevices[ui]);
			return CL_INVALID_DEVICE;
		}
		// check binary status
		cl_build_status clBuildStatus = pProgBin->GetStatus();
		if (CL_BUILD_IN_PROGRESS == clBuildStatus)
		{
			ErrLog(m_pLoggerClient, L"program binarie's status is CL_BUILD_IN_PROGRESS");
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
			ErrLog(m_pLoggerClient, L"binary of device %d isn't valid binary", pclDevices[ui]);
			return CL_INVALID_BINARY;
		}
	}
	return CL_SUCCESS;
}
cl_err_code Program::BuildBinarys(cl_uint uiNumDevices, const cl_device_id * pclDevices, const char * pcOptions)
{
	InfoLog(m_pLoggerClient, L"BuildBinarys enter. uiNumDevices=%d, pclDevices=%d", uiNumDevices, pclDevices);
	// check input parameters
	if (0 == uiNumDevices || NULL == pclDevices)
	{
		ErrLog(m_pLoggerClient, L"0 == uiNumDevices || NULL == pclDevices");
		return CL_INVALID_DEVICE;
	}
	// for each device in uiNumDevices: build binary
	for (cl_uint ui=0; ui<uiNumDevices; ++ui)
	{
		map<cl_device_id,ProgramBinary*>::iterator it = m_mapBinaries.find(pclDevices[ui]);
		if (it == m_mapBinaries.end())
		{
			ErrLog(m_pLoggerClient, L"device id %d not found in m_mapBinaries", pclDevices[ui]);
			return CL_INVALID_DEVICE;
		}
		ProgramBinary * pProgBin = it->second;
		if (NULL == pProgBin)
		{
			ErrLog(m_pLoggerClient, L"NULL == pProgBin");
			return CL_INVALID_BINARY;
		}
		IBuildDoneObserver * pBuildDoneObserver = dynamic_cast<IBuildDoneObserver*>(this);
		pProgBin->Build(pcOptions, pBuildDoneObserver);
	}
	return CL_SUCCESS;
}
cl_err_code Program::Build(cl_uint uiNumDevices,
						   const cl_device_id *	pclDeviceList,
						   const char *	pcOptions,
						   void (*pfnNotify)(cl_program clProgram, void * pUserData),
						   void * pUserData)
{
	InfoLog(m_pLoggerClient, L"Build enter. uiNumDevices=%d, pclDeviceList=%d, pcOptions=%d, pUserData=%d", 
		uiNumDevices, pclDeviceList, pcOptions, pUserData);

	if (m_eProgramSourceType == PST_SOURCE_CODE)
	{
		ErrLog(m_pLoggerClient, L"m_eProgramSourceType == PST_SOURCE_CODE; currently not supported");
		return CL_ERR_NOT_SUPPORTED;
		// TODO: support fe compilation - build IR and assign for each device
	}

	// check invalid input
	if (uiNumDevices > 0 && NULL == pclDeviceList	||
		uiNumDevices == 0 && NULL != pclDeviceList )
	{
		ErrLog(m_pLoggerClient, L"uiNumDevices > 0 && NULL == pclDeviceList || uiNumDevices == 0 && NULL != pclDeviceList");
		return CL_INVALID_VALUE;
	}

	// chack devices and binaries
	cl_err_code clErrRet = CheckBinaries(uiNumDevices, pclDeviceList);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"CheckBinaries(%d, %d)=%d", uiNumDevices, pclDeviceList, clErrRet);
		return clErrRet;
	}

	// TODO: check if kernels attached to the program

	m_pUserData = pUserData;
	m_pfnNotify = pfnNotify;

	clErrRet = BuildBinarys(uiNumDevices, pclDeviceList, pcOptions);

	return clErrRet;
}

cl_err_code Program::NotifyBuildDone(cl_device_id device, cl_build_status build_status)
{
		bool bFinished = true;
		OclAutoMutex CS(&m_UserNotifyCS);
		{ // Lock
			map<cl_device_id, ProgramBinary*>::iterator it = m_mapBinaries.begin();
			while (it != m_mapBinaries.end())
			{
				cl_build_status status = (it->second)->GetStatus();
				if (status == CL_BUILD_NONE || status == CL_BUILD_IN_PROGRESS)
				{
					bFinished = false;
				}
				it++;
			}
		} // Unlock
		if (true == bFinished && NULL != m_pfnNotify)
		{
			m_pfnNotify((cl_program)m_iId, m_pUserData);
		}
		return CL_SUCCESS;
}
cl_err_code Program::CreateKernel(const char * psKernelName, Kernel ** ppKernel)
{
	InfoLog(m_pLoggerClient, L"CreateKernel enter. pscKernelName=%s, ppKernel=%d", psKernelName, ppKernel);
	if (NULL == psKernelName || NULL == ppKernel)
	{
		return CL_INVALID_VALUE;
	}
	// check executables
	ProgramBinary ** ppBinaries = new ProgramBinary * [m_mapBinaries.size()];
	if (NULL == ppBinaries)
	{
		ErrLog(m_pLoggerClient, L"new ProgramBinary * [%d] == NULL", m_mapBinaries.size());
		return CL_OUT_OF_HOST_MEMORY;
	}
	map<cl_device_id, ProgramBinary*>::iterator it = m_mapBinaries.begin();
	for (cl_uint ui=0; ui<m_mapBinaries.size() && it != m_mapBinaries.end(); ++ui)
	{
		ProgramBinary * pProgBin = it->second;
		cl_build_status clBuildStatus = pProgBin->GetStatus();
		if (clBuildStatus != CL_BUILD_SUCCESS)
		{
			// missing executable in one of the devices
			ErrLog(m_pLoggerClient, L"device %s doesn't have valid executable", it->first);
			delete[] ppBinaries;
			return CL_INVALID_PROGRAM_EXECUTABLE;
		}
		ppBinaries[ui] = pProgBin;
		it++;
	}
	
	// create new kernel object;
	Kernel * pKernel = new Kernel(this, psKernelName);

	// for each device, create deivce kernel 
	cl_err_code clErrRet = CL_SUCCESS;
	DeviceKernel * pDeviceKernel = NULL;

	clErrRet = pKernel->CreateDeviceKernels(m_mapBinaries.size(), ppBinaries);
	if (CL_FAILED(clErrRet))
	{
		ErrLog(m_pLoggerClient, L"pKernel->CreateDeviceKernels(%d, ppBinaries) = %ws", m_mapBinaries.size(), ClErrTxt(clErrRet));
		pKernel->Release();
		delete pKernel;
		delete[] ppBinaries;
		return clErrRet;
	}

	m_pKernels->AddObject((OCLObject*)pKernel);
	*ppKernel = pKernel;
	delete[] ppBinaries;
	return CL_SUCCESS;
}
cl_err_code Program::CreateAllKernels(cl_uint uiNumKernels, cl_kernel * pclKernels, cl_uint * puiNumKernelsRet)
{
	return CL_ERR_NOT_IMPLEMENTED;
}