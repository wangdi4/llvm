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
#include "framework_proxy.h"
#include "Device.h"
#include "events_manager.h"
#include "fe_compiler.h"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;


DeviceProgram::DeviceProgram() : OCLObjectBase("DeviceProgram"), m_state(DEVICE_PROGRAM_INVALID), m_bBuiltFromSource(false), m_bFECompilerSuccess(false), m_bIsClone(false), 
m_pDevice(NULL), m_deviceHandle(0), m_programHandle(0), m_parentProgramHandle(0),
m_pUserData(NULL), m_pfn(NULL), m_pBuildOptions(NULL), m_pBuildEvent(NULL),
m_uiNumStrings(0), m_pszStringLengths(NULL), m_pSourceStrings(NULL), m_emptyString('\0'), 
m_pBinaryBits(NULL), m_szBinaryBitsSize(0), m_currentAccesses(0)
{
	//initialize build log to be the empty string
	m_szBuildLog = 1;
	m_pBuildLog  = &m_emptyString;
}

DeviceProgram::DeviceProgram(const Intel::OpenCL::Framework::DeviceProgram &dp) : OCLObjectBase("DeviceProgram"), m_state(DEVICE_PROGRAM_INVALID), m_bBuiltFromSource(false), 
m_bFECompilerSuccess(false), m_bIsClone(true), m_pDevice(NULL), m_deviceHandle(0), m_programHandle(0), m_parentProgramHandle(0),
m_pUserData(NULL), m_pfn(NULL), m_pBuildOptions(NULL), m_pBuildEvent(NULL),
m_uiNumStrings(0), m_pszStringLengths(NULL), m_pSourceStrings(NULL), m_emptyString('\0'), 
m_pBinaryBits(NULL), m_szBinaryBitsSize(0), m_currentAccesses(0)
{
    SetDevice(dp.m_pDevice);
    SetHandle(dp.m_parentProgramHandle);
    SetContext(dp.m_parentProgramContext);
    m_bBuiltFromSource   = dp.m_bBuiltFromSource;
    m_bFECompilerSuccess = dp.m_bFECompilerSuccess;
    //Current spec doesn't support cloning programs created from binaries
    assert(m_bBuiltFromSource);
    if (m_bBuiltFromSource)
    {
        SetSource(dp.m_uiNumStrings, dp.m_pszStringLengths, dp.m_pSourceStrings);
    }
    //Todo: in the future it's a good idea to copy a completed binary from my source, or to add myself as an observer for its completion
    // Currently, force a real re-build of the program even if we're copying a built program
    // Thus, no use for m_bIsClone currently
    m_bIsClone = false;
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
		m_pDevice->RemovePendency(this);
	}
}

void DeviceProgram::SetDevice(FissionableDevice* pDevice)
{
	m_pDevice = pDevice;
	//Must not give NULL ptr
	assert(m_pDevice);
	m_pDevice->AddPendency(this);
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
		//Else: some changed to build options. Need to build.
		m_state = DEVICE_PROGRAM_INVALID;
		//Intetional fall through.

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

	m_pBuildEvent = FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->CreateBuildEvent(m_parentProgramContext);
	if (!m_pBuildEvent)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	if (!m_pfn)
	{
		//Ensure no race between BE build finishing and our wait
		m_pBuildEvent->AddPendency(this);
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
				m_pBuildEvent->Wait();
				clErrRet = m_pBuildEvent->GetReturnCode();
			}
			m_pBuildEvent->RemovePendency(this);
		}
	}
	return clErrRet;
}

cl_err_code DeviceProgram::FeBuild()
{
	FrontEndCompiler* pFeCompiler = NULL;

	pFeCompiler = const_cast<FrontEndCompiler *>(m_pDevice->GetRootDevice()->GetFrontEndCompiler());
	if (NULL == pFeCompiler)
	{
		// No FE compiler assigned, need to allocate one
		FrameworkProxy::Instance()->GetPlatformModule()->InitFECompiler(m_pDevice->GetRootDevice());
		pFeCompiler = const_cast<FrontEndCompiler *>(m_pDevice->GetRootDevice()->GetFrontEndCompiler());
	}

	if (NULL == pFeCompiler)
	{
		FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pBuildEvent->GetHandle());
		if (!m_pfn)
		{
			m_pBuildEvent->RemovePendency(this);
		}
		return CL_COMPILER_NOT_AVAILABLE;
	}

	//Todo: why not? pFeCompiler->AddPendency(this);

	cl_err_code clErr = pFeCompiler->BuildProgram(m_deviceHandle, m_uiNumStrings, m_pSourceStrings, m_pszStringLengths, m_pBuildOptions, static_cast<IFrontendBuildDoneObserver *>(this));
	if (CL_FAILED(clErr))
	{
		FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pBuildEvent->GetHandle());
		if (!m_pfn)
		{
			m_pBuildEvent->RemovePendency(this);
		}
		return CL_BUILD_PROGRAM_FAILURE;
	}

	if (!m_pfn) //blocking build
	{
		m_pBuildEvent->Wait();
		clErr = m_pBuildEvent->GetReturnCode();
		m_pBuildEvent->RemovePendency(this);
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
		FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pBuildEvent->GetHandle());
	}
	return (cl_err_code)iRet;
}

