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
//  program.h
//  Implementation of the Program class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_OCL_PROGRAM_H_)
#define _OCL_PROGRAM_H_

#include <cl_types.h>
#include <cl_object.h>
#include <logger.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class Context;

	/**********************************************************************************************
	* Class name:	Buffer
	*
	* Inherit:		MemoryObject
	* Description:	represents a memory object
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class Program : public OCLObject
	{
	public:

		/******************************************************************************************
		* Function: 	Program
		* Description:	The Program class constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		Program(Context * pContext);

		/******************************************************************************************
		* Function: 	~Program
		* Description:	The Program class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~Program();

		/******************************************************************************************
		* Function: 	GetInfo    
		* Description:	get object specific information (inharited from OCLObject) the function 
		*				query the desirable parameter value from the device
		* Arguments:	param_name [in]				parameter's name
		*				param_value_size [inout]	parameter's value size (in bytes)
		*				param_value [out]			parameter's value
		*				param_value_size_ret [out]	parameter's value return size
		* Return value:	CL_SUCCESS - operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code	GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret);

		cl_err_code Release();

		cl_err_code AddSource(cl_uint uiCount, const char ** ppcStrings, const size_t * pszLengths);

	private:

		cl_uint			m_uiStcStrCount;
		
		char **			m_ppcSrcStrArr;
		
		size_t *		m_pszSrcStrLengths;

		Context *		m_pContext;

		Intel::OpenCL::Utils::LoggerClient *	m_pLoggerClient;

	};


}}};


#endif //_OCL_PROGRAM_H_