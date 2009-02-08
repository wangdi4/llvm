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
//  cl_memory_object.h
//  Implementation of the Class MemoryObject
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_OCL_MEMORY_OBJECT_H_)
#define _OCL_MEMORY_OBJECT_H_

#include <cl_types.h>
#include <logger.h>
#include <cl_object.h>
#include <logger.h>
#include "context.h"
#include <cl_device_api.h>

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Class name:	DeviceMemoryObject
	*
	* Inherit:		
	* Description:	represents a device memory object.
	*				The device memory object provides information and commands whihc related to
	*				
	* Author:		Uri Levy
	* Date:			January 2008
	**********************************************************************************************/		
	class DeviceMemoryObject
	{
	public:
		DeviceMemoryObject(Device * pDevice);

		~DeviceMemoryObject();

		// allocated device memory resource
		cl_err_code AllocateBuffer(cl_mem_flags clMemFlags, size_t szBuffersize, void * pHostPtr);

		// relese device memory resource
		virtual cl_err_code Release();

		bool IsDataValid() const { return m_bDataValid; }

		void SetDataValid(bool bDataValid) { m_bDataValid = bDataValid; }

		// is memory object was allocated within the device
		// calling to AllocateBuffer create a resource within the device and set the m_bAllocated
		// flag to True value.
		// calling to Release set the flag to False
		bool IsAllocated() const { return m_bAllocated; }

	private:

		bool			m_bAllocated;

		bool			m_bDataValid;

		Device *		m_pDevice;
	
		cl_dev_mem		m_clDevMemId;

		Intel::OpenCL::Utils::LoggerClient *	m_pLoggerClient;

	};

	/**********************************************************************************************
	* Class name:	MemoryObject
	*
	* Inherit:		OCLObject
	* Description:	represents a memory object
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class MemoryObject : public OCLObject
	{
	public:

		/******************************************************************************************
		* Function: 	MemoryObject
		* Description:	The MemoryObject class constructor
		* Arguments:	
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/		
		MemoryObject(Context * pContext, cl_mem_flags clMemFlags, void * pHostPtr, cl_err_code * pErr);

		/******************************************************************************************
		* Function: 	~MemoryObject
		* Description:	The MemoryObject class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~MemoryObject();

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

		// release the memory object
		cl_err_code Release();

		// check if the device memory object was allocated for the specific device
		// it is'nt assure that the data is available on the device. in order to know that you should
		// call to GetDataLocation
		bool IsAllocated(cl_device_id clDeviceId);

		// returns the device is where the memory is currently available
		// returns 0 when the data is not availble on any of the devices, or the memory object wasn't
		// allocated for any of the devices.
		// calling to this method doesn't promis that once it finished the data is available on the
		// same device
		cl_device_id GetDataLocation();

		// set the device id where the data is know availabe.
		// calling to this methos should be done once the data is allready available in the device
		cl_err_code SetDataLocation(cl_device_id clDevice);

		void AddPendency() { m_uiPendency++; }

		void RemovePendency() { m_uiPendency--; }

		// read the data from the memory object.
		// it is on the caller responsibility to ensure that the data which available localy on the
		// device object is the most updated data.
		virtual cl_err_code ReadData(size_t szDataSize, void * pData, size_t * pszDataSizeRet) = 0;

		// create resource of memory object for specific device.
		// this pure virtual function needs to be implemented in the buffer or image class
		virtual cl_err_code CreateDeviceResource(cl_device_id clDeviceId) = 0;

	protected:

		cl_mem_flags							m_clFlags; // memory object's flags

		void *									m_pHostPtr; // memory object's host ptr

		Context *								m_pContext;	// context to which the momory object belongs

		std::map<cl_device_id, DeviceMemoryObject*>	m_mapDeviceMemObjects; // list of device memory objects

		// recla the number of dependant commands - will be used in order to ensure that current memory
		// object is ready for release
		cl_uint									m_uiPendency;
		
		Intel::OpenCL::Utils::LoggerClient *	m_pLoggerClient;	// memory object's logger client

	};


}}};


#endif //_OCL_CONTEXT_H_