/////////////////////////////////////////////////////////////////////////
// dll_program.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "cl_device_api.h"
#include "cl_dynamic_lib.h"
#include "cl_thread.h"
#include "ocl_rt.h"

#include "dll_program.h"
#include "cpu_kernel.h"

using namespace Intel::OpenCL;
using namespace Intel::OpenCL::CPUDevice;
using namespace Intel::OpenCL::Utils;

// Contructor/Destructor
DLLProgram::DLLProgram() :
	m_clBuildStatus(CL_BUILD_NONE), m_pContainer(NULL), m_stContainerSize(0),
	m_pBuildingThread(NULL)
{
}

DLLProgram::~DLLProgram()
{
	if ( NULL != m_pContainer )
	{
		free(m_pContainer);
	}

	if ( NULL != m_pBuildingThread )
	{
		delete m_pBuildingThread;
	}

	FreeMap();
	m_dllProgram.Close();
}

cl_int DLLProgram::CreateProgram(const cl_prog_container*	pContainer)
{
	const char* pLibName;

	// Container is provided by user
	if ( NULL == pContainer->container )
	{
		pLibName = (const char*)pContainer+sizeof(cl_prog_container);
	}
	else
	{
		pLibName = (const char*)pContainer->container;
	}
	// Extract DLL file name from container
	bool rc = m_dllProgram.Load(pLibName);
	if ( !rc )
	{
		return CL_DEV_INVALID_BINARY;
	}

	// Store container data
	m_stContainerSize = pContainer->container_size+sizeof(cl_prog_container);
	m_pContainer = (cl_prog_container*)malloc(m_stContainerSize);
	if ( NULL == m_pContainer )
	{
		m_stContainerSize = 0;
		m_dllProgram.Close();
		return CL_DEV_OUT_OF_MEMORY;
	}

	// Copy container header
	memcpy(m_pContainer, pContainer, sizeof(cl_prog_container));
	// Copy container content
	m_pContainer->container = ((char*)m_pContainer)+sizeof(cl_prog_container);
	memcpy((void*)m_pContainer->container, pLibName, pContainer->container_size);

	return CL_DEV_SUCCESS;
}

cl_int DLLProgram::BuildProgram(fn_clDevBuildStatusUpdate* pfnCallBack, cl_dev_program progId, void* pUserData)
{
	// Trying build the program again?
	if ( CL_BUILD_NONE != m_clBuildStatus )
	{
		return CL_DEV_INVALID_OPERATION;
	}

	if ( CL_BUILD_IN_PROGRESS == m_clBuildStatus )
	{
		return CL_DEV_STILL_BUILDING;
	}

	// Create building thread
	m_pBuildingThread = new DLLProgramThread(this, pfnCallBack, progId, pUserData);
	if ( NULL == m_pBuildingThread )
	{
		m_clBuildStatus = CL_BUILD_ERROR;
		return CL_DEV_OUT_OF_MEMORY;
	}

	m_clBuildStatus = CL_BUILD_IN_PROGRESS;
	int iRet = m_pBuildingThread->Start();
	if ( 0 != iRet )
	{
		m_clBuildStatus = CL_BUILD_ERROR;
		return CL_DEV_ERROR_FAIL;
	}

	return CL_DEV_SUCCESS;
}

// ------------------------------------------------------------------------------
// Quaries program build log
const char*	DLLProgram::GetBuildLog() const
{
	if ( CL_BUILD_SUCCESS == m_clBuildStatus )
	{
		return "OK";
	}

	return "";
}

