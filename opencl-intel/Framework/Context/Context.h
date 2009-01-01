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
//  Context.h
//  Implementation of the Class Context
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_OCL_CONTEXT_H_)
#define _OCL_CONTEXT_H_

#include <cl_types.h>
#include <logger.h>
#include <device.h>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	Context
	*
	* Inherit:		OCLObject
	* Description:	represents a context
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class Context : public OCLObject
	{
	public:

		/******************************************************************************************
		* Function: 	Context
		* Description:	The Context class constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		Context(cl_context_properties clProperties, cl_uint uiNumDevices, Device **ppDevice, logging_fn pfnNotify, void *pUserData);

		/******************************************************************************************
		* Function: 	~Device
		* Description:	The Context class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~Context();

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


	private:

		cl_context_properties			m_clContextProperties; // context properties

		Intel::OpenCL::Utils::LoggerClient *					m_pLoggerClient;	// context's logger client

		logging_fn						m_pfnNotify; // notify function's pointer

		void *							m_pUserData; // user data

		std::map<cl_device_id,Device*>	m_mapDevices;	// map list - holds all devices
		
		cl_device_id *					m_pDeviceIds; // array of device ids
	};


}}};


#endif //_OCL_CONTEXT_H_