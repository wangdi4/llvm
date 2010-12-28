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

#include <cassert>
#include "framework_proxy.h"
#include "program.h"
#include "device_program.h"
#include "context.h"
#include "kernel.h"

#include <string.h>
#include <device.h>
#include <fe_compiler.h>
#include <cl_utils.h>

using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;


Program::Program(Context * pContext, ocl_entry_points * pOclEntryPoints) : m_pContext(pContext), m_pDevicePrograms(NULL), m_szNumAssociatedDevices(0)
{
	m_handle.object   = this;
	m_handle.dispatch = pOclEntryPoints;

	m_pContext->AddPendency();
}

Program::~Program()
{
	assert (0 == m_pKernels.Count());
	m_pContext->RemovePendency();
}

cl_err_code Program::GetBuildInfo(cl_device_id clDevice, cl_program_build_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	DeviceProgram* pDeviceProgram = GetDeviceProgram(clDevice);
	if (!pDeviceProgram)
	{
		return CL_INVALID_DEVICE;
	}
	return pDeviceProgram->GetBuildInfo(clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);
}

cl_err_code Program::Build(cl_uint	    uiNumDevices,
				  const cl_device_id *	pclDeviceList,
				  const char *			pcOptions,
				  pfnNotifyBuildDone    pfn,
				  void *				pUserData)
{
	cl_err_code ret = CL_SUCCESS;

	if (0 == m_szNumAssociatedDevices)
	{
		return CL_INVALID_DEVICE;
	}

	if (m_pKernels.Count() > 0)
	{
		return CL_INVALID_OPERATION;
	}

	size_t* indices = new size_t[m_szNumAssociatedDevices];

	if (uiNumDevices > 0)
	{
		//Phase one: get indices of the appropriate deviceProgram objects, and attempt to acquire all of them

		for (cl_uint i = 0; i < uiNumDevices; ++i)
		{
			cl_device_id curDeviceId = pclDeviceList[i];
			for (size_t deviceProg = 0; deviceProg < m_szNumAssociatedDevices; ++deviceProg)
			{
				bool bFound = false;
				DeviceProgram& pDeviceProgram = m_pDevicePrograms[deviceProg];
				if (curDeviceId == pDeviceProgram.GetDeviceId())
				{
					bFound = true;
					if (!pDeviceProgram.Acquire()) //Oops, someone else is trying to build this program for that device
					{
						//Release all accesses already acquired
						for (cl_uint j = i; j > 0; --j)
						{
							m_pDevicePrograms[indices[j-1]].Unacquire();
						}

						delete[] indices;
						return CL_INVALID_OPERATION;
					}
					else
					{
						indices[i] = deviceProg;
						break;
					}
				}
				if (!bFound) //We've been given a device not in the device list associated with program
				{
					//Release all accesses already acquired
					for (cl_uint j = i; j > 0; --j)
					{
						m_pDevicePrograms[indices[j-1]].Unacquire();
					}
					delete[] indices;
					return CL_INVALID_DEVICE;
				}
			}
		}
	}
	else //build for all devices
	{
		//Phase one: acquire DeviceProgram objects on all associated devices
		for (size_t deviceProg = 0; deviceProg < m_szNumAssociatedDevices; ++deviceProg)
		{
			DeviceProgram& pDeviceProgram = m_pDevicePrograms[deviceProg];
			if (!pDeviceProgram.Acquire()) //Oops, someone else is trying to build this program for that device
			{
				//Release all accesses already acquired
				for (size_t j = deviceProg; j > 0; --j)
				{
					m_pDevicePrograms[j-1].Unacquire();
				}

				delete[] indices;
				return CL_INVALID_OPERATION;
			}
			else
			{
				indices[deviceProg] = deviceProg;
			}

		}
	}

	//Todo: is it guaranteed to with simultaneous calls to clBuildProgram for a device, exactly one succeeds and all else return CL_INVALID_OPERATION?

	size_t numDevicesToBuildFor = m_szNumAssociatedDevices;
	if (uiNumDevices > 0)
	{
		numDevicesToBuildFor = uiNumDevices;
	}
	//At this point, indices 0 to numDevicesToBuildFor in the indices array are indices in m_pDevicePrograms that represent programs 
	//that need to be built and that we are guaranteed exclusive access to

	//Phase 2: build them
	for (size_t i = 0; i < numDevicesToBuildFor; ++i)
	{
		ret = m_pDevicePrograms[indices[i]].Build(pcOptions, pfn, pUserData);
		if (!CL_SUCCEEDED(ret))
		{
			break;
		}
	}

	//Release access on all programs. Further accesses while they're building should fail in the call to Build
	for (size_t i = 0; i < numDevicesToBuildFor; ++i)
	{
		m_pDevicePrograms[indices[i]].Unacquire();
	}

	delete[] indices;
	return ret;
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
	cl_device_id* clDevIds = NULL;
	cl_uint uiParam = 0;

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
		szParamValueSize = sizeof(cl_device_id) * m_szNumAssociatedDevices;
		clDevIds = new cl_device_id[m_szNumAssociatedDevices];
		if (!clDevIds)
		{
			return CL_OUT_OF_HOST_MEMORY;
		}
		for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
		{
			clDevIds[i] = m_pDevicePrograms[i].GetDeviceId();
		}
		pValue = clDevIds;
		break;

	case CL_PROGRAM_BINARIES:
	case CL_PROGRAM_BINARY_SIZES:
	case CL_PROGRAM_SOURCE:
		//All should have been handled in implementing classes
		assert(0);
		//Intentional fall through into default
		
	default:
		LOG_ERROR(L"param_name (=%d) isn't valid", param_name);
		return CL_INVALID_VALUE;
	}

	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (NULL != param_value && param_value_size < szParamValueSize)
	{
		LOG_ERROR(L"param_value_size (=%d) < szParamValueSize (=%d)", param_value_size, szParamValueSize);
		if (clDevIds)
		{
			delete[] clDevIds;
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
		memcpy_s(param_value, szParamValueSize, pValue, szParamValueSize);
	}
	if (clDevIds)
	{
		delete[] clDevIds;
	}

	return CL_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CreateKernel
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Program::CreateKernel(const char * psKernelName, Kernel ** ppKernel)
{
	//cl_start;

	LOG_DEBUG(L"CreateKernel enter. pscKernelName=%s, ppKernel=%d", psKernelName, ppKernel);

	// check invalid input
	if (NULL == psKernelName)
	{
		return CL_INVALID_VALUE;
	}

	// check if program binary already built for all devices
	for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
	{
		if (CL_BUILD_SUCCESS != m_pDevicePrograms[i].GetBuildStatus())
		{
			return CL_INVALID_PROGRAM_EXECUTABLE;
		}
	}


	// check if the current kernel already available in the program
	//bool bResult = IsKernelExists(psKernelName, ppKernel);
	//if (true == bResult)
	//{
	//	return CL_SUCCESS;
	//}

	// create new kernel object
	Kernel * pKernel = new Kernel(this, psKernelName, m_szNumAssociatedDevices, (ocl_entry_points*)m_handle.dispatch);
	pKernel->SetLoggerClient(GET_LOGGER_CLIENT);

	// next step - for each device that has the kernel, create device kernel 
	cl_err_code clErrRet = pKernel->CreateDeviceKernels(m_pDevicePrograms);
	if (CL_FAILED(clErrRet))
	{
		LOG_ERROR(L"pKernel->CreateDeviceKernels(ppBinaries) = %ws", ClErrTxt(clErrRet));
		pKernel->Release();
		return clErrRet;
	}

	// add the kernel object and adding new key for it
	//m_pKernels->AddObject((OCLObject<_cl_kernel_int>*)pKernel);
	m_pKernels.AddObject(pKernel);
	if (ppKernel)
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
	LOG_DEBUG(L"Enter CreateAllKernels (uiNumKernels=%d, pclKernels=%d, puiNumKernelsRet=%d", 
		uiNumKernels, pclKernels, puiNumKernelsRet);

	for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
	{
		if (CL_BUILD_SUCCESS != m_pDevicePrograms[i].GetBuildStatus())
		{
			return CL_INVALID_PROGRAM_EXECUTABLE;
		}
	}

	cl_err_code clErrRet = CL_SUCCESS;

	//Todo: need to discuss this
	//      Probably the best solution is just clDevGetProgramKernelNames
	//      Also need to see if it's possible for two devices to have different kernels

	assert(m_szNumAssociatedDevices > 0);

	size_t szNumKernels = 0;
	clErrRet = m_pDevicePrograms[0].GetNumKernels(&szNumKernels);
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}
	if (0 == szNumKernels)
	{
		return CL_INVALID_PROGRAM_EXECUTABLE;
	}
	if (NULL != puiNumKernelsRet)
	{
		assert(szNumKernels <= MAXUINT32);
		*puiNumKernelsRet = (cl_uint)szNumKernels;
	}
	//No point in creating user-invisible kernels
	if (NULL == pclKernels)
	{
		return CL_SUCCESS;
	}

	size_t* pszKernelNameLengths = new size_t[szNumKernels];
	if (!pszKernelNameLengths)
	{
		return CL_OUT_OF_HOST_MEMORY;
	}
	clErrRet = m_pDevicePrograms[0].GetKernelNames(NULL, pszKernelNameLengths, szNumKernels);
	if (CL_FAILED(clErrRet))
	{
		delete[] pszKernelNameLengths;
		return clErrRet;
	}
	char** ppKernelNames = new char*[szNumKernels];
	if (!ppKernelNames)
	{
		delete[] pszKernelNameLengths;
		return CL_OUT_OF_HOST_MEMORY;
	}
	for (size_t i = 0; i < szNumKernels; ++i)
	{
		ppKernelNames[i] = new char[pszKernelNameLengths[i]];
		if (!ppKernelNames[i])
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

	clErrRet = m_pDevicePrograms[0].GetKernelNames(ppKernelNames, pszKernelNameLengths, szNumKernels);
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
		clErrRet = CreateKernel(ppKernelNames[i], NULL);
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
	}
	for (size_t i = 0; i < szNumKernels; ++i)
	{
		delete[] ppKernelNames[i];
	}
	delete[] ppKernelNames;
	delete[] pszKernelNameLengths;

	if ( NULL != pclKernels )
	{
		// Check if passed buffer is big enough
		if (uiNumKernels >= m_pKernels.Count())
		{
			// get the kernels Ids
			clErrRet = m_pKernels.GetIDs(m_pKernels.Count(), (_cl_kernel_int**)pclKernels, puiNumKernelsRet);
		}
		else
		{
			clErrRet = CL_INVALID_VALUE;
		}

		if (CL_FAILED(clErrRet))
		{
			m_pKernels.ReleaseAllObjects();
			return clErrRet;
		}			
	}
	return CL_SUCCESS;
}


cl_err_code Program::RemoveKernel(cl_kernel clKernel)
{
	return m_pKernels.RemoveObject((_cl_kernel_int*)clKernel);
}

cl_err_code Program::GetKernels(cl_uint uiNumKernels, Kernel ** ppKernels, cl_uint * puiNumKernelsRet)
{
	LOG_DEBUG(L"Enter GetKernels (uiNumKernels=%d, ppKernels=%d, puiNumKernelsRet=%d", 
		uiNumKernels, ppKernels, puiNumKernelsRet);

	return m_pKernels.GetObjects(uiNumKernels, reinterpret_cast<OCLObject<_cl_kernel_int> **>(ppKernels), puiNumKernelsRet);
}




DeviceProgram* Program::GetDeviceProgram(cl_device_id clDeviceId)
{
	for (size_t deviceProg = 0; deviceProg < m_szNumAssociatedDevices; ++deviceProg)
	{
		DeviceProgram& pDeviceProgram = m_pDevicePrograms[deviceProg];
		if (clDeviceId == pDeviceProgram.GetDeviceId())
		{
			return m_pDevicePrograms + deviceProg;
		}
	}
	return NULL;
}





