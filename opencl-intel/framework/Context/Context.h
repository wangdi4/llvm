// Copyright (c) 2006-2012 Intel Corporation
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
#include "cl_framework.h"

#include "program_service.h"
#include "cl_object.h"
#include "cl_objects_map.h"
#include "MemoryAllocator/MemoryObjectFactory.h"
#include "MemoryAllocator/MemoryObject.h"

#include <Logger.h>
#include <cl_synch_objects.h>
#include <list>
#include <map>
#include "ocl_itt.h"
#include "cl_heap.h"

namespace Intel { namespace OpenCL { namespace Framework {

    typedef void (CL_CALLBACK *pfnNotifyBuildDone)(cl_program, void *);

	class Sampler;
    class Device;
	class FissionableDevice;
	class Program;
	class MemoryObject;

	/**********************************************************************************************
	* Class name:	Context
	*
	* Inherit:		OCLObject
	* Description:	represents a context
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/		
	class Context : public OCLObject<_cl_context_int>
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
		Context(const cl_context_properties * clProperties, cl_uint uiNumDevices, cl_uint uiNumRootDevices, FissionableDevice **ppDevice, logging_fn pfnNotify, void *pUserData, cl_err_code * pclErr, ocl_entry_points * pOclEntryPoints, ocl_gpa_data * pGPAData);

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
		* Description:	get object specific information (inherited from OCLObject) the function 
		*				query the desirable parameter value from the device
		* Arguments:	param_name [in]				parameter's name
		*				param_value_size [inout]	parameter's value size (in bytes)
		*				param_value [out]			parameter's value
		*				param_value_size_ret [out]	parameter's value return size
		* Return value:	CL_SUCCESS - operation succeeded
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/
		cl_err_code	GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret) const;

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

        /******************************************************************************************
		* Function: 	CreateProgramForLink    
		* Description:	creates an empty program
		* Arguments:	
		* Return value:	
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/
		cl_err_code CreateProgramForLink(cl_uint				IN  uiNumDevices, 
										  const cl_device_id *	IN  pclDeviceList, 
										  Program **			OUT ppProgram);

        /******************************************************************************************
		* Function: 	CompileProgram  
		* Description:	Compile program from a set of source and headers
		* Arguments:	
		* Return value:	
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/
		cl_err_code CompileProgram(cl_program			IN  clProgram, 
                                   cl_uint				IN  uiNumDevices,
								   const cl_device_id*	IN  pclDeviceList, 
                                   cl_uint				IN  uiNumHeaders,
                                   const cl_program*	IN  pclHeaders, 
                                   const char**         IN  pszHeadersNames, 
                                   const char*          IN  szOptions, 
                                   pfnNotifyBuildDone   IN  pfn_notify,
                                   void*                IN  user_data);

        /******************************************************************************************
		* Function: 	LinkProgram  
		* Description:	Link program from a set of binaries
		* Arguments:	
		* Return value:	
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/
		cl_err_code LinkProgram(cl_program				IN  clProgram, 
                                cl_uint					IN  uiNumDevices,
								const cl_device_id*	    IN  pclDeviceList, 
                                cl_uint					IN  uiNumBinaries,
                                const cl_program*		IN  pclBinaries, 
                                const char*             IN  szOptions, 
                                pfnNotifyBuildDone      IN  pfn_notify,
                                void*                   IN  user_data);

        /******************************************************************************************
		* Function: 	BuildProgram  
		* Description:	Build program from source or executable binary
		* Arguments:	
		* Return value:	
		* Author:		Sagi Shahar
		* Date:			January 2012
		******************************************************************************************/
		cl_err_code BuildProgram(cl_program				IN  clProgram, 
                                 cl_uint				IN  uiNumDevices,
								 const cl_device_id*	IN  pclDeviceList, 
                                 const char*            IN  szOptions, 
                                 pfnNotifyBuildDone     IN  pfn_notify,
                                 void*                  IN  user_data);


