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
//  program.cpp
//  Implementation of the Class program
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "framework_proxy.h"
#include "program.h"
#include "device_program.h"
#include "Context.h"
#include "kernel.h"
#include "sampler.h"
#include "cl_shared_ptr.hpp"
#include <string.h>
#include <Device.h>
#include <fe_compiler.h>
#include <cl_utils.h>
#include <assert.h>

using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;


Program::Program(SharedPtr<Context> pContext) :
	OCLObject<_cl_program_int>((_cl_context_int*)pContext->GetHandle(), "Program"),
	m_pContext(pContext), m_ppDevicePrograms(NULL), m_szNumAssociatedDevices(0)
{
}

Program::~Program()
{
	assert (0 == m_pKernels.Count());
}

cl_err_code Program::GetBuildInfo(cl_device_id clDevice, cl_program_build_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	OclAutoMutex deviceProgMapMutex(&m_deviceProgramMapMutex);
	DeviceProgram* pDeviceProgram = NULL;
	tDeviceProgramMap::iterator deviceIdToProgramIter = m_deviceToProgram.find(clDevice);
	if ((m_deviceToProgram.end() == deviceIdToProgramIter) || (NULL == deviceIdToProgramIter->second))
	{
		pDeviceProgram = GetDeviceProgram(clDevice);
	}
	else
	{
		pDeviceProgram = deviceIdToProgramIter->second;
	}
	if (NULL == pDeviceProgram)
	{
		return CL_INVALID_DEVICE;
	}
    return pDeviceProgram->GetBuildInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret) const
{
	LOG_DEBUG(TEXT("Program::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d"), 
		param_name, param_value_size, param_value, param_value_size_ret);

	size_t szParamValueSize = 0;
	const void * pValue = NULL;

	cl_context clContextParam = 0;
	cl_device_id* clDevIds = NULL;
    size_t* puiNumKernels = NULL;
    char* szKernelsNames = NULL;

	switch ( (cl_program_info)param_name )
	{
	case CL_PROGRAM_REFERENCE_COUNT:
		szParamValueSize = sizeof(cl_uint);
		pValue = &m_uiRefCount;
		break;

	case CL_PROGRAM_CONTEXT:
		szParamValueSize = sizeof(cl_context);
		clContextParam = m_pContext->GetHandle();
		pValue = &clContextParam;
		break;

	case CL_PROGRAM_NUM_DEVICES:
			szParamValueSize = sizeof(m_szNumAssociatedDevices);
			pValue = &m_szNumAssociatedDevices;
			break;

	case CL_PROGRAM_DEVICES:
        {
		    szParamValueSize = sizeof(cl_device_id) * m_szNumAssociatedDevices;
		    clDevIds = new cl_device_id[m_szNumAssociatedDevices];
		    if (NULL == clDevIds)
		    {
			    return CL_OUT_OF_HOST_MEMORY;
		    }
		    for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
		    {
			    clDevIds[i] = m_ppDevicePrograms[i]->GetDeviceId();
		    }
		    pValue = clDevIds;
		    break;
        }

	case CL_PROGRAM_BINARY_SIZES:
		{
			OclAutoMutex deviceProgMapMutex(&m_deviceProgramMapMutex);
			szParamValueSize = sizeof(size_t) * m_szNumAssociatedDevices;
			if (NULL != param_value)
			{
				if (param_value_size < szParamValueSize)
				{
					return CL_INVALID_VALUE;
				}
				assert(param_value_size >= szParamValueSize);
				size_t * pParamValue = static_cast<size_t *>(param_value);
				tDeviceProgramMap::const_iterator deviceToProgramIter;
				tDeviceProgramMap::const_iterator deviceToProgramEnd = m_deviceToProgram.end();
				for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
				{
					DeviceProgram* pDeviceProgram = m_ppDevicePrograms[i];
					deviceToProgramIter = m_deviceToProgram.find(pDeviceProgram->GetDeviceId());
					if (deviceToProgramEnd != deviceToProgramIter)
					{
						pDeviceProgram = deviceToProgramIter->second;
					}
					assert(NULL != pDeviceProgram);
					cl_int clErrRet = pDeviceProgram->GetBinary(0, NULL, pParamValue + i);
					if (CL_FAILED(clErrRet))
					{
						return clErrRet;
					}
				}
			}
			if (NULL != param_value_size_ret)
			{
				*param_value_size_ret = szParamValueSize;
			}
			return CL_SUCCESS;
		}

	case CL_PROGRAM_BINARIES:
		{
			OclAutoMutex deviceProgMapMutex(&m_deviceProgramMapMutex);
			szParamValueSize = sizeof(char *) * m_szNumAssociatedDevices;
			char ** pParamValue = static_cast<char **>(param_value);
            size_t uiParam = 0;
			// get  data
			if (NULL != pParamValue)
			{
				if (param_value_size < szParamValueSize)
				{
					return CL_INVALID_VALUE;
				}
				tDeviceProgramMap::const_iterator deviceToProgramIter;
				tDeviceProgramMap::const_iterator deviceToProgramEnd = m_deviceToProgram.end();
				for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
				{
					DeviceProgram* pDeviceProgram = m_ppDevicePrograms[i];
					deviceToProgramIter = m_deviceToProgram.find(pDeviceProgram->GetDeviceId());
					if (deviceToProgramEnd != deviceToProgramIter)
					{
						pDeviceProgram = deviceToProgramIter->second;
					}
					assert(NULL != pDeviceProgram);
					cl_int clErrRet = pDeviceProgram->GetBinary(0, NULL, &uiParam);
					if (CL_FAILED(clErrRet))
					{
						return clErrRet;
					}
					clErrRet = pDeviceProgram->GetBinary(uiParam, pParamValue[i], &uiParam);
					if (CL_FAILED(clErrRet))
					{
						return clErrRet;
					}
				}
			}
			// get  size
			if (NULL != param_value_size_ret)
			{
				*param_value_size_ret = szParamValueSize;
			}
			return CL_SUCCESS;
		}

	case CL_PROGRAM_SOURCE:
		{
			szParamValueSize = 0;
            pValue = NULL;
		}

    case CL_PROGRAM_NUM_KERNELS:
        {
            bool found = false;

            for (unsigned int i = 0; i < m_szNumAssociatedDevices; ++i)
            {
                DeviceProgram* pDevProg = m_ppDevicePrograms[i];

                if (CL_BUILD_SUCCESS == pDevProg->GetBuildStatus())
		        {
                    found = true;

                    cl_uint uiNumKernels = 0;
                    cl_int clErrRet = pDevProg->GetNumKernels(&uiNumKernels);
					if (CL_FAILED(clErrRet))
					{
						return clErrRet;
					}

                    puiNumKernels = new size_t;
			        szParamValueSize = sizeof(size_t);
		            puiNumKernels[0] = (size_t)uiNumKernels;
                    pValue = puiNumKernels;
		            break;
		        }
            }

            if (!found)
            {
                // No successful build on any device
                return CL_INVALID_PROGRAM_EXECUTABLE;
            }

            break;
        }

    case CL_PROGRAM_KERNEL_NAMES:
        {
            bool found = false;

            for (unsigned int i = 0; i < m_szNumAssociatedDevices; ++i)
            {
                DeviceProgram* pDevProg = m_ppDevicePrograms[i];

                if (CL_BUILD_SUCCESS == pDevProg->GetBuildStatus())
		        {
                    found = true;
                    size_t total_length = 0;

                    // first get the number of kernels
                    cl_uint uiNumKernels = 0;
                    cl_int clErrRet = pDevProg->GetNumKernels(&uiNumKernels);
					if (CL_FAILED(clErrRet))
					{
						return clErrRet;
					}

                    if (0 == uiNumKernels)
                    {
                        // should't happen but try to get names from next device
                        found = false;
                        break;
                    }

                    // now get the length of each kernel name
                    size_t* puiKernelNameLengths = new size_t[uiNumKernels];
	                if (NULL == puiKernelNameLengths)
	                {
		                return CL_OUT_OF_HOST_MEMORY;
	                }
	                clErrRet = pDevProg->GetKernelNames(NULL, puiKernelNameLengths, uiNumKernels);
	                if (CL_FAILED(clErrRet))
	                {
		                delete[] puiKernelNameLengths;
		                return clErrRet;
	                }
	                char** pszKernelNames = new char*[uiNumKernels];
	                if (NULL == pszKernelNames)
	                {
		                delete[] puiKernelNameLengths;
		                return CL_OUT_OF_HOST_MEMORY;
	                }
	                for (size_t i = 0; i < uiNumKernels; ++i)
	                {
                        total_length += puiKernelNameLengths[i];
		                pszKernelNames[i] = new char[puiKernelNameLengths[i]];
		                if (NULL == pszKernelNames[i])
		                {
			                for (size_t j = 0; j < i; ++j)
			                {
				                delete[] pszKernelNames[j];
			                }
			                delete[] pszKernelNames;
			                delete[] puiKernelNameLengths;
			                return CL_OUT_OF_HOST_MEMORY;
		                }
	                }

                    // and finaly get the names
	                clErrRet = pDevProg->GetKernelNames(pszKernelNames, puiKernelNameLengths, uiNumKernels);
	                if (CL_FAILED(clErrRet))
	                {
		                for (size_t i = 0; i < uiNumKernels; ++i)
		                {
			                delete[] pszKernelNames[i];
		                }
		                delete[] pszKernelNames;
		                delete[] puiKernelNameLengths;
		                return clErrRet;
	                }

                    // once we have the actual names, we need to concatenate them
                    assert(total_length > 0);
                    szKernelsNames = new char[total_length];
                    if (NULL == szKernelsNames)
                    {
                        for (size_t i = 0; i < uiNumKernels; ++i)
	                    {
		                    delete[] pszKernelNames[i];
	                    }
	                    delete[] pszKernelNames;
	                    delete[] puiKernelNameLengths;
                        return CL_OUT_OF_HOST_MEMORY;
                    }

                    // There is at least one kernel
                    STRCPY_S(szKernelsNames, total_length, pszKernelNames[0]);
                    
                    for (size_t i = 1; i < uiNumKernels; ++i)
                    {
                        STRCAT_S(szKernelsNames, total_length, ";");
                        STRCAT_S(szKernelsNames, total_length, pszKernelNames[i]);  
                    }

			        szParamValueSize = sizeof(char) * total_length;
                    pValue = szKernelsNames;
		            break;
		        }
            }

            if (!found)
            {
                // No successful build on any device
                return CL_INVALID_PROGRAM_EXECUTABLE;
            }

            break;
        }
		
	default:
		LOG_ERROR(TEXT("param_name (=%d) isn't valid"), param_name);
		return CL_INVALID_VALUE;
	}

	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (NULL != param_value && param_value_size < szParamValueSize)
	{
		LOG_ERROR(TEXT("param_value_size (=%d) < szParamValueSize (=%d)"), param_value_size, szParamValueSize);
		if (clDevIds)
		{
			delete[] clDevIds;
		}
        if (puiNumKernels)
        {
            delete puiNumKernels;
        }
        if (szKernelsNames)
        {
            delete[] szKernelsNames;
        }
		return CL_INVALID_VALUE;
	}

	// if param_value == NULL return only param value size
	if (NULL != param_value_size_ret)
	{
		*param_value_size_ret = szParamValueSize;
	}

	if (NULL != param_value && szParamValueSize > 0)
	{
		MEMCPY_S(param_value, szParamValueSize, pValue, szParamValueSize);
	}
	if (clDevIds)
	{
		delete[] clDevIds;
	}
    if (puiNumKernels)
    {
        delete puiNumKernels;
    }
    if (szKernelsNames)
    {
        delete[] szKernelsNames;
    }

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::GetBinaryInternal
///////////////////////////////////////////////////////////////////////////////////////////////////
const char* Program::GetBinaryInternal(cl_device_id clDevice)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return NULL;
	}
    return pDeviceProgram->GetBinaryInternal();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::GetBinarySizeInternal
