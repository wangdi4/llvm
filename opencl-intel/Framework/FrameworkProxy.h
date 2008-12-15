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
///////////////////////////////////////////////////////////
//  FrameworkProxy.h
//  Implementation of the Class FrameworkProxy
//  Created on:      10-Dec-2008 8:45:02 AM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "cl_framework.h"
#include "Platform\PlatformModule.h"
#include "Context\ContextModule.h"
#include "Execution\ExecutionModule.h"
#include "Logger.h"

namespace Intel { namespace OpenCL { namespace Framework {

	/**
	* class FrameworkProxy
	*
	* the framework proxy class design to pass the OpenCL api calls to the framework's moduls
	**/
	class FrameworkProxy
	{
	public:
		
		/**
		* Instance
		* Description: Get the instance of the framework proxt module. 
		* Input Parameters: 
		*	None
		* Output Parameters: 
		*	pDevEntry:	A pointer to structure that holds set of pointers to device driver entry points
		* Returns:
		*	FrameworkFactory* - instance to the framework factory
		*/
		static	FrameworkProxy *	Instance();

		ContextModule *				GetContextModule();
		ExecutionModule *			GetExecutionModule();
		PlatformModule *			GetPlatformModule();


		
	private:
		FrameworkProxy();
		virtual ~FrameworkProxy();

		void						Initialize();
		void						Release();
		
		/**
		* static instance of the framework factory class
		*/
		static  FrameworkProxy * m_pInstance;
		
		PlatformModule * m_PlatformModule;
		ContextModule * m_ContextModule;
		ExecutionModule * m_ExecutionModule;

		FileLogHandler * m_pFileLogHandler;

	};



}}};