		// get the number of devices
		cl_uint GetDevicesCount() { return m_mapDevices.Count(); }

		// get the device object pointers that associated to the context
		FissionableDevice ** GetDevices(cl_uint * puiNumDevices);

		// get the device ids that associated to the context
		cl_device_id * GetDeviceIds(size_t * puiNumDevices);

        // Get the list of root-level devices associated with this context
        Device** GetRootDevices(cl_uint* puiNumDevices);

        cl_dev_subdevice_id GetSubdeviceId(cl_device_id id); 

        /******************************************************************************************
		* Function: 	GetDeviceByIndex
		* Description:	Get a device associated with the context according to the device index
		* Arguments:	uiDeviceIndex [in]	- Device's index
		*				pDevice	      [out]	- Placeholder for the device object. must be a valid pointer.		
		* Return value:	CL_SUCCESS -		- the device was found and returned
		*				CL_ERR_KEY_NOT_FOUND- the device index is not associated with the context
        *               CL_INVALID_VALUE    - The pDevice input is not valid.   
		* Author:		Arnon Peleg
		* Date:			January 2009
		******************************************************************************************/
        cl_err_code GetDeviceByIndex(cl_uint uiDeviceIndex, Device** pDevice);

		// get device by device id
		cl_err_code GetDevice(cl_device_id clDeviceId, FissionableDevice ** ppDevice)
		{
			return m_mapDevices.GetOCLObject((_cl_device_id_int*)clDeviceId, (OCLObject<_cl_device_id_int>**)ppDevice);
		}

        // remove the program from the context
		cl_err_code RemoveProgram(cl_program clProgramId);

		 // remove the memory object from the context
		cl_err_code RemoveMemObject(cl_mem clMem);

		// remove the memory object from the context
		cl_err_code RemoveSampler(cl_sampler clSampler);

		// create new buffer object
		cl_err_code CreateBuffer(cl_mem_flags clFlags, size_t szSize, void * pHostPtr, MemoryObject ** ppBuffer);

		// create new sub buffer object
		cl_err_code CreateSubBuffer(MemoryObject * buffer, cl_mem_flags clFlags, cl_buffer_create_type szSize, const void * buffer_create_info, MemoryObject ** ppBuffer);

		// create new image object
        template<size_t DIM, cl_mem_object_type OBJ_TYPE>
		cl_err_code CreateImage(	cl_mem_flags	        clFlags,
									const cl_image_format * pclImageFormat,
									void *                  pHostPtr,
                                    const size_t*           szImageDims,
									const size_t*           szImagePitches,
									MemoryObject **         ppImage,
                                    bool                    bIsImageBuffer);

        // create new array of image objects
        cl_err_code CreateImageArray(cl_mem_flags		    clflags,
                                     const cl_image_format*	pclImageFormat,
                                     void*					pHostPtr,
                                     const cl_image_desc*   pClImageDesc,
                                     MemoryObject**         ppImageArr);

		// get the supported image formats for this context
		cl_err_code GetSupportedImageFormats(	cl_mem_flags		clFlags,
												cl_mem_object_type	clType,
												cl_uint				uiNumEntries,
												cl_image_format*	pclImageFormats,
												cl_uint *			puiNumImageFormats);

		// get memory object according the mem id
		cl_err_code GetMemObject(cl_mem clMemId, MemoryObject ** ppMemObj);

		// create new sampler object
		cl_err_code CreateSampler(cl_bool bNormalizedCoords, cl_addressing_mode clAddressingMode, cl_filter_mode clFilterMode, Sampler ** ppSampler);

		// get sampler object according to the sampler id
		cl_err_code GetSampler(cl_sampler clSamplerId, Sampler ** ppSampler);

		// check that all devices belong to this context
		bool CheckDevices(cl_uint uiNumDevices, const cl_device_id * pclDevices);

		// Get devices from the device list
		bool GetDevicesFromList(cl_uint uiNumDevices, const cl_device_id * pclDevices, FissionableDevice** ppDevices);
		
