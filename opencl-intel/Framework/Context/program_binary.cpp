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
//  program_binary.cpp
//  Implementation of the ProgramBinary class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "program_binary.h"
#include <device.h>
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// ProgramBinary C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
ProgramBinary::ProgramBinary(cl_uint uiBinSize,
						const void * pBinData,
						cl_dev_binary_prop clDevBinaryProp,
						Device * pDevice,
						cl_err_code * pErr)
{
	m_pLoggerClient = new LoggerClient(L"program_binary", LL_DEBUG);
	InfoLog(m_pLoggerClient, L"ProgramBinary C'tor enter");

	if (NULL == pDevice)
	{
		ErrLog(m_pLoggerClient, L"pDevice is not valid device (pDevice==NULL)");
		if (NULL != pErr)
		{
			*pErr = CL_ERR_INITILIZATION_FAILED;
			return;
		}
	}
	m_pDevice = pDevice;
	m_clDevBinaryProp = clDevBinaryProp;
	m_clBuildStatus = CL_BUILD_NONE;

	// copy the binary data
	// TODO: maybe we don't need to do that
	m_uiBinSize = uiBinSize;
	m_pBinData = new char[m_uiBinSize];
	if (NULL == m_pBinData)
	{
		ErrLog(m_pLoggerClient, L"Can't allocate host memory for binary data");
		if (NULL != pErr)
		{
			*pErr = CL_OUT_OF_HOST_MEMORY;
			return;
		}
	}
	// copy the binary data
	memcpy_s(m_pBinData, m_uiBinSize, pBinData, m_uiBinSize);
	if (NULL != pErr)
	{
		*pErr = CL_SUCCESS;
	}

	// create device program
	m_clDevProgram = 0;
	*pErr = m_pDevice->CreateProgram((size_t)m_uiBinSize, m_pBinData, m_clDevBinaryProp, &m_clDevProgram);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// ProgramBinary D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
ProgramBinary::~ProgramBinary()
{
	InfoLog(m_pLoggerClient, L"ProgramBinary D'tor enter");
	delete m_pBinData;
	m_uiBinSize = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// GetStatus
///////////////////////////////////////////////////////////////////////////////////////////////////
const cl_build_status ProgramBinary::GetStatus()
{
	cl_build_status clBuildStatus = CL_BUILD_NONE;
	OclAutoMutex CS(&m_CS);
	{
		// Lock
		clBuildStatus = m_clBuildStatus;
		// Unlock
	}
	return clBuildStatus;
}
cl_err_code ProgramBinary::Build(const cl_char * pcOptions, IBuildDoneObserver * pBuildDoneObserver)
{
	m_pBuildDoneObserver = pBuildDoneObserver;

	// set build status
	OclAutoMutex CS(&m_CS);
	{
		// Lock
		m_clBuildStatus = CL_BUILD_IN_PROGRESS;
		// Unlock
	}

	IBuildDoneObserver * pMyBuildDoneObserver = dynamic_cast<IBuildDoneObserver*>(this);
	cl_int iRet = m_pDevice->BuildProgram(m_clDevProgram, pcOptions, pMyBuildDoneObserver);
	
	return (cl_err_code)iRet;
}
cl_err_code ProgramBinary::NotifyBuildDone(cl_device_id device, cl_build_status build_status)
{
	OclAutoMutex CS(&m_CS);
	{ // Lock
	
		// set new status
		m_clBuildStatus = build_status;
		// notify observer
		if (NULL != m_pBuildDoneObserver)
		{
			m_pBuildDoneObserver->NotifyBuildDone((cl_device_id)m_pDevice->GetId(), build_status);
		}
	
	} // Unlock
	return CL_SUCCESS;
}