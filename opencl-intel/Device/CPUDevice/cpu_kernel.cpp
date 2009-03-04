/////////////////////////////////////////////////////////////////////////
// cpu_kernel.cpp:
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

#include "cpu_kernel.h"

#include <string.h>
#include <stdlib.h>

using namespace Intel::OpenCL::CPUDevice;

// Constructor/Destructor
CPUKernel::CPUKernel() :
	m_pFuncPtr(NULL), m_szName(NULL), m_uiArgCount(0), m_pArguments(NULL),
		m_stMDSize(0), m_pMetaData(NULL)
{
}

CPUKernel::~CPUKernel()
{
	if ( NULL != m_szName )
	{
		free(m_szName);
	}

	if ( NULL != m_pArguments )
	{
		free(m_pArguments);
	}

	if ( NULL != m_pMetaData )
	{
		free(m_pMetaData);
	}
}

// Returns a size of implicitly defined local memory buffers
size_t CPUKernel::GetImplicitLocalSize() const
{
	if ( NULL == m_pMetaData )
	{
		return 0;
	}

	size_t stLocSize = 0;
	unsigned int* pLocSizes = (unsigned int*)((char*)m_pMetaData+sizeof(CPUKernelMDHeader));
	for(unsigned int i=0; i<m_pMetaData->uiImplLocalMemCount; ++i)
	{
		stLocSize+=pLocSizes[i];
	}

	return stLocSize;
}

void CPUKernel::SetName(const char* szName, size_t stLen)
{
	if ( NULL != m_szName )
	{
		free(m_szName);
	}

	m_szName = (char*)malloc(stLen+1);
	if ( NULL != m_szName )
	{
		strncpy_s(m_szName, stLen+1, szName, stLen);
	}
}

void CPUKernel::SetArgumentList(const cl_kernel_argument* pArgs, unsigned int uiArgCount)
{
	if ( NULL != m_pArguments )
	{
		free(m_pArguments);
		m_uiArgCount = 0;
	}

	size_t stBuffLen = uiArgCount*sizeof(cl_kernel_argument);
	m_pArguments = (cl_kernel_argument*)malloc(stBuffLen);
	if ( NULL != m_pArguments )
	{
		m_uiArgCount = uiArgCount;
		memcpy(m_pArguments, pArgs, stBuffLen);
	}
}

void CPUKernel::SetMetaData(CPUKernelMDHeader* pMDHeader)
{
	if ( NULL != m_pMetaData )
	{
		free(m_pMetaData);
		m_stMDSize = 0;
	}

	m_pMetaData = (CPUKernelMDHeader*)malloc(pMDHeader->stMDsize);
	if ( NULL != m_pMetaData )
	{
		m_stMDSize = pMDHeader->stMDsize;
		memcpy(m_pMetaData, pMDHeader, m_stMDSize);
	}
}