///////////////////////////////////////////////////////////////////////////////////////////////////
size_t Program::GetBinarySizeInternal(cl_device_id clDevice)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return 0;
	}
    return pDeviceProgram->GetBinarySizeInternal();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::SetBinaryInternal
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::SetBinaryInternal(cl_device_id clDevice, size_t uiBinarySize, const void *pBinary)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return CL_INVALID_DEVICE;
	}
    return pDeviceProgram->SetBinaryInternal(uiBinarySize, pBinary);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::ClearBuildLogInternal
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::ClearBuildLogInternal(cl_device_id clDevice)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return CL_INVALID_DEVICE;
	}
    return pDeviceProgram->ClearBuildLogInternal();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::SetBuildLogInternal
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::SetBuildLogInternal(cl_device_id clDevice, const char *szBuildLog)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return CL_INVALID_DEVICE;
	}
    return pDeviceProgram->SetBuildLogInternal(szBuildLog);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::SetBuildOptionsInternal
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::SetBuildOptionsInternal(cl_device_id clDevice, const char* szBuildOptions)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return CL_INVALID_DEVICE;
	}
    return pDeviceProgram->SetBuildOptionsInternal(szBuildOptions);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::GetBuildOptionsInternal
///////////////////////////////////////////////////////////////////////////////////////////////////
const char* Program::GetBuildOptionsInternal(cl_device_id clDevice)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return NULL;
	}
    return pDeviceProgram->GetBuildOptionsInternal();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::SetStateInternal
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::SetStateInternal(cl_device_id clDevice, EDeviceProgramState state)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return CL_INVALID_DEVICE;
	}
    return pDeviceProgram->SetStateInternal(state);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::GetStateInternal