cl_err_code DeviceProgram::NotifyFEBuildDone(cl_device_id device, size_t szBinSize, const void * pBinData, const char *pBuildLog)
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
			MEMCPY_S(m_pBuildLog, m_szBuildLog, pBuildLog, m_szBuildLog);
		}
	}
	if (0 == szBinSize)
	{
		//Build failed
		m_bFECompilerSuccess = false;
		NotifyBuildDone(m_deviceHandle, CL_BUILD_ERROR);
		return CL_BUILD_PROGRAM_FAILURE;
	}

	//Else FE build succeeded
	m_bFECompilerSuccess = true;
	CopyBinary(szBinSize, static_cast<const unsigned char*>(pBinData));
	m_state = DEVICE_PROGRAM_INTERNAL_IR;

	cl_err_code clErrRet = m_pDevice->GetDeviceAgent()->clDevCreateProgram(m_szBinaryBitsSize, m_pBinaryBits, CL_DEV_BINARY_COMPILER, &m_programHandle);
	if (CL_FAILED(clErrRet))
	{
		m_pBuildEvent->SetComplete(CL_BUILD_PROGRAM_FAILURE);
		FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pBuildEvent->GetHandle());
		return clErrRet;
	}
	//Todo: do we want this action to drive the BE build or not?
	return BeBuild();
}

cl_err_code DeviceProgram::NotifyBuildDone(cl_device_id device, cl_build_status build_status)
{
	assert(CL_BUILD_ERROR == build_status || CL_BUILD_SUCCESS == build_status);

	cl_int iRetCode;

	FrameworkProxy::Instance()->GetExecutionModule()->GetEventsManager()->ReleaseEvent(m_pBuildEvent->GetHandle());

	if (CL_BUILD_ERROR == build_status)
	{
		m_state = DEVICE_PROGRAM_INVALID;
		iRetCode = CL_BUILD_PROGRAM_FAILURE;
	}
	else
	{
		m_state = DEVICE_PROGRAM_MACHINE_CODE;
		iRetCode = CL_SUCCESS;
	}

	if (m_pfn)
	{
		m_pfn(m_parentProgramHandle, m_pUserData);
	} else
	{
		m_pBuildEvent->SetComplete(iRetCode);
	}

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
	MEMCPY_S(m_pBinaryBits, m_szBinaryBitsSize, pBinary, m_szBinaryBitsSize);

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
		MEMCPY_S(m_pBuildOptions, szOptionLength, pcBuildOptions, szOptionLength);
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
			if ( '\0' != m_pBuildLog[0] )
			{
				szParamSize += m_szBuildLog;
			}
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
				if ( '\0' != m_pBuildLog[0] )
				{
					//Copy the FE log minus the terminating NULL
					MEMCPY_S(pParamValue, szParamValueSize, m_pBuildLog, m_szBuildLog - 1);
					// and let the device write the rest of the log
					szParamSize -= m_szBuildLog;
				}
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
		MEMCPY_S(pParamValue, szParamValueSize, pValue, szParamSize);
	}

	return CL_SUCCESS;
}

cl_err_code DeviceProgram::GetBinary(size_t uiBinSize, void * pBin, size_t * puiBinSizeRet)
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
		if ( NULL == pBin)
		{
			assert(m_szBinaryBitsSize <= CL_MAX_UINT32);
			*puiBinSizeRet = (cl_uint)m_szBinaryBitsSize;
			return CL_SUCCESS;
		}
		if (uiBinSize < m_szBinaryBitsSize)
		{
			return CL_INVALID_VALUE;
		}
		MEMCPY_S(pBin, uiBinSize, m_pBinaryBits, m_szBinaryBitsSize);
		return CL_SUCCESS;

	default:
		if ( NULL == pBin)	// When query for binary size and it's not available, we should return 0
		{
			*puiBinSizeRet = 0;
			return CL_SUCCESS;
		}
		// In every other case, we have nothing intelligent to return
		// Todo: see if it's reasonable to return IR if BE is still building in ProgramWithSource
		// Todo: see what I should return here
		return CL_INVALID_VALUE;
	}
}

cl_err_code DeviceProgram::GetNumKernels(cl_uint* pszNumKernels)
{
	assert(pszNumKernels);
	return m_pDevice->GetDeviceAgent()->clDevGetProgramKernels(m_programHandle, 0, NULL, pszNumKernels);
}

cl_err_code DeviceProgram::GetKernelNames(char **ppNames, size_t *pszNameSizes, size_t szNumNames)
{
	cl_uint         numKernels;
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
	assert(szNumNames <= CL_MAX_UINT32);
	errRet = m_pDevice->GetDeviceAgent()->clDevGetProgramKernels(m_programHandle, (cl_uint)szNumNames, devKernels, &numKernels);
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
