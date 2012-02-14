// Copyright (c) 2006-2012 Intel Corporation
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


DeviceProgram::DeviceProgram() : OCLObjectBase("DeviceProgram"), m_state(DEVICE_PROGRAM_INVALID), 
m_bBuiltFromSource(false), m_bFECompilerSuccess(false), m_bIsClone(false), m_pDevice(NULL), 
m_deviceHandle(0), m_programHandle(0), m_parentProgramHandle(0), m_parentProgramContext(0),
m_uiBuildLogSize(0), m_szBuildLog(NULL), m_emptyString('\0'), m_szBuildOptions(NULL), 
m_pBinaryBits(NULL), m_uiBinaryBitsSize(0), m_currentAccesses(0)
{
}

DeviceProgram::DeviceProgram(const Intel::OpenCL::Framework::DeviceProgram &dp) : 
OCLObjectBase("DeviceProgram"), m_state(DEVICE_PROGRAM_INVALID), m_bBuiltFromSource(false), 
m_bFECompilerSuccess(false), m_bIsClone(true), m_pDevice(NULL), m_deviceHandle(0), 
m_programHandle(0), m_parentProgramHandle(0), m_emptyString('\0'), m_szBuildOptions(NULL),
m_pBinaryBits(NULL), m_uiBinaryBitsSize(0), m_currentAccesses(0)
{
    SetDevice(dp.m_pDevice);
    SetHandle(dp.m_parentProgramHandle);
    SetContext(dp.m_parentProgramContext);
    m_bBuiltFromSource   = dp.m_bBuiltFromSource;
    m_bFECompilerSuccess = dp.m_bFECompilerSuccess;
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
		m_uiBinaryBitsSize = 0;
	}
    if (m_szBuildOptions)
	{
		delete[] m_szBuildOptions;
		m_szBuildOptions = NULL;
	}
	if (m_szBuildLog != NULL)
	{
		delete[] m_szBuildLog;
		m_szBuildLog  = NULL;
		m_uiBuildLogSize = 0;
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

cl_err_code DeviceProgram::SetBinary(size_t uiBinarySize, const unsigned char* pBinary, cl_int* piBinaryStatus)
{
	cl_int iBinaryStatus = m_pDevice->GetDeviceAgent()->clDevCheckProgramBinary(uiBinarySize, pBinary);
	// check binary
	if (piBinaryStatus)
	{
		*piBinaryStatus = iBinaryStatus;
	}

	// if binary is valid binary create program binary object and add it to the program object
	if (CL_FAILED(iBinaryStatus))
	{
        m_state = DEVICE_PROGRAM_INVALID;
        return CL_INVALID_BINARY;
	}

    return SetBinaryInternal(uiBinarySize, pBinary);
}

cl_err_code DeviceProgram::SetBinaryInternal(size_t uiBinarySize, const void *pBinary)
 {
	if (m_uiBinaryBitsSize > 0)
 	{
		assert(m_pBinaryBits);
		delete[] m_pBinaryBits;
 	}
	m_uiBinaryBitsSize = uiBinarySize;
	m_pBinaryBits      = new char[uiBinarySize];
	if (!m_pBinaryBits)
	{
		m_uiBinaryBitsSize = 0;
		return CL_OUT_OF_HOST_MEMORY;
	}

	MEMCPY_S(m_pBinaryBits, m_uiBinaryBitsSize, pBinary, m_uiBinaryBitsSize);

	return CL_SUCCESS;
}

cl_err_code DeviceProgram::ClearBuildLogInternal()
{
    if (m_szBuildLog)
    {
        delete[] m_szBuildLog;
        m_szBuildLog = NULL;
    }

    return CL_SUCCESS;
}

cl_err_code DeviceProgram::SetBuildLogInternal(const char* szBuildLog)
{
    size_t uiLogSize = strlen(szBuildLog) + 1;

    if (m_szBuildLog)
    {
        size_t uiNewBuildLogSize = m_uiBuildLogSize + uiLogSize - 1;   //no need for two NULL termination

        char* szNewBuildLog = new char[uiNewBuildLogSize];
        if (!szNewBuildLog)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        STRCPY_S(szNewBuildLog, uiNewBuildLogSize, m_szBuildLog);
        STRCAT_S(szNewBuildLog, uiNewBuildLogSize, szBuildLog);

        m_uiBuildLogSize = uiNewBuildLogSize;
        delete[] m_szBuildLog;
        m_szBuildLog = szNewBuildLog;

        return CL_SUCCESS;
    }

    
    m_szBuildLog = new char[uiLogSize];
    if (!m_szBuildLog)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    STRCPY_S(m_szBuildLog, uiLogSize, szBuildLog);
    m_uiBuildLogSize = uiLogSize;

    return CL_SUCCESS;
}

cl_err_code DeviceProgram::SetBuildOptionsInternal(const char *szBuildOptions)
{
    if (m_szBuildOptions)
	{
		delete[] m_szBuildOptions;
		m_szBuildOptions = NULL;
	}

	if (szBuildOptions)
	{
		size_t uiOptionLength = strlen(szBuildOptions) + 1;
		m_szBuildOptions = new char[uiOptionLength];
		if (!m_szBuildOptions)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}
		MEMCPY_S(m_szBuildOptions, uiOptionLength, szBuildOptions, uiOptionLength);
	}

	return CL_SUCCESS;
}