///////////////////////////////////////////////////////////////////////////////////////////////////
EDeviceProgramState Program::GetStateInternal(cl_device_id clDevice)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return DEVICE_PROGRAM_INVALID;
	}
    return pDeviceProgram->GetStateInternal();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Program::GetStateInternal
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::SetDeviceHandleInternal(cl_device_id clDevice, cl_dev_program programHandle)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return CL_INVALID_DEVICE;
	}
    return pDeviceProgram->SetDeviceHandleInternal(programHandle);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CreateKernel
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::CreateKernel(const char * psKernelName, SharedPtr<Kernel>* ppKernel)
{
	//cl_start;

	LOG_DEBUG(TEXT("CreateKernel enter. pscKernelName=%s, ppKernel=%d"), psKernelName, ppKernel);

	// check invalid input
	if (NULL == psKernelName)
	{
		return CL_INVALID_VALUE;
	}

	// check if there's a valid program binary already built for any device
	bool bAnyValid = false;
	for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
	{
		if ((CL_BUILD_SUCCESS == m_ppDevicePrograms[i]->GetBuildStatus()) ||
			(DEVICE_PROGRAM_BUILTIN_KERNELS==m_ppDevicePrograms[i]->GetStateInternal()))
		{
			bAnyValid = true;
			break;
		}
	}
	if (!bAnyValid)
	{
		return CL_INVALID_PROGRAM_EXECUTABLE;
	}

	// create new kernel object
    SharedPtr<Kernel> pKernel = Kernel::Allocate(this, psKernelName, m_szNumAssociatedDevices);
	pKernel->SetLoggerClient(GET_LOGGER_CLIENT);

	// next step - for each device that has the kernel, create device kernel 
	cl_err_code clErrRet = pKernel->CreateDeviceKernels(m_ppDevicePrograms);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(TEXT("pKernel->CreateDeviceKernels(ppBinaries) = %s"), ClErrTxt(clErrRet));
		pKernel->Release();
		return clErrRet;
	}

	// add the kernel object and adding new key for it
	//m_pKernels->AddObject((SharedPtr<OCLObject<_cl_kernel_int>>)pKernel);
	m_pKernels.AddObject(pKernel);
	if (NULL != ppKernel)
	{
		*ppKernel = pKernel;
	}

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CreateAllKernels
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::CreateAllKernels(cl_uint uiNumKernels, cl_kernel * pclKernels, cl_uint * puiNumKernelsRet)
{
	LOG_DEBUG(TEXT("Enter CreateAllKernels (uiNumKernels=%d, pclKernels=%d, puiNumKernelsRet=%d"), 
		uiNumKernels, pclKernels, puiNumKernelsRet);

	for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
	{
		if (CL_BUILD_SUCCESS != m_ppDevicePrograms[i]->GetBuildStatus())
		{
			return CL_INVALID_PROGRAM_EXECUTABLE;
		}
	}

	cl_err_code clErrRet = CL_SUCCESS;

	//Todo: need to discuss this
	//      Probably the best solution is just clDevGetProgramKernelNames
	//      Also need to see if it's possible for two devices to have different kernels

	assert(m_szNumAssociatedDevices > 0);

	cl_uint szNumKernels = 0;
	clErrRet = m_ppDevicePrograms[0]->GetNumKernels(&szNumKernels);
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	if (NULL != puiNumKernelsRet)
	{
		assert(szNumKernels <= CL_MAX_UINT32);
		*puiNumKernelsRet = (cl_uint)szNumKernels;
	}
    if (0 == szNumKernels)
    {
        // If we don't have any kernels return
        return CL_SUCCESS;
    }
	//No point in creating user-invisible kernels
	if (NULL == pclKernels)
	{
		return CL_SUCCESS;
	}
    if (uiNumKernels < szNumKernels)
    {
        return CL_INVALID_VALUE;
    }

	size_t* pszKernelNameLengths = new size_t[szNumKernels];
	if (NULL == pszKernelNameLengths)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	clErrRet = m_ppDevicePrograms[0]->GetKernelNames(NULL, pszKernelNameLengths, szNumKernels);
	if (CL_FAILED(clErrRet))
	{
		delete[] pszKernelNameLengths;
		return clErrRet;
	}
	char** ppKernelNames = new char*[szNumKernels];
	if (NULL==ppKernelNames)
	{
		delete[] pszKernelNameLengths;
		return CL_OUT_OF_HOST_MEMORY;
	}
	for (size_t i = 0; i < szNumKernels; ++i)
	{
		ppKernelNames[i] = new char[pszKernelNameLengths[i]];
		if (NULL == ppKernelNames[i])
		{
			for (size_t j = 0; j < i; ++j)
			{
				delete[] ppKernelNames[j];
			}
			delete[] ppKernelNames;
			delete[] pszKernelNameLengths;
			return CL_OUT_OF_HOST_MEMORY;
		}
	}

	clErrRet = m_ppDevicePrograms[0]->GetKernelNames(ppKernelNames, pszKernelNameLengths, szNumKernels);
	if (CL_FAILED(clErrRet))
	{
		for (size_t i = 0; i < szNumKernels; ++i)
		{
			delete[] ppKernelNames[i];
		}
		delete[] ppKernelNames;
		delete[] pszKernelNameLengths;
		return clErrRet;
	}

	for (size_t i = 0; i < szNumKernels; ++i)
	{
        SharedPtr<Kernel> pKernel;
		clErrRet = CreateKernel(ppKernelNames[i], &pKernel);
        
		if (CL_FAILED(clErrRet) || NULL == pKernel)
		{
			for (size_t k = 0; k < szNumKernels; ++k)
			{
				delete[] ppKernelNames[k];
			}
			for (size_t j = 0; j < i; ++j)
			{
		        m_pKernels.ReleaseObject((_cl_kernel_int*)pclKernels[j]);
                pclKernels[j] = NULL;
			}
			delete[] ppKernelNames;
			delete[] pszKernelNameLengths;
            if (NULL != pKernel)
            {
                m_pKernels.ReleaseObject((_cl_kernel_int*)pKernel->GetHandle());
            }
			return clErrRet;
		}
        pclKernels[i] = pKernel->GetHandle();
	}
	for (size_t i = 0; i < szNumKernels; ++i)
	{
		delete[] ppKernelNames[i];
	}
	delete[] ppKernelNames;
	delete[] pszKernelNameLengths;

	return CL_SUCCESS;
}