// ------------------------------------------------------------------------------
// Copies internally stored container into provided buffer
cl_int DLLProgram::CopyContainer(void* pBuffer, size_t stSize) const
{
	if ( (NULL == pBuffer) || (stSize < m_stContainerSize) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	// Copy container header
	memcpy(pBuffer, m_pContainer, sizeof(cl_prog_container));
	// Copy container content
	memcpy(((char*)pBuffer)+sizeof(cl_prog_container), m_pContainer->container, m_pContainer->container_size);
	// container pointer for user must be NULL
	((cl_prog_container*)pBuffer)->container = NULL;

	return CL_DEV_SUCCESS;
}

// ------------------------------------------------------------------------------
// Retrieves a pointer to a function descriptor by function short name
cl_int DLLProgram::GetKernel(const char* pName, const ICLDevKernel* *pKernel) const
{
	if ( CL_BUILD_SUCCESS != m_clBuildStatus )
	{
		return CL_DEV_STILL_BUILDING;
	}

	TKernelMap::const_iterator	it;

	it = m_mapKernels.find(pName);
	if ( m_mapKernels.end() == it )
	{
		return CL_DEV_INVALID_KERNEL_NAME;
	}

	*pKernel = it->second;

	return CL_DEV_SUCCESS;
}

// ------------------------------------------------------------------------------
// Retrieves a vector of pointers to a function descriptors
cl_int DLLProgram::GetAllKernels(const ICLDevKernel* *pKernels, unsigned int uiCount, unsigned int *puiRetCount) const
{
	if ( CL_BUILD_SUCCESS != m_clBuildStatus )
	{
		return CL_DEV_STILL_BUILDING;
	}

	if ( NULL != puiRetCount )
	{
		*puiRetCount = m_mapKernels.size();
	}

	if ( (NULL == pKernels) &&  (0 == uiCount) )
	{
		if ( NULL == puiRetCount )
		{
			return CL_DEV_INVALID_VALUE;
		}

		return CL_DEV_SUCCESS;
	}

	if ( (NULL == pKernels) ||  (m_mapKernels.size() > uiCount) )
	{
		return CL_DEV_INVALID_VALUE;
	}

	TKernelMap::const_iterator it;
	unsigned int i = 0;
	for(it = m_mapKernels.begin(); it!=m_mapKernels.end();++it)
	{
		pKernels[i] = it->second;
		++i;
	}

	return CL_DEV_SUCCESS;
}

// ------------------------------------------------------------------------------
//	Parses libary information and retrieves/builds function descripotrs
cl_int DLLProgram::LoadProgram()
{
	cl_kernel_argument	vArgs[MAX_ARG_COUNT];
	unsigned int	uiArgCount;
	unsigned int	uiCount = m_dllProgram.GetNumberOfFunctions();

	for (unsigned int i = 0; i < uiCount; ++i)
	{
		uiArgCount = 0;

		// Allocate memory buffer for decriptors
		CPUKernel* pKernel = new CPUKernel;
		if ( NULL == pKernel )
		{
			m_clBuildStatus = CL_BUILD_ERROR;
			FreeMap();
			return CL_DEV_OUT_OF_MEMORY;
		}

		const char* szName = m_dllProgram.GetFunctionName(i);

		// Parse for the short name
		++szName;
		int index=0;
		while( '@' != szName[index] )
		{
			++index;
		}

		pKernel->SetName(szName, index);

		// Skip Short name
		szName+=index;
		// Skip "@@YAX"
		szName+=5;
		cl_kernel_argument tmpArg;

		// Parse rest of the full name
		while ( 'Z' != szName[0] )	// End of string
		{
			tmpArg.type = (cl_kernel_arg_type)-1;
			switch (szName[0])
			{
			// A Pointer
			case 'P' : case 'Q':
				tmpArg.type = ('P' == szName[0]) ? CL_KRNL_ARG_PTR_GLOBAL : CL_KRNL_ARG_PTR_CONST;
				tmpArg.size_in_bytes = sizeof(void*);

				// Skip pointer classifier
				szName+=2;
				// Is pointer to long type, skip to end of defention '@'
				switch (szName[0])
				{
				case 'V' : 
				case 'T' :
				case 'U' :
					while( '@' != szName[0] )
					{
						++szName;
					}
					++szName;
					break;

				case 'M':
				case 'I':
				case 'H':
					++szName;
					break;

				default:
					assert(0);
					break;
				}
				break;

			// A float
			case 'M' :
				tmpArg.type = CL_KRNL_ARG_FLOAT;
				tmpArg.size_in_bytes = sizeof(float);
				++szName;
				break;

			// integer types
			case 'H' : 
				tmpArg.type = CL_KRNL_ARG_UINT;
				tmpArg.size_in_bytes = sizeof(unsigned int);
				++szName;
				break;

			case 'I' :
				tmpArg.type = CL_KRNL_ARG_INT;
				tmpArg.size_in_bytes = sizeof(int);
				++szName;
				break;

			// Same as previous type
			case '0' :
				tmpArg = vArgs[uiArgCount-1];
				++szName;
				break;

			case 'X': case '@':
				++szName;
				break;

			default :
				assert(0);
			}

			if ( -1 != tmpArg.type )
			{
				vArgs[uiArgCount] = tmpArg;
				++uiArgCount;
			}
		}

		pKernel->SetArgumentList(vArgs, uiArgCount);
		pKernel->SetFuncPtr(m_dllProgram.GetFunctionPtr(i));

		// Store kernel to map
		m_mapKernels[pKernel->GetKernelName()] = pKernel;
	}

	m_clBuildStatus = CL_BUILD_SUCCESS;

	return CL_DEV_SUCCESS;
}

void DLLProgram::FreeMap()
{
	// Free descriptors map
	TKernelMap::iterator it;
	for(it = m_mapKernels.begin(); it != m_mapKernels.end(); ++it)
	{
		delete it->second;
	}

	m_mapKernels.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////
// DLLProgramThread implementation
DLLProgramThread::DLLProgramThread(DLLProgram* pProgram, fn_clDevBuildStatusUpdate* pfnCallBack,
	cl_dev_program progId, void* pUserData) : 
m_pProgram(pProgram), m_pfnCallBack(pfnCallBack), m_progId(progId), m_pUserData(pUserData)
{
}

int DLLProgramThread::Run()
{
	cl_int ret = m_pProgram->LoadProgram();

	cl_build_status status = CL_DEV_SUCCEEDED(ret) ? CL_BUILD_SUCCESS : CL_BUILD_ERROR;
	m_pfnCallBack(m_progId, m_pUserData, status);

	Clean();

	return ret;
}
