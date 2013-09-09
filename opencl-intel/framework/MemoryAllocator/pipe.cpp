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
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES{ } LOSS OF USE, DATA, OR
// PROFITS{ } OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "pipe.h"

using namespace Intel::OpenCL::Framework;

cl_err_code Pipe::Initialize(cl_uint uiPacketSize, cl_uint uiMaxPackets, void* pHostPtr)
{
	m_uiPacketSize = uiPacketSize;
	m_uiMaxPackets = uiMaxPackets;
	const size_t szDim = CalcPipeSize(uiPacketSize, uiMaxPackets);
    if (NULL == pHostPtr)
    {
        return GenericMemObject::Initialize(CL_MEM_READ_WRITE | CL_MEM_HOST_NO_ACCESS, NULL, 1, &szDim, NULL, NULL, 0);
    }
    else
    {
        return GenericMemObject::Initialize(CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, NULL, 1, &szDim, NULL, pHostPtr, 0);
    }	
}

cl_int Pipe::GetPipeInfo(cl_pipe_info paramName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet)
{
	if (NULL != pParamValue && szParamValueSize < sizeof(cl_uint))
	{
		return CL_INVALID_VALUE;
	}
	if (NULL != pszParamValueSizeRet)
	{
		*pszParamValueSizeRet = sizeof(cl_uint);
	}
	switch (paramName)
	{
	case CL_PIPE_PACKET_SIZE:
		if (NULL != pParamValue)
		{
			*(cl_uint*)pParamValue = m_uiPacketSize;
		}
		break;
	case CL_PIPE_MAX_PACKETS:
		if (NULL != pParamValue)
		{
			*(cl_uint*)pParamValue = m_uiMaxPackets;
		}
		break;
	default:
		return CL_INVALID_VALUE;
	}
	return CL_SUCCESS;
}