cl_err_code Program::RemoveKernel(cl_kernel clKernel)
{
	return m_pKernels.RemoveObject((_cl_kernel_int*)clKernel);
}

cl_err_code Program::GetKernels(cl_uint uiNumKernels, SharedPtr<Kernel>* ppKernels, cl_uint * puiNumKernelsRet)
{
	LOG_DEBUG(TEXT("Enter GetKernels (uiNumKernels=%d, ppKernels=%d, puiNumKernelsRet=%d"), 
		uiNumKernels, ppKernels, puiNumKernelsRet);

    if (NULL == ppKernels)
    {
        return m_pKernels.GetObjects(uiNumKernels, NULL, puiNumKernelsRet);
    }
    else
    {
        assert(uiNumKernels > 0);
        
        std::vector<SharedPtr<OCLObject<_cl_kernel_int> > > kernels(uiNumKernels);
        const cl_err_code err = m_pKernels.GetObjects(uiNumKernels, &kernels[0], puiNumKernelsRet);
        if (CL_FAILED(err))
        {
            return err;
        }
        for (size_t i = 0; i < uiNumKernels; i++)
        {
            ppKernels[i] = kernels[i].DynamicCast<Kernel>();
        }
        return CL_SUCCESS;
    }
}

DeviceProgram* Program::GetDeviceProgram(cl_device_id clDeviceId)
{
    return InternalGetDeviceProgram(clDeviceId);
}

