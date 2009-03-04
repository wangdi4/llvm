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
#include <cl_synch_objects.h>
#include "context.h"
#include <cl_device_api.h>

// Using namespace here for mutex support in inline functinos
using namespace Intel::OpenCL::Utils;


namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Enumartion:	EMemObjectType
	* Description:	Represents the memory object type
	* Members:		MOT_UNKNOWN		-	represents unknown object
	*				MOT_BUFFER		-	represents Buffer object
	*				MOT_IMAGE_2D	-	represents 2 dimentional Image object
	*				MOT_IMAGE_3D	-	represents 3 dimentional Image object
	*				MOT_SAMPLER		-	represents Sampler object
	* Author:		Uri Levy
	* Date:			January 2008
	**********************************************************************************************/
	enum EMemObjectType
	{
		MOT_UNKNOWN,
		MOT_BUFFER,
		MOT_IMAGE_2D,
		MOT_IMAGE_3D,
		MOT_SAMPLER,
	};

	/**********************************************************************************************
	* Class name:	DeviceMemoryObject
	* Inherit:		
	* Description:	represents a device memory object.
	*				The device memory object provides information and commands which related to
	*				memory resources within the device. Since the memory object assign to a context
	*				it'll have to know its status for each device, therefore it should hold such
	*				object for each device to whihc the context assigned
	* Author:		Uri Levy
	* Date:			January 2008
	**********************************************************************************************/		
	class DeviceMemoryObject
	{
	public:
		
		// Contstructor
		DeviceMemoryObject(Device * pDevice);

		// Destructor
		~DeviceMemoryObject();

		// Allocate device memory resource. each memory object (buffer, image, etc.) has its own
		// allocation function
		cl_err_code AllocateBuffer(cl_mem_flags clMemFlags, size_t szBuffersize, void * pHostPtr);

		// Relese device memory resource
		virtual cl_err_code Release();

		// get the data valid status of the device memory object
		bool IsDataValid() { OclAutoMutex CS(&m_oclLocker); return m_bDataValid; }

		// set the data valid status of the device memory object
		void SetDataValid(bool bDataValid) { OclAutoMutex CS(&m_oclLocker); m_bDataValid = bDataValid; }

		// is memory object was allocated within the device
		// calling to AllocateBuffer create a resource within the device and set the m_bAllocated
		// flag to True value.
		// calling to Release set the flag to False
		bool IsAllocated() { OclAutoMutex CS(&m_oclLocker); return m_bAllocated; }

        // Returns the device memory handler
        cl_dev_mem GetDeviceMemoryId() { OclAutoMutex CS(&m_oclLocker); return m_clDevMemId; }

	private:

		bool			m_bAllocated;	// Allocation flag - inform whether the device memory 
										//resources was allocated or not.

		bool			m_bDataValid;	// valid daya flag - inform whether the momory data
										// within the device is valid or not. it is on the
										// memory object's responsiblity to modify this flag
										// on any status changes

		Device *		m_pDevice;		// pointer to the parent device
	
		cl_dev_mem		m_clDevMemId;	// device memory handler
        
        OclMutex        m_oclLocker;        // Synch object

		LoggerClient *	m_pLoggerClient;	// logger client

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
		cl_err_code	GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		// release the memory object
		cl_err_code Release();

		// check if the device memory object was allocated for the specific device
		// it is'nt assure that the data is available on the device. in order to know that you should
		// call to GetDataLocation
        // Only if IsAllocated returns true for clDeviceId == 0 (host memory), than a zero value return
        // of GetDataLocation means that data is available in the host memory.
		bool IsAllocated(cl_device_id clDeviceId);

		// returns the device is where the memory is currently available
		// returns 0 when the data is not availble on any of the devices, or the memory object wasn't
		// allocated for any of the devices.
		// calling to this method doesn't promis that once it finished the data is available on the
		// same device
		cl_device_id GetDataLocation();

		// returns the handle for the a device memory buffer in  clDeviceId.
		// returns 0 when the data is not availble on the device, or the memory object wasn't allocated.
		// calling to this method doesn't promis that once it finished the data is available on the
		// same device
        cl_dev_mem GetDeviceMemoryHndl( cl_device_id clDeviceId );

		// set the device id where the data is know availabe.
		// calling to this methos should be done just before the write command is sent to the device agent.
		cl_err_code SetDataLocation(cl_device_id clDevice);

        // Return the cl_context handle of the context that this memory object is belong to.
        cl_context GetContextId () const { return (cl_context)(m_pContext->GetId()); };

		// get the type of the memory object
		cl_mem_object_type GetType() const { return m_clMemObjectType; }

		// get parent context
		const Context * GetContext() const { return m_pContext; }

		// get memory object's flags
		cl_mem_flags GetFlags() const { return m_clFlags; }

		// got host ptr; TODO: Uri, check if neccassery.
		const void * GetHostPtr() const { return m_pHostPtr; }

		// increase map calls counter by one, returns the map calls counter value;
		cl_uint IncreaseMapCount(){ m_uiMapCount--; return m_uiMapCount; }

		// decrease map calls counter by one, returns the map calls counter value;
		cl_uint DecreaseMapCount(){ if (m_uiMapCount > 0) m_uiMapCount--; return m_uiMapCount; }

		// return the map calls counter value
		cl_uint GetMapCount() const { return m_uiMapCount; }

		///////////////////////////////////////////////////////////////////////////////////////////
		// Pure virtual functions
		///////////////////////////////////////////////////////////////////////////////////////////

		// read the data from the memory object.
		// it is on the caller responsibility to ensure that the data which available localy on the
		// device object is the most updated data.
		// if szDataSize=0 and pData=NULL, the value in pszDataSizeRet will be the total size of
		// the memory buffer. if pData is not NULL and pszDataSizeRet is not NULL too, the value
		// in pszDataSizeRet will be the number of bytes copied
		virtual cl_err_code ReadData(void * pData, size_t szOffset, size_t szDataSize) = 0;

		// get the total size (in bytes) of the memory object
		virtual size_t GetSize() const = 0;

		// get pointer to the data of the memory object.
		// it is on the caller responsiblity to save the data.
		// if no data availalble locally on the memory object the function returns NULL.
		virtual void * GetData() const = 0;

		// create resource of memory object for specific device.
		// this pure virtual function need to be implemented in the buffer or image class
		virtual cl_err_code CreateDeviceResource(cl_device_id clDeviceId) = 0;

	protected:

		cl_mem_object_type						m_clMemObjectType;

		cl_mem_flags							m_clFlags; // memory object's flags

		void *									m_pHostPtr; // memory object's host ptr

		Context *								m_pContext;	// context to which the momory object belongs

		std::map<cl_device_id, DeviceMemoryObject*>	m_mapDeviceMemObjects; // list of device memory objects

		cl_uint									m_uiMapCount;

		bool									m_bDataOnHost;	// flag that specify if the data is available on host;

		LoggerClient *	                        m_pLoggerClient;	// memory object's logger client

	};


}}};


#endif //_OCL_CONTEXT_H_