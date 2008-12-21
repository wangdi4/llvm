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
//  Device.h
//  Implementation of the Class Device
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_OCL_DEVICE_H_)
#define _OCL_DEVICE_H_

#include "cl_framework.h"
#include "..\OCLObject.h"
#include "Logger.h"
#include "cl_device_api.h"


namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	Device
	*
	* Inherit:		OCLObject
	* Description:	This object is a gate from the framework into the openCL device driver that is 
	*				implemented by a seperated library.
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class Device : public OCLObject
	{
	public:

		/******************************************************************************************
		* Function: 	Device
		* Description:	The Device class constructor
		* Arguments:	deviceId [in]
		*				devEntryPoints [out]
		*				devCallBacks [out]
		*				devLogDesc [out]
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		Device(	cl_device_id deviceId, 
				cl_dev_entry_points devEntryPoints, 
				cl_dev_call_backs devCallBacks, 
				cl_dev_log_descriptor devLogDesc);

		/******************************************************************************************
		* Function: 	~Device
		* Description:	The OCLObject class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~Device();

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

	private:
		cl_device_id			m_clDeviceId;			// device id

		cl_dev_entry_points		m_clDevEntryPoints;		// device's entry points
		
		cl_dev_call_backs		m_clDevCallBacks;		// device's call backs

		cl_dev_log_descriptor	m_clDevLogDescriptor;	// device's log descriptor
	};


}}};


#endif //_OCL_DEVICE_H_