DeviceProgram* Program::InternalGetDeviceProgram(cl_device_id clDeviceId)
{
    for (size_t deviceProg = 0; deviceProg < m_szNumAssociatedDevices; ++deviceProg)
    {
        DeviceProgram* pDeviceProgram = m_ppDevicePrograms[deviceProg];
        if (clDeviceId == pDeviceProgram->GetDeviceId())
        {
            return m_ppDevicePrograms[deviceProg];
        }
    }
    return NULL;
}

cl_uint Program::GetNumDevices()
{
    return m_szNumAssociatedDevices;
}

cl_err_code Program::GetDevices(cl_device_id* pDeviceID)
{
    for (unsigned int i = 0; i < m_szNumAssociatedDevices; ++i)
    {
        DeviceProgram* pDeviceProgram = m_ppDevicePrograms[i];
        pDeviceID[i] = pDeviceProgram->GetDeviceId();
    }

    return CL_SUCCESS;
}

cl_uint Program::GetNumKernels()
{
    return m_pKernels.Count();
}

bool Program::Acquire(cl_device_id clDevice)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return false;
	}

    return pDeviceProgram->Acquire();
}

void Program::Unacquire(cl_device_id clDevice)
{
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(clDevice);
    if (NULL == pDeviceProgram)
	{
		return;
	}

    return pDeviceProgram->Unacquire();
}

