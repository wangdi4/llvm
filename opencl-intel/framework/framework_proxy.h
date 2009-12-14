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
//  FrameworkProxy.h
//  Implementation of the Class FrameworkProxy
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_types.h"
#include <platform_module.h>
#include <context_module.h>
#include <execution_module.h>
#include "logger.h"
#include "ocl_config.h"

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	FrameworkProxy
	*
	* Description:	the framework proxy class design to pass the OpenCL api calls to the 
	*				framework's moduls
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class FrameworkProxy
	{
	public:
		
		/******************************************************************************************
		* Function: 	Instance
		* Description:	Get the instance of the framework proxy module.
		* Arguments:		
		* Return value:	instance to the framework factory
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		static	FrameworkProxy * Instance();

		static void Destroy();

		/******************************************************************************************
		* Function: 	GetContextModule
		* Description:	Get handle to the context module
		* Arguments:		
		* Return value:	pointer to the ContextModule class. NULL if context module wasn't
		*				initialized successfuly
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		ContextModule * GetContextModule() const { return m_pContextModule; }
		
		/******************************************************************************************
		* Function: 	GetExecutionModule
		* Description:	Get handle to the execution module
		* Arguments:		
		* Return value:	pointer to the ExecutionModule class. NULL if module wasn't initialized
		*				successfuly
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		ExecutionModule * GetExecutionModule() const { return m_pExecutionModule; }
		
		/******************************************************************************************
		* Function: 	GetPlatformModule
		* Description:	Get handle to the platform module
		* Arguments:		
		* Return value:	pointer to the PlatformModule class. NULL if module wasn't initialized
		*				successfuly
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		PlatformModule * GetPlatformModule() const { return m_pPlatformModule; }

	private:
		/******************************************************************************************
		* Function: 	FrameworkProxy
		* Description:	The FrameworkProxy class constructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		FrameworkProxy();

		/******************************************************************************************
		* Function: 	~FrameworkProxy
		* Description:	The FrameworkProxy class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		virtual ~FrameworkProxy();

		void Initialize();
		void Release();
		
		// static instance of the framework factory class
		static  FrameworkProxy * m_pInstance;

		static ocl_entry_points	 OclEntryPoints;

		// handle to the platform module
		PlatformModule * m_pPlatformModule;
		
		// handle to the context module		
		ContextModule * m_pContextModule;

		// handle to the execution module
		ExecutionModule * m_pExecutionModule;

		// handle to the file log handler
		Intel::OpenCL::Utils::FileLogHandler * m_pFileLogHandler;
		
		// handle to the configuration object
		OCLConfig * m_pConfig;

		// handle to the logger client
		DECLARE_LOGGER_CLIENT;

	};



}}};
