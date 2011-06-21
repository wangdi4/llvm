// Copyright (c) 2006-2010 Intel Corporation
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
//  cl_memory_object.h
//  Implementation of the Class MemoryObject
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "cl_framework.h"
#include "Context.h"
#include "Device.h"
#include "MemoryObject.h"

#include <Logger.h>
#include <cl_object.h>
#include <cl_synch_objects.h>
#include <cl_device_api.h>
#include <stack>

// Using namespace here for mutex support in inline functions
using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework {

	class Context;


	/**********************************************************************************************
	* Class name:	SingleUnifiedMemObject
	*
	* Inherit:		MemoryObject
	* Description:	Represents a memory object that operates with single device that has unified host memory
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class SingleUnifiedMemObject : public MemoryObject
	{
	public:
		// MemoryObject methods

		// initialize the data on the memory object
		// initialize the memory object
		virtual cl_err_code Initialize(
			cl_mem_flags		clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int		dim_count,
			const size_t*		dimension,
			const size_t*       pitches,
			void*				pHostPtr
			);

		cl_err_code UpdateHostPtr(cl_mem_flags	clMemFlags, void* pHostPtr) {return CL_INVALID_OPERATION;}

		// set the device id where the data is know available.
		// calling to this methods should be done just before the write command is sent to the device agent.
		cl_err_code UpdateLocation(FissionableDevice* pDevice);

		bool	IsSharedWith(FissionableDevice* pDevice);

		cl_err_code CreateDeviceResource(FissionableDevice* pDevice);
		cl_err_code GetDeviceDescriptor(FissionableDevice* pDevice, IOCLDevMemoryObject* *ppDevObject, OclEvent** ppEvent);

		bool IsSupportedByDevice(FissionableDevice* pDevice) { return true; }

		// IDeviceFissionObserver interface
		cl_err_code NotifyDeviceFissioned(FissionableDevice* parent, size_t count, FissionableDevice** children);

    protected:
		// Low level mapped region creation function
		virtual	cl_err_code	MemObjCreateDevMappedRegion(const FissionableDevice*,
			cl_dev_cmd_param_map*	cmd_param_map);
		// Low level mapped region release function
		virtual	cl_err_code	MemObjReleaseDevMappedRegion(const FissionableDevice*,
			cl_dev_cmd_param_map*	cmd_param_map);

		/******************************************************************************************
		* Function: 	SingleUnifiedMemObject
		* Description:	The MemoryObject class constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		SingleUnifiedMemObject(Context * pContext, ocl_entry_points * pOclEntryPoints);

		/******************************************************************************************
		* Function: 	~MemoryObject
		* Description:	The MemoryObject class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~SingleUnifiedMemObject();

		IOCLDevMemoryObject*	m_pDeviceObject;
	};


}}}
