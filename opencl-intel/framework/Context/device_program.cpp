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
//  device_program.cpp
//  Implementation of the DeviceProgram class
//  Created on:      28-Jul-2010
//  Original author: Doron Singer
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "device_program.h"
#include "device.h"
#include "framework_proxy.h"
#include "events_manager.h"
#include "fe_compiler.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

DeviceProgram::DeviceProgram() : m_state(DEVICE_PROGRAM_INVALID), m_pDevice(NULL), m_deviceHandle(0), m_programHandle(0), m_parentProgramHandle(0),
m_pUserData(NULL), m_pfn(NULL), m_pBuildOptions(NULL), m_currentAccesses(0), m_pBinaryBits(NULL), m_szBinaryBitsSize(0), 
m_uiNumStrings(0), m_pszStringLengths(NULL), m_pSourceStrings(NULL), m_pFeBuildEvent(NULL), m_pBeBuildEvent(NULL), m_emptyString('\0'), 
m_bBuiltFromSource(false), m_bFECompilerSuccess(false)
{
	//initialize build log to be the empty string
	m_szBuildLog = 1;
	m_pBuildLog  = &m_emptyString;
}

DeviceProgram::~DeviceProgram()
{
	if (m_pBinaryBits)
	{
		delete[] m_pBinaryBits;
		m_pBinaryBits = NULL;
		m_szBinaryBitsSize = 0;
	}
	if (m_pBuildOptions)
	{
		delete[] m_pBuildOptions;
		m_pBuildOptions = NULL;
	}
	if (m_pBuildLog != NULL && m_pBuildLog != &m_emptyString)
	{
		delete[] m_pBuildLog;
		m_pBuildLog  = &m_emptyString;
		m_szBuildLog = 1;
	}
	if (m_pDevice)
	{
		if (0 != m_programHandle)
		{
			m_pDevice->GetDeviceAgent()->clDevReleaseProgram(m_programHandle);
		}
		m_pDevice->RemovePendency();
	}
}

void DeviceProgram::SetDevice(Device* pDevice)
{
	//Must not call this twice
	assert(!m_pDevice);
	m_pDevice = pDevice;
	//Must not give NULL ptr
	assert(m_pDevice);
	m_pDevice->AddPendency();
	m_deviceHandle = m_pDevice->GetHandle();
}

cl_err_code DeviceProgram::SetBinary(size_t szBinarySize, const unsigned char *pBinary, cl_int *iBinaryStatus)
{
	cl_int iBinStauts = m_pDevice->GetDeviceAgent()->clDevCheckProgramBinary(szBinarySize, pBinary);
	// check binary
	if (iBinaryStatus)
	{
		*iBinaryStatus = iBinStauts;
	}

	// if binary is valid binary create program binary object and add it to the program object
	if (CL_SUCCEEDED(iBinStauts))
	{
		cl_err_code clErrRet = CopyBinary(szBinarySize, pBinary);
		if (CL_SUCCEEDED(clErrRet))
		{
			m_state = DEVICE_PROGRAM_EXTERNAL_BIN;
			clErrRet = m_pDevice->GetDeviceAgent()->clDevCreateProgram(m_szBinaryBitsSize, m_pBinaryBits, CL_DEV_BINARY_USER, &m_programHandle);
		}
		return clErrRet;
	}
	else
	{
		m_state = DEVICE_PROGRAM_INVALID;
		return CL_INVALID_BINARY;
	}
	return CL_SUCCESS;
}

void DeviceProgram::SetSource(cl_uint uiNumStrings, size_t* pszLengths, const char** pSourceStrings)
{
	m_uiNumStrings     = uiNumStrings;
	m_pszStringLengths = pszLengths;
	m_pSourceStrings   = pSourceStrings;
	m_state            = DEVICE_PROGRAM_SOURCE;
	m_bBuiltFromSource = true;
}

bool DeviceProgram::Acquire()
{
	if (0 == m_currentAccesses++)
	{
		return true;
	}
	m_currentAccesses--;
	return false;
}

