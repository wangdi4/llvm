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
//  fe_compiler.h
//  Implementation of the front-end compiler class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <cl_dynamic_lib.h>
#include <cl_types.h>
#include <logger.h>
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
	class FECompiler : public OCLObject
	{
	
	public:

		/******************************************************************************************
		* Function: 	FECompiler
		* Description:	The Frontend compiler class constructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			March 2008
		******************************************************************************************/
		FECompiler();

		/******************************************************************************************
		* Function: 	~FECompiler
		* Description:	The Frontend compiler class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			March 2008
		******************************************************************************************/
		virtual ~FECompiler();

		/******************************************************************************************
		* Function: 	Initialize    
		* Description:	Initialize the front-end compiler
		* Arguments:		
		* Return value:	CL_SUCCESS - The initializtion operation succeded
		* Author:		Uri Levy
		* Date:			March 2008
		******************************************************************************************/		
		cl_err_code		Initialize(const char * psModuleName);

		/******************************************************************************************
		* Function: 	Release    
		* Description:	Release the front-end compiler resources
		* Arguments:		
		* Return value:	CL_SUCCESS - The release operation succeded
		* Author:		Uri Levy
		* Date:			March 2008
		******************************************************************************************/
		cl_err_code		Release();

		/******************************************************************************************
		* Function: 	BuildProgram    
		* Description:	Build source code and return binary data
		* Arguments:	
		* Return value:	CL_SUCCESS - The operation succedeed
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
	
	private:
		Utils::OclDynamicLib		m_dlModule;
		// module name
		const char *				m_pszModuleName;
		fn_FEBuildProgram	*		m_fnBuild;

		static void BuildNotifyCallBack(void* pData, void* pBuffer, size_t stBufferSize, int iStatus, const char* szErrLog);

		DECLARE_LOGGER_CLIENT;
	};

}}};
