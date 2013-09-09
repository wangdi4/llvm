// Copyright (c) 2006-2013 Intel Corporation
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

#pragma once

#include "GenericMemObj.h"
#include "PipeCommon.h"

namespace Intel { namespace OpenCL { namespace Framework {

/**
 * This class represents a pipe memory object
 */
class Pipe : public GenericMemObject
{

	PREPARE_SHARED_PTR(Pipe)

	static SharedPtr<Pipe> Allocate(const SharedPtr<Context>& pContext, cl_mem_object_type clObjType)
	{
		return new Pipe(pContext, clObjType);
	}

    /**
     * @param uiPacketSize the size in byte of this Pipe's packet
	 * @param uiMaxPackets the maximum number of packets this Pipe can hold
     * @return the pipe's size
     */
    static size_t CalcPipeSize(cl_uint uiPacketSize, cl_uint uiMaxPackets) { return INTEL_PIPE_HEADER_RESERVED_SPACE + uiPacketSize * uiMaxPackets; }

	/**
	 * Initialize this Pipe
	 * @param uiPacketSize the size in byte of this Pipe's packet
	 * @param uiMaxPackets the maximum number of packets this Pipe can hold
     * @param pHostPtr     optional pointer coming from CRT
	 * @return CL_SUCCESS in case of success or error code in case of failure
	 */
	cl_err_code Initialize(cl_uint uiPacketSize, cl_uint uiMaxPackets, void* pHostPtr);

	/**
	 * @param paramName				specifies the information to query
	 * @param szParamValueSize		is used to specify the size in bytes of memory pointed to by paramName
	 * @param pParamValue			is a pointer to memory where the appropriate result being queried is returned. If pParamValue is NULL, it is ignored.
	 * @param pszParamValueSizeRet	returns the actual size in bytes of data being queried by pParamValue. If pszParamValueSizeRet is NULL, it is ignored.
	 * @return CL_SUCCESS in case of success or error code in case of failure
	 */
	cl_int GetPipeInfo(cl_pipe_info paramName, size_t szParamValueSize, void* pParamValue, size_t* pszParamValueSizeRet);

private:

	Pipe(const SharedPtr<Context>& pContext, cl_mem_object_type clObjType) : GenericMemObject(pContext, clObjType)
	{
		assert(CL_MEM_OBJECT_PIPE == clObjType && "CL_MEM_OBJECT_PIPE != clObjType");
	}

	cl_uint m_uiPacketSize;
	cl_uint m_uiMaxPackets;

};

}}}