cl_err_code DeviceProgram::Build(const char * pcOptions, pfnNotifyBuildDone pfn, void* pUserData)
{
	cl_err_code clErrRet = CL_SUCCESS;
	switch (m_state)
	{
	case DEVICE_PROGRAM_MACHINE_CODE:
		//We're already built. If build options changed, build again. Otherwise, report success immediately.
		if (((NULL == m_pBuildOptions) && (NULL == pcOptions)) ||
			((NULL != m_pBuildOptions) && (NULL != pcOptions) && (0 == strcmp(pcOptions, m_pBuildOptions))))
		{
			//No changes in build options, so effectively the build can succeed vacuously
			if (pfn)
			{
				pfn(m_parentProgramHandle, pUserData);
			}
			return CL_SUCCESS;
		}
		//Else: some changes to build options. Need to build.
		m_state = DEVICE_PROGRAM_INVALID;
		//Intentional fall through.

	case DEVICE_PROGRAM_INVALID:
		// Possibly retrying a failed build - legal
		if (!m_bBuiltFromSource)
		{
			//invalid binaries are hopeless, report the appropriate failure
			return CL_INVALID_BINARY;
		}
	case DEVICE_PROGRAM_SOURCE:		 
		// Building from source
	case DEVICE_PROGRAM_EXTERNAL_BIN:
		// Building from external bin
	case DEVICE_PROGRAM_INTERNAL_IR: 
		// Todo: is this legal?
		break;

	default:
	case DEVICE_PROGRAM_FE_BUILDING:
	case DEVICE_PROGRAM_BE_BUILDING:
		// Illegal state: building the same program twice
		return CL_INVALID_OPERATION;
	}

	CopyBuildOptions(pcOptions);
	m_pfn       = pfn;
	m_pUserData = pUserData;

	m_pBeBuildEvent = FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->CreateBuildEvent(m_parentProgramContext);
	if (!m_pBeBuildEvent)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	if (!m_pfn)
	{
		//Ensure no race between BE build finishing and our wait
		m_pBeBuildEvent->AddPendency();
	}

	//Build from sources
	if (DEVICE_PROGRAM_SOURCE == m_state || ((DEVICE_PROGRAM_INVALID == m_state) && m_bBuiltFromSource))
	{
		m_state  = DEVICE_PROGRAM_FE_BUILDING;
		clErrRet = FeBuild();
	}
	else
	{
		clErrRet = BeBuild();
		if (!m_pfn)
		{
			if (CL_SUCCEEDED(clErrRet))
			{
				m_pBeBuildEvent->Wait();
				clErrRet = m_pBeBuildEvent->GetReturnCode();
			}
			m_pBeBuildEvent->RemovePendency();
		}
	}
	return clErrRet;
}

cl_err_code DeviceProgram::FeBuild()
{
	FECompiler* pFeCompiler = NULL;

	pFeCompiler = const_cast<FECompiler *>(m_pDevice->GetFECompiler());
	if (NULL == pFeCompiler) //attempt to get global FE compiler
	{
		pFeCompiler = const_cast<FECompiler *>(FrameworkProxy::Instance()->GetPlatformModule()->GetDefaultFECompiler());
	}

	if (NULL == pFeCompiler)
	{
		FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pBeBuildEvent->GetHandle());
		if (!m_pfn)
		{
			m_pBeBuildEvent->RemovePendency();
		}
		return CL_COMPILER_NOT_AVAILABLE;
	}

	//Todo: why not? pFeCompiler->AddPendency();

	m_pFeBuildEvent = FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->CreateBuildEvent(m_parentProgramContext);
	if (!m_pFeBuildEvent)
	{
		FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pBeBuildEvent->GetHandle());
		if (!m_pfn)
		{
			m_pBeBuildEvent->RemovePendency();
		}
		return CL_OUT_OF_HOST_MEMORY;
	}

	m_pBeBuildEvent->AddDependentOn(m_pFeBuildEvent);

	cl_err_code clErr = pFeCompiler->BuildProgram(m_deviceHandle, m_uiNumStrings, m_pSourceStrings, m_pszStringLengths, m_pBuildOptions, static_cast<IFrontendBuildDoneObserver *>(this));
	if (CL_FAILED(clErr))
	{
		FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pBeBuildEvent->GetHandle());
		FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pFeBuildEvent->GetHandle());
		if (!m_pfn)
		{
			m_pBeBuildEvent->RemovePendency();
		}
		return CL_BUILD_PROGRAM_FAILURE;
	}

	if (!m_pfn) //blocking build
	{
		m_pBeBuildEvent->Wait();
		clErr = m_pBeBuildEvent->GetReturnCode();
		m_pBeBuildEvent->RemovePendency();
	}
	return clErr;
}

