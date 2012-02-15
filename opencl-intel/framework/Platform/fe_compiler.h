// Copyright (c) 2006-2012 Intel Corporation
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
//  fe_compiler.h
//  Implementation of the front-end compiler class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Logger.h>
#include <cl_dynamic_lib.h>
#include "cl_framework.h"
#include <cl_object.h>
#include <frontend_api.h>

namespace Intel { namespace OpenCL { namespace Framework {

	class IFrontendBuildDoneObserver;
	/**********************************************************************************************
	* Class name:	FECompiler
	*
	* Description:	front-end compiler class
	* Author:		Uri Levy
	* Date:			March 2008
	**********************************************************************************************/
	class FrontEndCompiler : public OCLObject<_cl_object>
	{
	
	public:

		/******************************************************************************************
		* Function: 	FECompiler
		* Description:	The Frontend compiler class constructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			March 2008
		******************************************************************************************/
		FrontEndCompiler();

		/******************************************************************************************
		* Function: 	Initialize    
		* Description:	Initialize the front-end compiler
		* Arguments:		
		* Return value:	CL_SUCCESS - The initialization operation succeeded
		* Author:		Uri Levy
		* Date:			March 2008
		******************************************************************************************/		
		cl_err_code		Initialize(const char * psModuleName, const void *pDeviceInfo, size_t stDevInfoSize);

		/******************************************************************************************
		* Function: 	FreeResources    
		* Description:	Frees the front-end compiler resources
		* Arguments:		
		* Return value:	CL_SUCCESS - The operation succeeded
		* Author:		Doron Singer
		* Date:			March 2008
		******************************************************************************************/
		void		FreeResources();

		/******************************************************************************************
		* Function: 	BuildProgram    
		* Description:	Build source code and return binary data
		* Arguments:	
		* Return value:	CL_SUCCESS - The operation succeeded
		* Author:		Uri Levy
		* Date:			March 2008
		******************************************************************************************/
		cl_err_code	BuildProgram(	cl_device_id		devId,
									cl_uint				uiStcStrCount,
									const char **		ppcSrcStrArr,
									size_t *			pszSrcStrLengths,
									const char *		psOptions,
									IFrontendBuildDoneObserver *	pBuildDoneObserver);

		/******************************************************************************************
		* Function: 	GetModuleName    
		* Description:	returns the module name of the front-end compiler
		* Arguments:	N/A
		* Return value:	[char *] - pointer to the module name's string
		* Author:		Uri Levy
		* Date:			March 2008
		******************************************************************************************/
		const char * GetModuleName() const { return m_pszModuleName; }

		//OclObject implementation
		cl_err_code	GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const {return CL_INVALID_OPERATION; }

	protected:
		/******************************************************************************************
		* Function: 	~FECompiler
		* Description:	The Frontend compiler class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			March 2008
		******************************************************************************************/
		virtual ~FrontEndCompiler();
	
		Utils::OclDynamicLib		m_dlModule;
		Intel::OpenCL::FECompilerAPI::fnCreateFECompilerInstance*	m_pfnCreateInstance;

		// module name
		const char *				m_pszModuleName;

		Intel::OpenCL::FECompilerAPI::IOCLFECompiler* m_pFECompiler;

		DECLARE_LOGGER_CLIENT;
	};

}}}
