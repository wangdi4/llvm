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
//  Implementation of the Context class
//  Created on:      10-Dec-2008 2:08:23 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////////////////////////////////////////////

#if !defined(_OCL_CONTEXT_H_)
#define _OCL_CONTEXT_H_

#include <cl_types.h>
#include <logger.h>
#include <cl_object.h>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	class Buffer;
	class Device;
	class Program;
	class MemoryObject;
	class OCLObjectsMap;

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
		* Arguments:	clProperties [in] -	context's properties
		*				uiNumDevices [in] -	number of devices associated to the context
		*				ppDevice [in] -		list of devices
		*				pfnNotify [in] -	error notification function's pointer
		*				pUserData [in] -	user date
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

		/******************************************************************************************
		* Function: 	Release    
		* Description:	relase the context and its resources
		* Arguments:	
		* Return value:	CL_SUCCESS - operation succeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code Release();

		/******************************************************************************************
		* Function: 	CreateProgramWithSource    
		* Description:	creates new program object with source code attached
		* Arguments:	
		* Return value:	
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		cl_err_code CreateProgramWithSource(cl_uint        IN  uiCount, 
											const char **  IN  ppcStrings, 
											const size_t * IN  szLengths, 
											Program **     OUT ppProgram);

		/******************************************************************************************
		* Function: 	CreateProgramWithBinary    
		* Description:	creates new program object with binaries
		* Arguments:	
		* Return value:	
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		cl_err_code CreateProgramWithBinary(cl_uint              IN  uiNumDevices, 
											const cl_device_id * IN  pclDeviceList, 
											const size_t *       IN  pszLengths, 
											const void **        IN  ppBinaries, 
											cl_int *             OUT piBinaryStatus, 
											Program **           OUT ppProgram);

		/******************************************************************************************
		* Function: 	GetDevices    
		* Description:	get the devices that associated to the context
		* Arguments:	uiNumDevices [in]
		*				ppDevices [in]
		*				puiNumDevicesRet [out]
		* Return value:	
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		cl_err_code GetDevices(cl_uint uiNumDevices, Device ** ppDevices, cl_uint * puiNumDevicesRet);


        /******************************************************************************************
		* Function: 	GetDeviceByIndex
		* Description:	Get a device associated with the context according to the device index
		* Arguments:	uiDeviceIndex [in]	- Device's index
		*				pDevice	      [out]	- Placeholder for the device object. must be a valid pointer.		
		* Return value:	CL_SUCCESS -		- the device was found and returned
		*				CL_ERR_KEY_NOT_FOUND- the device index is not associated with the contex
        *               CL_INVALID_VALUE    - The pDevice input is not valid.   
		* Author:		Arnon Peleg
		* Date:			January 2009
		******************************************************************************************/
        cl_err_code GetDeviceByIndex(cl_uint uiDeviceIndex, Device** pDevice);

        // remove the program from the context
		cl_err_code RemoveProgram(cl_program clProgramId);

		 // remove the memory object from the context
		cl_err_code RemoveMemObject(cl_mem clMem);


		cl_err_code CreateBuffer(cl_mem_flags clFlags, size_t szSize, void * pHostPtr, Buffer ** ppBuffer);

		cl_err_code GetMemObject(cl_mem clMemId, MemoryObject ** ppMemObj);

		// check that all devices belong to this context
		bool CheckDevices(cl_uint uiNumDevices, const cl_device_id * pclDevices);

    private:

		cl_ulong GetMaxMemAllocSize();

        OCLObjectsMap *							m_pPrograms;	// holds the programs that related to this context

		OCLObjectsMap *							m_pDevices;		// holds the devices that associated to the program

		OCLObjectsMap *							m_pMemObjects;	// holds the memory object that belongs to the context

		cl_context_properties					m_clContextProperties; // context properties

		logging_fn								m_pfnNotify; // notify function's pointer

		void *									m_pUserData; // user data

		Intel::OpenCL::Utils::LoggerClient *	m_pLoggerClient;	// context's logger client
	};


}}};


#endif //_OCL_CONTEXT_H_