void Program::SetContextDevicesToProgramMappingInternal()
{
	OclAutoMutex deviceProgMapMutex(&m_deviceProgramMapMutex);
	// Clear the previous mapping
	m_deviceToProgram.clear();
	unsigned int numDevicesInContext = 0;
	tDeviceProgramMap realBuiltDeviceToDeviceProgram;
	// Collect all the devices that really execute build.
	for (unsigned int i = 0; i < m_szNumAssociatedDevices; i++)
	{
		if (CL_BUILD_NONE != m_ppDevicePrograms[i]->GetBuildStatus())
		{
			realBuiltDeviceToDeviceProgram.insert( pair<cl_device_id, DeviceProgram*>(m_ppDevicePrograms[i]->GetDeviceId(), m_ppDevicePrograms[i]) );
		}
	}
	SharedPtr<FissionableDevice>* ppContextDevices = m_pContext->GetDevices(&numDevicesInContext);
	assert(ppContextDevices);
	// Collect all context devices in as set
	set<SharedPtr<FissionableDevice> > contextDevicesSet;
	for (unsigned int i = 0; i < numDevicesInContext; i++)
	{
		contextDevicesSet.insert(ppContextDevices[i]);
	}
	set<SharedPtr<FissionableDevice> >::const_iterator contextDevicesEnd = contextDevicesSet.end();
	tDeviceProgramMap::iterator realBuiltDeviceIter;
	tDeviceProgramMap::const_iterator realBuiltDeviceEnd = realBuiltDeviceToDeviceProgram.end();
	// Traverse over all the devices in the context
	for (unsigned int i = 0; i < numDevicesInContext; i++)
	{
		SharedPtr<FissionableDevice> pCurrentLevelDevice = ppContextDevices[i];
		while (true)
		{
			realBuiltDeviceIter = realBuiltDeviceToDeviceProgram.find(pCurrentLevelDevice->GetHandle());
			// Is this device have built program? and this device in the same context
			if ((realBuiltDeviceEnd != realBuiltDeviceIter) && (contextDevicesEnd != contextDevicesSet.find(pCurrentLevelDevice)))
			{
				m_deviceToProgram.insert( pair<cl_device_id, DeviceProgram*>(ppContextDevices[i]->GetHandle(), realBuiltDeviceIter->second) );
				break;
			}
			// If this device is root device than ppContextDevices[i] does not have parent with built program
			if (pCurrentLevelDevice->IsRootLevelDevice())
			{
				break;
			}
            pCurrentLevelDevice = pCurrentLevelDevice.DynamicCast<SubDevice>()->GetParentDevice();
		}
	}
}

bool Program::GetMyRelatedProgramDeviceIDInternal(const cl_device_id devID, cl_int* pOutID)
{
	OclAutoMutex deviceProgMapMutex(&m_deviceProgramMapMutex);
	// Find the DeviceProgram that related to me (myne or my parent recursively (if in the same context))
	tDeviceProgramMap::iterator deviceProgramIter = m_deviceToProgram.find(devID);
	if (m_deviceToProgram.end() == deviceProgramIter)
	{
		return false;
	}
	// Set the Object ID of the device that built this Program and have DeviceProgram
	*pOutID = deviceProgramIter->second->GetDevice()->GetId();
	return true;
}
