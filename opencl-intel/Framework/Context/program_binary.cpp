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
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// ProgramBinary C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
ProgramBinary::ProgramBinary(cl_uint uiBinSize, const void * pBinData, Device * pDevice, cl_err_code * pErr)
{
	m_pLoggerClient = new LoggerClient(L"program_binary", LL_DEBUG);
	InfoLog(m_pLoggerClient, L"ProgramBinary C'tor enter");

	m_uiBinSize = uiBinSize;
	m_pBinData = new char[m_uiBinSize];
	if (NULL == m_pBinData)
	{
		if (NULL != pErr)
		{
			*pErr = CL_OUT_OF_HOST_MEMORY;
		}
	}
	memcpy_s(m_pBinData, m_uiBinSize, pBinData, m_uiBinSize);
	if (NULL != pErr)
	{
		*pErr = CL_SUCCESS;
	}

	m_pDevice = pDevice;

	//m_clDevProgContainer.description = ???
	//m_clDevProgContainer.container_type = ???
	m_clDevProgContainer.container_size = m_uiBinSize;
	m_clDevProgContainer.container = m_pBinData;
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