cl_err_code DeviceProgram::BeBuild()
{
	assert (DEVICE_PROGRAM_EXTERNAL_BIN == m_state || DEVICE_PROGRAM_INTERNAL_IR == m_state || ((DEVICE_PROGRAM_INVALID == m_state) && !m_bBuiltFromSource));
	m_state = DEVICE_PROGRAM_BE_BUILDING;
	cl_int iRet = m_pDevice->GetDeviceAgent()->clDevBuildProgram(m_programHandle, m_pBuildOptions, dynamic_cast<IBuildDoneObserver *>(this));
	if (CL_FAILED(iRet))
	{
		FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pBeBuildEvent->GetHandle());
	}
	return (cl_err_code)iRet;
}

cl_err_code DeviceProgram::NotifyFEBuildDone(cl_device_id device, size_t szBinSize, void * pBinData, const char *pBuildLog)
{
	if (NULL != pBuildLog)
	{
		m_szBuildLog = strlen(pBuildLog) + 1;
		m_pBuildLog  = new char[m_szBuildLog];
		if (NULL == m_pBuildLog)
		{
			m_szBuildLog = 0;
		}
		else
		{
			memcpy_s(m_pBuildLog, m_szBuildLog, pBuildLog, m_szBuildLog);
		}
	}
	if (0 == szBinSize)
	{
		//Build failed
		m_bFECompilerSuccess = false;
		m_pFeBuildEvent->SetComplete(CL_BUILD_PROGRAM_FAILURE);
		FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pFeBuildEvent->GetHandle());
		NotifyBuildDone(m_deviceHandle, CL_BUILD_ERROR);
		return CL_BUILD_PROGRAM_FAILURE;
	}

	//Else FE build succeeded
	m_bFECompilerSuccess = true;
	CopyBinary(szBinSize, static_cast<unsigned char*>(pBinData));
	m_state = DEVICE_PROGRAM_INTERNAL_IR;
	m_pFeBuildEvent->SetComplete(CL_SUCCESS);
	FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pFeBuildEvent->GetHandle());
	m_pFeBuildEvent = NULL;

	cl_err_code clErrRet = m_pDevice->GetDeviceAgent()->clDevCreateProgram(m_szBinaryBitsSize, m_pBinaryBits, CL_DEV_BINARY_COMPILER, &m_programHandle);
	if (CL_FAILED(clErrRet))
	{
		m_pBeBuildEvent->SetComplete(CL_BUILD_PROGRAM_FAILURE);
		FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pBeBuildEvent->GetHandle());
		return clErrRet;
	}
	//Todo: do we want this action to drive the BE build or not?
	return BeBuild();
}

cl_err_code DeviceProgram::NotifyBuildDone(cl_device_id device, cl_build_status build_status)
{
	assert(CL_BUILD_ERROR == build_status || CL_BUILD_SUCCESS == build_status);

	if (CL_BUILD_ERROR == build_status)
	{
		m_state = DEVICE_PROGRAM_INVALID;
		m_pBeBuildEvent->SetComplete(CL_BUILD_PROGRAM_FAILURE);
	}
	else
	{
		m_state = DEVICE_PROGRAM_MACHINE_CODE;
		m_pBeBuildEvent->SetComplete(CL_SUCCESS);
	}

	if (m_pfn)
	{
		m_pfn(m_parentProgramHandle, m_pUserData);
	}
	FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pBeBuildEvent->GetHandle());
	return CL_SUCCESS;
}

