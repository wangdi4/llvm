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
//  cl_buffer.cpp
//  Implementation of the Buffer Class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_buffer.h"
#include <assert.h>
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Buffer::Buffer(Context * pContext, cl_mem_flags clMemFlags, void * pHostPtr, size_t szBufferSize, cl_err_code * pErrCode):
		MemoryObject(pContext, clMemFlags, pHostPtr)
{
#ifdef _DEBUG
	assert ( NULL != pErrCode );
#endif
	m_pLoggerClient = new LoggerClient(L"buffer", LL_DEBUG);
	
	m_szBufferSize = szBufferSize;

	*pErrCode = CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Buffer D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Buffer::~Buffer()
{
	InfoLog(m_pLoggerClient, L"Enter MemoryObject D'tor");
	delete m_pLoggerClient;
	MemoryObject::~MemoryObject();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// GetInfo D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	MemoryObject::GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret)
{
	return CL_ERR_NOT_IMPLEMENTED;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Release D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code MemoryObject::Release()
{
	return OCLObject::Release();
}