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
//  kernel.cpp
//  Implementation of the Kernel class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "kernel.h"
#include "program_binary.h"
#include "program.h"
#include <cl_objects_map.h>
#include <device.h>
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::DeviceKernel(Kernel * pKernel, ProgramBinary * pProgBin, const char * psKernelName, cl_err_code * pErr)
{
	m_pLoggerClient = new LoggerClient(L"DeviceKernel", LL_DEBUG);
	InfoLog(m_pLoggerClient, L"DeviceKernel C'tor enter");

	if (NULL == pKernel || NULL == pProgBin || NULL == psKernelName || NULL == pProgBin->GetDevice())
	{
		ErrLog(m_pLoggerClient, L"NULL == pProgBin || NULL == pKernel || NULL == psKernelName || NULL == pProgBin->GetDevice()");
		if (NULL != pErr)
		{
			*pErr = CL_INVALID_VALUE;
		}
		return;
	}

	m_pKernel = pKernel;
	m_pDevice = (Device*)pProgBin->GetDevice();

	// update kernel prototype
	m_sKernelPrototype.m_psKernelName = new char[strlen(psKernelName) + 1];
	if (NULL == m_sKernelPrototype.m_psKernelName)
	{
		ErrLog(m_pLoggerClient, L"new char[%d] == NULL", strlen(psKernelName) + 1);
		if (NULL != pErr)
		{
			*pErr = CL_OUT_OF_HOST_MEMORY;
		}
		return;
	}
	strcpy_s(m_sKernelPrototype.m_psKernelName, strlen(psKernelName) + 1, psKernelName);

	// get kernel id
	cl_err_code clErrRet = m_pDevice->GetKernelId(pProgBin->GetId(), m_sKernelPrototype.m_psKernelName, &m_clDevKernel);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErr)
		{
			*pErr = clErrRet;
		}
		return;
	}

	// get kernel prototype
	size_t szArgsCount = 0;
	clErrRet = m_pDevice->GetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE, 0, NULL, &szArgsCount);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErr)
		{
			*pErr = clErrRet;
		}
		return;
	}
	m_sKernelPrototype.m_uiArgsCount = szArgsCount / sizeof(cl_kernel_arg_type);
	m_sKernelPrototype.m_pArgs = new cl_kernel_arg_type[m_sKernelPrototype.m_uiArgsCount];
	if (NULL == m_sKernelPrototype.m_pArgs)
	{
		if (NULL != pErr)
		{
			*pErr = CL_OUT_OF_HOST_MEMORY;
		}
		return;
	}
	clErrRet = m_pDevice->GetKernelInfo(m_clDevKernel, CL_DEV_KERNEL_PROTOTYPE, szArgsCount, m_sKernelPrototype.m_pArgs, NULL);
	if (CL_FAILED(clErrRet))
	{
		if (NULL != pErr)
		{
			*pErr = clErrRet;
		}
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
DeviceKernel::~DeviceKernel()
{
	InfoLog(m_pLoggerClient, L"DeviceKernel D'tor enter");
	delete[] m_sKernelPrototype.m_psKernelName;
	delete[] m_sKernelPrototype.m_pArgs;
	delete m_pLoggerClient;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// DeviceKernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
bool DeviceKernel::CheckKernelDefinition(DeviceKernel * pKernel)
{
	if (NULL == pKernel)
	{
		return false;
	}
	SKernelPrototype sKernelPrototype = pKernel->GetPrototype();
	if (strcmp(sKernelPrototype.m_psKernelName, m_sKernelPrototype.m_psKernelName) != 0)
	{
		return false;
	}
	if (sKernelPrototype.m_uiArgsCount != m_sKernelPrototype.m_uiArgsCount)
	{
		return false;
	}
	for (cl_uint ui=0; ui<m_sKernelPrototype.m_uiArgsCount; ++ui)
	{
		if (sKernelPrototype.m_pArgs[ui] != m_sKernelPrototype.m_pArgs[ui])
		{
			return false;
		}
	}
	// kernel prototypes are identical
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Kernel::Kernel(Program * pProgram, const char * psKernelName)
{
	m_pLoggerClient = new LoggerClient(L"Kernel", LL_DEBUG);
	InfoLog(m_pLoggerClient, L"Kernel C'tor enter");
	m_pProgram = pProgram;
	m_psKernelName = new char[strlen(psKernelName) + 1];
	if (NULL != m_psKernelName)
	{
		strcpy_s(m_psKernelName, strlen(psKernelName) + 1, psKernelName);
	}
	m_pDeviceKernels = new OCLObjectsMap();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Kernel::~Kernel()
{
	InfoLog(m_pLoggerClient, L"Kernel D'tor enter");
	delete m_psKernelName;
	delete m_pDeviceKernels;
	delete m_pLoggerClient;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	Kernel::GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret)
{
	return CL_ERR_NOT_IMPLEMENTED;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::Release()
{
	return OCLObject::Release();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel::CreateDeviceKernels
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Kernel::CreateDeviceKernels(cl_uint uiBinariesCount, ProgramBinary ** ppBinaries)
{
	InfoLog(m_pLoggerClient, L"AddDeviceKernel enter. uiBinariesCount=%d, ppBinaries=%d", uiBinariesCount, ppBinaries);
	if (NULL == ppBinaries)
	{
		return CL_INVALID_VALUE;
	}
	
	cl_err_code clErrRet = CL_SUCCESS;
	Device * pDevice = NULL;
	DeviceKernel * pDeviceKernel = NULL, * pPrevDeviceKernel = NULL;
	bool bResult = false;
	
	for(cl_uint ui=0; ui<uiBinariesCount; ++ui)
	{
		if (NULL == ppBinaries[ui] || NULL == ppBinaries[ui]->GetDevice())
		{
			ErrLog(m_pLoggerClient, L"NULL == ppBinaries[%d] || NULL == ppBinaries[%d]->GetDevice()", ui, ui);
			clErrRet = CL_INVALID_VALUE;
			break;
		}
		pDevice = (Device*)ppBinaries[ui]->GetDevice();
		if (m_pDeviceKernels->IsExists(pDevice->GetId()))
		{
			ErrLog(m_pLoggerClient, L"m_pDeviceKernels->IsExists(%d) = true", pDevice->GetId());
			clErrRet = CL_ERR_KEY_ALLREADY_EXISTS;
			break;
		}
		pDeviceKernel = new DeviceKernel(this, ppBinaries[ui], m_psKernelName, &clErrRet);
		if (CL_FAILED(clErrRet))
		{
			delete pDeviceKernel;
			break;
		}
		// check kernel definition - compare previous kernel to the next one
		if (NULL != pPrevDeviceKernel)
		{
			bResult = pDeviceKernel->CheckKernelDefinition(pPrevDeviceKernel);
			if (false == bResult)
			{
				delete pDeviceKernel;
				clErrRet = CL_INVALID_KERNEL_DEFINITION;
				break;
			}
		}
		pPrevDeviceKernel = pDeviceKernel;
		clErrRet = m_pDeviceKernels->AddObject((OCLObject*)pDeviceKernel, pDevice->GetId(), false);
		if (CL_FAILED(clErrRet))
		{
			delete pDeviceKernel;
			break;
		}
	}
	if (CL_FAILED(clErrRet))
	{
		// empty device kernels list
		for (cl_uint ui=0; ui<m_pDeviceKernels->Count(); ++ui)
		{
			clErrRet = m_pDeviceKernels->GetObjectByIndex(ui, (OCLObject**)&pDeviceKernel);
			if (CL_SUCCEEDED(clErrRet))
			{
				delete pDeviceKernel;
			}
		}
		m_pDeviceKernels->Clear();
	}
	return CL_SUCCESS;
}
