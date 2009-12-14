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

#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////
//  sampler.h
//  Implementation of the Sampler objects
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_OCL_SAMPLER_H_)
#define _OCL_SAMPLER_H_

#include <cl_types.h>
#include <cl_object.h>
#include <logger.h>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	class Context;

	/**********************************************************************************************
	* Class name:	Sampler
	*
	* Inherit:		OCLObject
	* Description:	represents a sampler object
	* Author:		Uri Levy
	* Date:			May 2008
	**********************************************************************************************/		
	class Sampler : public OCLObject
	{
	public:

		/******************************************************************************************
		* Function: 	Sampler
		* Description:	The Sampler class constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			May 2008
		******************************************************************************************/		
		Sampler();

		/******************************************************************************************
		* Function: 	~Sampler
		* Description:	The Sampler class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			May 2008
		******************************************************************************************/			
		virtual ~Sampler();

		// get image info
		cl_err_code	GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		virtual cl_err_code Initialize(Context * pContext, cl_bool bNormalizedCoords, cl_addressing_mode clAddressingMode, cl_filter_mode clFilterMode);

		virtual cl_err_code Release();

		const Context * GetContext() const { return m_pContext; }

		cl_uint	GetValue() const {return m_clSamlerProps;}

	protected:

		Context *			m_pContext;	// the context to which the sampler belongs

		cl_addressing_mode	m_clAddressingMode;
		cl_filter_mode		m_clFilterMode;
		cl_bool				m_bNormalizedCoords;

		cl_uint				m_clSamlerProps;
		
		DECLARE_LOGGER_CLIENT;

	};

}}};


#endif //_OCL_SAMPLER_H_