cl_err_code DeviceProgram::CopyBinary(size_t szBinarySize, const unsigned char* pBinary)
{
	assert(DEVICE_PROGRAM_INVALID == m_state || DEVICE_PROGRAM_FE_BUILDING == m_state);
	if (m_szBinaryBitsSize > 0)
	{
		assert(m_pBinaryBits);
		delete[] m_pBinaryBits;
	}
	m_szBinaryBitsSize = szBinarySize;
	m_pBinaryBits      = new char[szBinarySize];
	if (!m_pBinaryBits)
	{
		m_szBinaryBitsSize = 0;
		return CL_OUT_OF_HOST_MEMORY;
	}

	//Todo: not portable
	memcpy_s(m_pBinaryBits, m_szBinaryBitsSize, pBinary, m_szBinaryBitsSize);

	return CL_SUCCESS;
}

cl_err_code DeviceProgram::CopyBuildOptions(const char* pcBuildOptions)
{
	if (m_pBuildOptions)
	{
		delete[] m_pBuildOptions;
		m_pBuildOptions = NULL;
	}
	if (pcBuildOptions)
	{
		size_t szOptionLength = strlen(pcBuildOptions) + 1;
		m_pBuildOptions = new char[szOptionLength];
		if (!m_pBuildOptions)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}
		memcpy_s(m_pBuildOptions, szOptionLength, pcBuildOptions, szOptionLength);
	}
	return CL_SUCCESS;
}



cl_build_status DeviceProgram::GetBuildStatus() const 
{
	switch (m_state)
	{
	default:
	case DEVICE_PROGRAM_INVALID:
		return CL_BUILD_ERROR;

	case DEVICE_PROGRAM_SOURCE:
	case DEVICE_PROGRAM_EXTERNAL_BIN:
		return CL_BUILD_NONE;

	case DEVICE_PROGRAM_FE_BUILDING:
	case DEVICE_PROGRAM_INTERNAL_IR:
	case DEVICE_PROGRAM_BE_BUILDING:
		return CL_BUILD_IN_PROGRESS;

	case DEVICE_PROGRAM_MACHINE_CODE:
		return CL_BUILD_SUCCESS;
	}

	return CL_BUILD_ERROR;
}