		ocl_gpa_data * GetGPAData() const;
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

		// return context-specific memory objects heap handle
		Intel::OpenCL::Utils::ClHeap	GetMemoryObjectsHeap( void ) const { return m_MemObjectsHeap; };

	protected:
		/******************************************************************************************
		* Function: 	~Device
		* Description:	The Context class destructor
		* Arguments:		
		* Author:		Uri Levy
		* Date:			December 2008
		******************************************************************************************/			
		virtual ~Context();

		cl_ulong	GetMaxMemAllocSize();
		cl_err_code GetMaxImageDimensions(	size_t * psz2dWidth, 
											size_t * psz2dHeight, 
											size_t * psz3dWidth, 
											size_t * psz3dHeight, 
											size_t * psz3dDepth,
                                            size_t * pszArraySize,
                                            size_t * psz1dImgBufSize);

		cl_err_code	CheckSupportedImageFormat(const cl_image_format *pclImageFormat, cl_mem_flags clMemFlags, cl_mem_object_type clObjType);
		size_t		QuerySupportedImageFormats( const cl_mem_flags clMemFlags, cl_mem_object_type clObjType );


		bool									m_bTEActivated;

		// -------------- DEVICES -------------- 
		
		OCLObjectsMap<_cl_device_id_int>		m_mapDevices;			// holds the devices that associated to the program
        Device**                                m_ppRootDevices;
		FissionableDevice **					m_ppAllDevices;
		cl_device_id *							m_pDeviceIds;
        cl_device_id *                          m_pOriginalDeviceIds;
        cl_uint                                 m_pOriginalNumDevices;
        cl_uint                                 m_uiNumRootDevices;

		cl_bitfield								m_devTypeMask;			// Mask of device types involved by the context
		
        OCLObjectsMap<_cl_program_int>			m_mapPrograms;			// holds the programs that related to this context
		OCLObjectsMap<_cl_mem_int>				m_mapMemObjects;		// holds the memory objects that belongs to the context
		OCLObjectsMap<_cl_sampler_int>			m_mapSamplers;			// holds the sampler objects that belongs to the context

        Intel::OpenCL::Framework::ProgramService                          m_programService;

		cl_context_properties *					m_pclContextProperties; // context properties
//		std::map<cl_context_properties, cl_context_properties>
//												m_mapPropertyMap;		// map to search context properties

		cl_uint									m_uiContextPropCount;

		logging_fn								m_pfnNotify; // notify function's pointer

		void *									m_pUserData; // user data

		ocl_gpa_data *							m_pGPAData;
		cl_ulong								m_ulMaxMemAllocSize;
        size_t                                  m_sz1dImgBufSize;
		size_t									m_sz2dWidth;
		size_t									m_sz2dHeight;
		size_t									m_sz3dWidth;
		size_t									m_sz3dHeight;
		size_t									m_sz3dDepth;
		size_t									m_szArraySize;

		typedef std::list<cl_image_format>		tImageFormatList;
		typedef std::map<int, tImageFormatList>	tImageFormatMap;
		Intel::OpenCL::Utils::OclSpinMutex		m_muFormatsMap;
		tImageFormatMap							m_mapSupportedFormats;