const char* DeviceProgram::GetBuildOptionsInternal()
{
    return m_szBuildOptions;
}

cl_err_code DeviceProgram::SetStateInternal(EDeviceProgramState state)
{
    //TODO: maybe add state machine
    m_state = state;

    return CL_SUCCESS;
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

cl_build_status DeviceProgram::GetBuildStatus() const 
{
	switch (m_state)
	{
	default:
	case DEVICE_PROGRAM_INVALID:
		return CL_BUILD_ERROR;

	case DEVICE_PROGRAM_SOURCE:
	case DEVICE_PROGRAM_LOADED_IR:
		return CL_BUILD_NONE;

	case DEVICE_PROGRAM_FE_COMPILING:
	case DEVICE_PROGRAM_FE_LINKING:
	case DEVICE_PROGRAM_BE_BUILDING:
		return CL_BUILD_IN_PROGRESS;

	case DEVICE_PROGRAM_COMPILED:
    case DEVICE_PROGRAM_LINKED:
    case DEVICE_PROGRAM_BUILD_DONE:
		return CL_BUILD_SUCCESS;
	}

	return CL_BUILD_ERROR;
}

cl_err_code DeviceProgram::GetBuildInfo(cl_program_build_info clParamName, size_t uiParamValueSize, void * pParamValue, size_t * puiParamValueSizeRet) const
{
	cl_err_code clErr = CL_SUCCESS;

	if ((NULL == pParamValue && NULL == puiParamValueSizeRet)	||
		(NULL == pParamValue && uiParamValueSize > 0))
	{
		return CL_INVALID_VALUE;
	}

	size_t uiParamSize = 0;
	void * pValue = NULL;
	cl_build_status clBuildStatus;
	char emptyString = '\0';

	switch (clParamName)
	{
	case CL_PROGRAM_BUILD_STATUS:
		uiParamSize = sizeof(cl_build_status);
		clBuildStatus = GetBuildStatus();
		pValue = &clBuildStatus;
		break;

	case CL_PROGRAM_BUILD_OPTIONS:
		if (NULL != m_szBuildOptions)
		{
			uiParamSize = strlen(m_szBuildOptions) + 1;
			pValue = m_szBuildOptions;
			break;
		}
		uiParamSize = 1;
		pValue		= &emptyString;
		break;

	case CL_PROGRAM_BUILD_LOG:
		switch (m_state)
		{
		default:
        case DEVICE_PROGRAM_INVALID:
		case DEVICE_PROGRAM_SOURCE:
		case DEVICE_PROGRAM_LOADED_IR:
		case DEVICE_PROGRAM_FE_COMPILING:
        case DEVICE_PROGRAM_FE_LINKING:
        case DEVICE_PROGRAM_BE_BUILDING:
			uiParamSize = 1;
			pValue		= &emptyString;
			break;

		case DEVICE_PROGRAM_COMPILED:
		case DEVICE_PROGRAM_LINKED:
        case DEVICE_PROGRAM_COMPILE_FAILED:
        case DEVICE_PROGRAM_LINK_FAILED:
	        uiParamSize = m_uiBuildLogSize;
			pValue      = m_szBuildLog;
			break;

		case DEVICE_PROGRAM_BUILD_DONE:
        case DEVICE_PROGRAM_BUILD_FAILED:
			// still need to append the FE build log
			// First of all calculate the size
			clErr = m_pDevice->GetDeviceAgent()->clDevGetBuildLog(m_programHandle, 0, NULL, &uiParamSize);
			if CL_DEV_FAILED(clErr)
			{
				return CL_INVALID_VALUE;
			}
			if ( NULL != m_szBuildLog )
			{
				uiParamSize += m_uiBuildLogSize;
			}
			if (NULL != pParamValue && uiParamSize > uiParamValueSize)
			{
				return CL_INVALID_VALUE;
			}

			// if pParamValue == NULL return param value size
			if (NULL != puiParamValueSizeRet)
			{
				*puiParamValueSizeRet = uiParamSize;
			}

			// get the actual log
			if (NULL != pParamValue)
			{
				if ( NULL != m_szBuildLog )
				{
					//Copy the FE log minus the terminating NULL
					MEMCPY_S(pParamValue, uiParamValueSize, m_szBuildLog, m_uiBuildLogSize - 1);
					// and let the device write the rest of the log
					uiParamSize -= m_uiBuildLogSize;

                    clErr = m_pDevice->GetDeviceAgent()->clDevGetBuildLog(m_programHandle, uiParamSize, ((char*)pParamValue) + m_uiBuildLogSize - 1, NULL);
				}
                else
                {
                    clErr = m_pDevice->GetDeviceAgent()->clDevGetBuildLog(m_programHandle, uiParamSize, (char*)pParamValue, NULL);
                }	
			}
			return clErr;
			break;
		}

		break;

	default:
		return CL_INVALID_VALUE;
	}

	if (NULL != pParamValue && uiParamSize > uiParamValueSize)
	{
		return CL_INVALID_VALUE;
	}

	// if pParamValue == NULL return only param value size
	if (NULL != puiParamValueSizeRet)
	{
		*puiParamValueSizeRet = uiParamSize;
	}

	if (NULL != pParamValue && uiParamSize > 0)
	{
		MEMCPY_S(pParamValue, uiParamValueSize, pValue, uiParamSize);
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
	case DEVICE_PROGRAM_BUILD_DONE:
		//Return the resultant compiled binaries
		return m_pDevice->GetDeviceAgent()->clDevGetProgramBinary(m_programHandle, uiBinSize, pBin, puiBinSizeRet);

    case DEVICE_PROGRAM_COMPILED:
    case DEVICE_PROGRAM_LINKED:
	case DEVICE_PROGRAM_LOADED_IR:
		if ( NULL == pBin)
		{
			assert(m_uiBinaryBitsSize <= CL_MAX_UINT32);
			*puiBinSizeRet = (cl_uint)m_uiBinaryBitsSize;
			return CL_SUCCESS;
		}
		if (uiBinSize < m_uiBinaryBitsSize)
		{
			return CL_INVALID_VALUE;
		}
		MEMCPY_S(pBin, uiBinSize, m_pBinaryBits, m_uiBinaryBitsSize);
		return CL_SUCCESS;

	default:
		if ( NULL == pBin)	// When query for binary size and it's not available, we should return 0
		{
			*puiBinSizeRet = 0;
			return CL_SUCCESS;
		}
		// In every other case, we have nothing intelligent to return
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

cl_err_code DeviceProgram::SetDeviceHandleInternal(cl_dev_program programHandle)
{
    m_programHandle = programHandle;
    return CL_SUCCESS;
}
