// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include <assert.h>

#include "cl_logger.h"
#include "cl_sys_defines.h"
#include "sampler.h"
#include "Context.h"

using namespace std;
using namespace Intel::OpenCL::Utils;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Sampler::Sampler(_cl_context_int* context) : OCLObject<_cl_sampler_int>(context, "Sampler")
{
	INIT_LOGGER_CLIENT("Sampler",LL_DEBUG);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
Sampler::~Sampler()
{
	LOG_DEBUG(TEXT("%s"), TEXT("Enter Sampler D'tor"));

	RELEASE_LOGGER_CLIENT;
}

cl_err_code Sampler::Initialize(SharedPtr<Context> pContext, cl_bool bNormalizedCoords, cl_addressing_mode clAddressingMode, cl_filter_mode clFilterMode)
{
	LOG_DEBUG(TEXT("%s"), TEXT("Enter Initialize"));

	assert( pContext != NULL );

	m_pContext = pContext;

	// Combine sampler properties
	m_clSamlerProps = 0;

	// Set normalized coords
	m_bNormalizedCoords = bNormalizedCoords;
	m_clSamlerProps |= bNormalizedCoords ? CLK_NORMALIZED_COORDS_TRUE : CLK_NORMALIZED_COORDS_FALSE;


	// Set Addressing Mode
	m_clAddressingMode = clAddressingMode;
	switch (clAddressingMode)
	{
	case CL_ADDRESS_NONE:
		m_clSamlerProps |= CLK_ADDRESS_NONE;
		break;
	case CL_ADDRESS_CLAMP_TO_EDGE:
		m_clSamlerProps |= CLK_ADDRESS_CLAMP_TO_EDGE;
		break;
	case CL_ADDRESS_CLAMP:
		m_clSamlerProps |= CLK_ADDRESS_CLAMP;
		break;
	case CL_ADDRESS_REPEAT:
		m_clSamlerProps |= CLK_ADDRESS_REPEAT;
		break;
	case CL_ADDRESS_MIRRORED_REPEAT:
		m_clSamlerProps |= CLK_ADDRESS_MIRRORED_REPEAT;
		break;
	default:
		return CL_INVALID_VALUE;
	}

	// Set Filtering Mode
	m_clFilterMode = clFilterMode;
	switch (clFilterMode)
	{
	case CL_FILTER_NEAREST:
		m_clSamlerProps |= CLK_FILTER_NEAREST;
		break;
	case CL_FILTER_LINEAR:
		m_clSamlerProps |= CLK_FILTER_LINEAR;
		break;
	default:
		return CL_INVALID_VALUE;
	}

	return CL_SUCCESS;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Sampler::GetInfo
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code	Sampler::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
{
	LOG_DEBUG(TEXT("Enter Sampler::GetInfo (iParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"), 
		iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

	size_t szSize = 0;
	volatile cl_context clContext = 0;
	const void * pValue = nullptr;
	
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
		clContext = m_pContext->GetHandle();
		pValue = (void*)&clContext;
		break;
	default:
		LOG_ERROR(TEXT("param_name (=%d) isn't valid"), iParamName);
		return CL_INVALID_VALUE;
	}
	if (CL_FAILED(clErrRet))
	{
		return clErrRet;
	}

	// if param_value_size < actual value size return CL_INVALID_VALUE
	if (nullptr != pParamValue && szParamValueSize < szSize)
	{
		LOG_ERROR(TEXT("szParamValueSize (=%d) < szSize (=%d)"), szParamValueSize, szSize);
		return CL_INVALID_VALUE;
	}

	// return param value size
	if (nullptr != pszParamValueSizeRet)
	{
		*pszParamValueSizeRet = szSize;
	}

	if (nullptr != pParamValue && szSize > 0)
	{
		MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
	}

	return CL_SUCCESS;
}