		Intel::OpenCL::Utils::ClHeap			m_MemObjectsHeap;
	};

#if !defined (_WIN32)
    /* In the line:
    for (size_t i = 0; i < DIM - 1; i++)
    don't issue an error that i < DIM - 1 is always false when DIM is 1 - this is intentional */
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

template<size_t DIM, cl_mem_object_type OBJ_TYPE>
cl_err_code Context::CreateImage(cl_mem_flags	         clFlags,
                                 const cl_image_format * pclImageFormat,
                                 void *                  pHostPtr,
                                 const size_t*           szImageDims,
                                 const size_t*           szImagePitches,
                                 MemoryObject **         ppImage,
                                 bool                    bIsImageBuffer)
{
    assert ( NULL != ppImage );
    //check image sizes
    const size_t szImageDimsPerDim[3][3] = {
        { m_sz2dWidth },                            // DIM == 1
        { m_sz2dWidth, m_sz2dHeight, 0},            // DIM == 2
        { m_sz3dWidth, m_sz3dHeight, m_sz3dDepth }  // DIM == 3
    };
    // OpenCL 1.2 doesn't state that the dimensions shouldn't be 0, but I believe this is a mistake (Yariv should verify it)
    if (bIsImageBuffer)
    {        
        if (0 == szImageDims[0])
        {
            LOG_ERROR(TEXT("image width is 0") , "");
            return CL_INVALID_IMAGE_DESCRIPTOR;
        }
        if (szImageDims[0] > m_sz1dImgBufSize)
        {
            LOG_ERROR(TEXT("For a 1D image buffer, the image width must be <= CL_DEVICE_IMAGE_MAX_BUFFER_SIZE"), "");
            return CL_INVALID_IMAGE_SIZE;
        }
    }
    else
    {
        for (size_t i = 0; i < DIM; i++)
        {
            if (0 == szImageDims[i])
            {
                LOG_ERROR(TEXT("0 == szImageDims[i]"), "");
                return CL_INVALID_IMAGE_DESCRIPTOR;
            }
            if (szImageDims[i] > szImageDimsPerDim[DIM - 1][i])
            {
                LOG_ERROR(TEXT("image dimension is not allowed"), "");
                return CL_INVALID_IMAGE_SIZE;
            }
        }
    }    

    cl_err_code clErr;
    // Need to perform inverse checking, becuase CL_MEM_READ_WRITE value is 0
    // If WRITE_ONLY flag is not set check for read image support
    if ( 0 == (CL_MEM_WRITE_ONLY & clFlags) )
    {
        clErr = CheckSupportedImageFormat(pclImageFormat, CL_MEM_READ_ONLY, OBJ_TYPE);
        if (CL_FAILED(clErr))
        {
            LOG_ERROR(TEXT("Image format not supported: %S"), ClErrTxt(clErr));
            return clErr;
        }
    }
    // If READ_ONLY flag is not set check for write image support
    if ( 0 == (CL_MEM_READ_ONLY & clFlags) )
    {
        clErr = CheckSupportedImageFormat(pclImageFormat, CL_MEM_WRITE_ONLY, OBJ_TYPE);
        if (CL_FAILED(clErr))
        {
            LOG_ERROR(TEXT("Image format not supported: %S"), ClErrTxt(clErr));
            return clErr;
        }
    }

    clErr = MemoryObjectFactory::GetInstance()->CreateMemoryObject(m_devTypeMask, OBJ_TYPE, CL_MEMOBJ_GFX_SHARE_NONE, this, ppImage);
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Error creating new Image3D, returned: %ws"), ClErrTxt(clErr));
        return clErr;
    }

    size_t dim[3] = {0}, pitch[2] = {0};
    for (size_t i = 0; i < DIM; i++)
    {
        dim[i] = szImageDims[i];
    }
    for (size_t i = 0; i < DIM - 1; i++)
    {
        pitch[i] = szImagePitches[i];
    }
    if (bIsImageBuffer)
    {
        clErr = (*ppImage)->Initialize(clFlags, pclImageFormat, DIM, dim, pitch, pHostPtr, CL_RT_MEMOBJ_FORCE_BS);
    }
    else
    {
        clErr = (*ppImage)->Initialize(clFlags, pclImageFormat, DIM, dim, pitch, pHostPtr, 0);
    }    
    if (CL_FAILED(clErr))
    {
        LOG_ERROR(TEXT("Error Initialize new buffer, returned: %S"), ClErrTxt(clErr));
        (*ppImage)->Release();
        return clErr;
    }

    m_mapMemObjects.AddObject((OCLObject<_cl_mem_int>*)*ppImage);

    return clErr;
}

}}}

