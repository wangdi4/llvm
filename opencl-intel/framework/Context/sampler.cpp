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
//  sampler.cpp
//  Implementation of the Sampler Class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "sampler.h"
#include "context.h"
#include <assert.h>
using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Sampler::Sampler()
{
	INIT_LOGGER_CLIENT(L"Sampler",LL_DEBUG);
	
	m_pHandle = new _cl_sampler;
	m_pHandle->object = this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Sampler::~Sampler()
{
    m_pContext->RemovePendency();
	LOG_DEBUG(L"Enter Sampler D'tor");

	RELEASE_LOGGER_CLIENT;

	if (NULL != m_pHandle)
	{
		delete m_pHandle;
	}
}
cl_err_code Sampler::Initialize(Context * pContext, cl_bool bNormalizedCoords, cl_addressing_mode clAddressingMode, cl_filter_mode clFilterMode)
{
	LOG_DEBUG(L"Enter Initialize");

	assert( pContext != NULL );

	m_pContext = pContext;

    // Sign to be dependent on the context, ensure the context will be delated only after the object was
    m_pContext->AddPendency();


	// Combine sampler properties
	m_clSamlerProps = 0;

	// Set normilized coords
	m_bNormalizedCoords = bNormalizedCoords;
	m_clSamlerProps |= bNormalizedCoords ? CL_DEV_SAMPLER_NORMALIZED_COORDS_TRUE : CL_DEV_SAMPLER_NORMALIZED_COORDS_FALSE;


	// Set Addressing Mode
	m_clAddressingMode = clAddressingMode;
	switch (clAddressingMode)
	{
	case CL_ADDRESS_NONE:
		m_clSamlerProps |= CL_DEV_SAMPLER_ADDRESS_NONE;
		break;
	case CL_ADDRESS_CLAMP_TO_EDGE:
		m_clSamlerProps |= CL_DEV_SAMPLER_ADDRESS_CLAMP_TO_EDGE;
		break;
	case CL_ADDRESS_CLAMP:
		m_clSamlerProps |= CL_DEV_SAMPLER_ADDRESS_CLAMP;
		break;
	case CL_ADDRESS_REPEAT:
		m_clSamlerProps |= CL_DEV_SAMPLER_ADDRESS_REPEAT;
		break;
	default:
		return CL_INVALID_VALUE;
	}

	// Set Filtering Mode
	m_clFilterMode = clFilterMode;
	switch (clFilterMode)
	{
	case CL_FILTER_NEAREST:
		m_clSamlerProps |= CL_DEV_SAMPLER_FILTER_NEAREST;
		break;
	case CL_FILTER_LINEAR:
		m_clSamlerProps |= CL_DEV_SAMPLER_FILTER_LINEAR;
		break;
	default:
		return CL_INVALID_VALUE;
	}

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler::Release
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code Sampler::Release()
{

	return OCLObject::Release();
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	Sampler::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
{
	LOG_DEBUG(L"Enter Sampler::GetInfo (iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)", 
		iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	if ((NULL == pParamValue && NULL == pszParamValueSizeRet) || 
		(NULL == pParamValue && iParamName != 0))
	{
		return CL_INVALID_VALUE;
	}
	size_t szSize = 0;
	cl_context clContext = 0;
	void * pValue = NULL;
	
	cl_err_code clErrRet = CL_SUCCESS;
	switch ( (cl_mem_info)iParamName )
	{

	case CL_SAMPLER_ADDRESSING_MODE:
		szSize = sizeof(cl_addressing_mode);
		pValue = &m_clAddressingMode;
		break;
	case CL_SAMPLER_FILTER_MODE:
		szSize = sizeof(cl_filter_mode);
		pValue = &m_clFilterMode;
		break;
	case CL_SAMPLER_NORMALIZED_COORDS:
		szSize = sizeof(cl_bool);
		pValue = &m_bNormalizedCoords;
		break;
	case CL_SAMPLER_REFERENCE_COUNT:
		szSize = sizeof(cl_uint);
		pValue = &m_uiRefCount;
		break;
	case CL_SAMPLER_CONTEXT:
		szSize = sizeof(cl_context);
		clContext = (cl_context)m_pContext->GetId();
		pValue = &clContext;
		break;
	default:
		LOG_ERROR(L"param_name (=%d) isn't valid", iParamName);
		return CL_INVALID_VALUE;
	}
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}

	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (NULL != pParamValue && szParamValueSize < szSize)
	{
		LOG_ERROR(L"szParamValueSize (=%d) < szSize (=%d)", szParamValueSize, szSize);
		return CL_INVALID_VALUE;
	}

	// return param value size
	if (NULL != pszParamValueSizeRet)
	{
		*pszParamValueSizeRet = szSize;
	}

	if (NULL != pParamValue && szSize > 0)
	{
		memcpy_s(pParamValue, szParamValueSize, pValue, szSize);
	}

	return CL_SUCCESS;
}
