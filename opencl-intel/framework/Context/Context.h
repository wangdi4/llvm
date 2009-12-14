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
#include <cl_objects_map.h>
#include <map>
using namespace::std;

namespace Intel { namespace OpenCL { namespace Framework {

	class Buffer;
	class Image2D;
	class Image3D;
	class Sampler;
	class Device;
	class Program;
	class MemoryObject;
	//class OCLObjectsMap;

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
		Context(const cl_context_properties * clProperties, cl_uint uiNumDevices, Device **ppDevice, logging_fn pfnNotify, void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints);

		/******************************************************************************************
		* Function: 	~Device
		* Description:	The Context class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~Context();

        /******************************************************************************************
        * Function: 	Cleanup    
        * Description:	Cleanup devices in the context if terminate is false
        * Arguments:	
        * Author:		Arnon Peleg
        *
        ******************************************************************************************/
        void Cleanup( bool bTerminate = false );

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
		cl_err_code CreateProgramWithBinary(cl_uint					IN  uiNumDevices, 
											const cl_device_id *	IN  pclDeviceList, 
											const size_t *			IN  pszLengths, 
											const unsigned char **	IN  ppBinaries, 
											cl_int *				OUT piBinaryStatus, 
											Program **				OUT ppProgram);


		// get the number of devices
		const cl_uint GetDevicesCount() const { return m_pDevices->Count(); }

		// get the device object pointers that associated to the context
		Device ** GetDevices(cl_uint * puiNumDevices);

		// get the device ids that associated to the context
		cl_device_id * GetDeviceIds(cl_uint * puiNumDevices);

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

		// get device by device id
		cl_err_code GetDevice(cl_device_id clDeviceId, Device ** ppDevice)
		{
			return m_pDevices->GetOCLObject((cl_int)clDeviceId, (OCLObject**)ppDevice);
		}

        // remove the program from the context
		cl_err_code RemoveProgram(cl_program clProgramId);

		 // remove the memory object from the context
		cl_err_code RemoveMemObject(cl_mem clMem);

		// remove the memory object from the context
		cl_err_code RemoveSampler(cl_sampler clSampler);

		// create new buffer object
		cl_err_code CreateBuffer(cl_mem_flags clFlags, size_t szSize, void * pHostPtr, Buffer ** ppBuffer);

		// create new 1 or 2 dimentional image object
		cl_err_code CreateImage2D(	cl_mem_flags	        clFlags,
									const cl_image_format * pclImageFormat,
									void *                  pHostPtr,
									size_t                  szImageWidth,
									size_t                  szImageHeight,
									size_t                  szImageRowPitch,
									Image2D **		        ppImage2d);

		// create new 3 dimentional image object
		cl_err_code CreateImage3D(	cl_mem_flags	        clFlags,
									const cl_image_format * pclImageFormat,
									void *                  pHostPtr,
									size_t                  szImageWidth,
									size_t                  szImageHeight,
									size_t                  szImageDepth,
									size_t                  szImageRowPitch,
									size_t                  szImageSlicePitch,
									Image3D **		        ppImage3d);

		// get the supported image formats for this context
		cl_err_code GetSupportedImageFormats(	cl_mem_flags       clFlags,
												cl_mem_object_type clType,
												cl_uint            uiNumEntries,
												cl_image_format * pclImageFormats,
												cl_uint *         puiNumImageFormats);

		// get memory object according the mem id
		cl_err_code GetMemObject(cl_mem clMemId, MemoryObject ** ppMemObj);

		// create new sampler object
		cl_err_code CreateSampler(cl_bool bNormalizedCoords, cl_addressing_mode clAddressingMode, cl_filter_mode clFilterMode, Sampler ** ppSampler);

		// get sampler object according to the sampler id
		cl_err_code GetSampler(cl_sampler clSamplerId, Sampler ** ppSampler);

		// check that all devices belong to this context
		bool CheckDevices(cl_uint uiNumDevices, const cl_device_id * pclDevices);

        /******************************************************************************************
		* Function: 	NotifyError
		* Description:	Report information on errors that occur in this context using the callback
		*				function registered by the application
		* Arguments:	pcErrInfo [in]		- pointer to an error string
		*				pPrivateInfo [in]	- represent a pointer to binary data that is returned
		*									that can be used to log additional information helpful 
		*									in debugging the error
		*				szCb [in]			- length of binary data
		* Return value:	February
		* Author:		Uri Levy
		* Date:			January 2009
		******************************************************************************************/
		void NotifyError(const char * pcErrInfo, const void * pPrivateInfo, size_t szCb);

    private:

		cl_ulong GetMaxMemAllocSize();
		cl_err_code GetMaxImageDimensions(	size_t * psz2dWidth, 
											size_t * psz2dHeight, 
											size_t * psz3dWidth, 
											size_t * psz3dHeight, 
											size_t * psz3dDepth);

		// -------------- DEVICES -------------- 
		
		OCLObjectsMap *							m_pDevices;		// holds the devices that associated to the program
	
		Device **								m_ppDevices;

		cl_device_id *							m_pDeviceIds;

        OCLObjectsMap *							m_pPrograms;	// holds the programs that related to this context

		OCLObjectsMap *							m_pMemObjects;	// holds the memory objects that belongs to the context

		OCLObjectsMap *							m_pSamplers;	// holds the sampler objects that belongs to the context

		cl_context_properties *					m_pclContextProperties; // context properties

		cl_uint									m_uiContextPropCount;

		logging_fn								m_pfnNotify; // notify function's pointer

		void *									m_pUserData; // user data

        static map<cl_device_id, cl_uint>		m_sContextsPerDevices;   // Used to determine whether to create/close a device inst.

	};


}}};


#endif //_OCL_CONTEXT_H_