cl_err_code DeviceProgram::GetBuildInfo(cl_program_build_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pzsParamValueSizeRet)
{
	cl_err_code clErr = CL_SUCCESS;
	if ((NULL == pParamValue && NULL == pzsParamValueSizeRet)	||
		(NULL == pParamValue && szParamValueSize > 0))
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamSize = 0;
	void * pValue = NULL;
	cl_build_status clBuildStatus;
	char emptyString = '\0';

	switch (clParamName)
	{
	case CL_PROGRAM_BUILD_STATUS:
		szParamSize = sizeof(cl_build_status);
		clBuildStatus = GetBuildStatus();
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
		switch (m_state)
		{
		default:
		case DEVICE_PROGRAM_SOURCE:
		case DEVICE_PROGRAM_EXTERNAL_BIN:
		case DEVICE_PROGRAM_FE_BUILDING:
			szParamValueSize = 1;
			pValue           = &emptyString;
			break;

		case DEVICE_PROGRAM_INTERNAL_IR:
		case DEVICE_PROGRAM_BE_BUILDING:
			szParamSize = m_szBuildLog;
			pValue      = m_pBuildLog;
			break;

		case DEVICE_PROGRAM_INVALID:
			if (!m_bFECompilerSuccess)
			{
				szParamSize = m_szBuildLog;
				pValue      = m_pBuildLog;
				break;
			}

			//Intentional fall through to get BE build log

		case DEVICE_PROGRAM_MACHINE_CODE:
			// still need to append the FE build log
			// First of all calculate the size
			clErr = m_pDevice->GetDeviceAgent()->clDevGetBuildLog(m_programHandle, 0, NULL, &szParamSize);
			if CL_DEV_FAILED(clErr)
			{
				return CL_INVALID_VALUE;
			}
			szParamSize += m_szBuildLog;
			if (NULL != pParamValue && szParamSize > szParamValueSize)
			{
				return CL_INVALID_VALUE;
			}

			// if pParamValue == NULL return param value size
			if (NULL != pzsParamValueSizeRet)
			{
				*pzsParamValueSizeRet = szParamSize;
			}

			// get the actual log
			if (NULL != pParamValue)
			{
				//Copy the FE log minus the terminating NULL
				memcpy_s(pParamValue, szParamValueSize, m_pBuildLog, m_szBuildLog - 1);
				// and let the device write the rest of the log
				szParamSize -= m_szBuildLog;
				clErr = m_pDevice->GetDeviceAgent()->clDevGetBuildLog(m_programHandle, szParamSize, ((char*)pParamValue) + m_szBuildLog - 1, NULL);
			}

			return clErr;
			break;
		}

		break;

	default:
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

cl_err_code DeviceProgram::GetBinary(cl_uint uiBinSize, void * pBin, cl_uint * puiBinSizeRet)
{
	if (NULL == pBin && NULL == puiBinSizeRet)
	{
		return CL_INVALID_VALUE;
	}

	if (uiBinSize > 0 && NULL == pBin)
	{
		return CL_INVALID_VALUE;
	}

	switch(m_state)
	{
	case DEVICE_PROGRAM_MACHINE_CODE:
		//Return the resultant compiled binaries
		return m_pDevice->GetDeviceAgent()->clDevGetProgramBinary(m_programHandle, uiBinSize, pBin, puiBinSizeRet);

	case DEVICE_PROGRAM_EXTERNAL_BIN:
		//Return the user-supplied IR
		if (!pBin)
		{
			*puiBinSizeRet = m_szBinaryBitsSize;
			return CL_SUCCESS;
		}
		if (uiBinSize < m_szBinaryBitsSize)
		{
			return CL_INVALID_VALUE;
		}
		memcpy_s(pBin, uiBinSize, m_pBinaryBits, m_szBinaryBitsSize);
		return CL_SUCCESS;

	default:
		// In every other case, we have nothing intelligent to return
		// Todo: see if it's reasonable to return IR if BE is still building in ProgramWithSource
		// Todo: see what I should return here
		return CL_INVALID_VALUE;
	}
}

cl_err_code DeviceProgram::GetNumKernels(size_t* pszNumKernels)
{
	assert(pszNumKernels);
	return m_pDevice->GetDeviceAgent()->clDevGetProgramKernels(m_programHandle, 0, NULL, pszNumKernels);
}

cl_err_code DeviceProgram::GetKernelNames(char **ppNames, size_t *pszNameSizes, size_t szNumNames)
{
	size_t         numKernels;
	cl_err_code    errRet     = CL_SUCCESS;
	cl_dev_kernel* devKernels = new cl_dev_kernel[szNumNames];

	if (!devKernels)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	if (!pszNameSizes)
	{
		delete[] devKernels;
		return CL_INVALID_VALUE;
	}

	errRet = m_pDevice->GetDeviceAgent()->clDevGetProgramKernels(m_programHandle, szNumNames, devKernels, &numKernels);
	if (CL_FAILED(errRet))
	{
		delete[] devKernels;
		return errRet;
	}
	assert(numKernels == szNumNames);

	if (NULL == ppNames)
	{
		for (size_t i = 0; i < numKernels; ++i)
		{
			errRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(devKernels[i], CL_DEV_KERNEL_NAME, 0, NULL, pszNameSizes + i);
			if (CL_FAILED(errRet))
			{
				delete[] devKernels;
				return errRet;
			}
		}
		delete[] devKernels;
		return CL_SUCCESS;
	}

	for (size_t i = 0; i < numKernels; ++i)
	{
		size_t kernelNameSize;
		errRet = m_pDevice->GetDeviceAgent()->clDevGetKernelInfo(devKernels[i], CL_DEV_KERNEL_NAME, pszNameSizes[i], ppNames[i], &kernelNameSize);
		if (CL_FAILED(errRet))
		{
			delete[] devKernels;
			return errRet;
		}
		assert(kernelNameSize == pszNameSizes[i]);
	}
	return CL_SUCCESS;
}