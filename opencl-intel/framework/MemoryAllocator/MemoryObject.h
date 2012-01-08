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

#include "cl_object.h"
#include "cl_framework.h"
#include "observer.h"

#include <Logger.h>
#include <cl_device_api.h>
#include <stack>
#include <map>

namespace Intel { namespace OpenCL { namespace Framework {

	/**********************************************************************************************
	* Description:	Represents a base memory object interface.
	**********************************************************************************************/
	class MemoryObject;
	class Context;
	class FissionableDevice;
	class OclEvent;
	class GraphicsApiMemoryObject;

	// This struct contains information about previous maps
	// It will be inserted into the map, indexed by the pointer the device returns
	struct MapParamPerPtr
	{
		cl_dev_cmd_param_map	cmd_param_map;
		const FissionableDevice*							pDevice;
		size_t												refCount;
	};

	typedef std::pair<mem_dtor_fn,void*> MemDtorNotifyData;


	/**********************************************************************************************
	* Class name:	MemoryObject
	*
	* Description:	Declares a memory object interface
	* Author:		Uri Levy
	* Date:			December 2008
	**********************************************************************************************/
	class MemoryObject : public OCLObject<_cl_mem_int>, public IDeviceFissionObserver
	{
	public:

        friend class GraphicsApiMemoryObject;

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
		virtual cl_err_code	GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet);

		/******************************************************************************************
		* Function: 	GetImageInfo
		* Description:	get image specific information. This function extends the GetInfo for memory objects
        *               that are images. Any Non-Image memory object do not implemented this API and return error
		* Arguments:	param_name [in]				parameter's name
		*				param_value_size [inout]	parameter's value size (in bytes)
		*				param_value [out]			parameter's value
		*				param_value_size_ret [out]	parameter's value return size
		* Return value:	CL_SUCCESS - operation succeeded
		* Author:		Arnon Peleg
		* Date:			April 2009
		******************************************************************************************/
        virtual cl_err_code	GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet)
            {return CL_INVALID_MEM_OBJECT; }

		// initialize the memory object
		virtual cl_err_code Initialize(
			cl_mem_flags		clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int		dim_count,
			const size_t*		dimension,
			const size_t*       pitches,
			void*				pHostPtr
			) = 0;

		// Update the host pointer that is used for the memory object
		virtual cl_err_code UpdateHostPtr(
			cl_mem_flags		clMemFlags,
			void* pHostPtr) = 0;

		// returns the device is where the memory is currently available
		// returns NULL when the data is not available on any of the devices, or the memory object wasn't
		// allocated for any of the devices.
		// calling to this method doesn't promise that once it finished the data is available on the
		// same device
		virtual FissionableDevice* GetLocation() const	{return m_pLocation;}

		// set the device id where the data is know available.
		// calling to this methods should be done just before the write command is sent to the device agent.
		virtual cl_err_code UpdateLocation(FissionableDevice* pDevice) = 0;

		// Returns true if current location of the memory buffer can share with the requested device
		virtual bool	IsSharedWith(FissionableDevice* pDevice) = 0;

		// get the type of the memory object
		cl_mem_object_type GetType() const { return m_clMemObjectType; }

		// get parent context
		const Context *GetContext() const { return m_pContext; }

		// get memory object's flags
		cl_mem_flags GetFlags() const { return m_clFlags; }

		// Return CL_SUCCSS if flags are adequate for child buffer, or
		// CL_INVALID_OPERATION if not OK.
		int ValidateChildFlags( const cl_mem_flags childFlags);

		// Return CL_SUCCSS if flags are adequate for mapping buffer, or
		// CL_INVALID_OPERATION if not OK.
		int ValidateMapFlags( const cl_mem_flags mapFlags);

		// reads the data from the memory object into pOutData
		//
        // pszOrigin - defines the (x, y, z) offset in pixels in the memory object from where to copy.
        // If the memory object is a Buffer (1 dimension object), pszOrigin[0] refers to the offset
        // in bytes where to begin copying data into dst_buffer
        // If it is 2D image object, the z value given by pszOrigin[2] must be 0.
        // In addition to the above, if it is a buffer object, the y value given by pszOrigin[1] must be 0.
        //
        // pszRegion - defines the (width, height, depth) in pixels of the memory object rectangle to copy.
        // If the memory object is a Buffer (1 dimension object) the pszRegion[0] is the buffer size in bytes.
        // If it is a 2D image object, the height value given by pszRegion[2] must be 1.
        // In addition to the above, if it is a buffer object, the y value given by pszRegion[1] must be 1.
        //
        // szRowPitch - is the length of each row in bytes in the pOutData.
        // If szRowPitch is set to 0, the appropriate row pitch is calculated by the object itself.
        // It must be 0 if object is a Buffer.
        //
        // szSlicePitch is the size in bytes of the 2D slice of the 3D region of a 3D image.
        // If szSlicePitch is set to 0, the appropriate slice pitch is calculated by the object itself.
        // It must be 0 if object is a 2D image or a Buffer.
		//
		virtual cl_err_code ReadData(	void *          pOutData,
										const size_t *  pszOrigin,
										const size_t *  pszRegion,
										size_t          szRowPitch   = 0,
										size_t          szSlicePitch = 0) = 0;

		// writes the data from the pOutData into the memory object
		//
		virtual cl_err_code WriteData(	const void *    pOutData,
										const size_t *  pszOrigin,
										const size_t *  pszRegion,
										size_t          szRowPitch   = 0,
										size_t          szSlicePitch = 0) = 0;

		// get the total size (in bytes) of the memory object
		size_t GetSize() const {return m_stMemObjSize;}

		// Returns the number of dimensions for the memory object
		cl_uint GetNumDimensions() const {return m_uiNumDim;}

        // Get object pitches. If pitch is irrelevant to the memory object, zero pitch is returned
        virtual size_t GetRowPitchSize() const = 0;
        virtual size_t GetSlicePitchSize() const = 0;

		// Get base pixel (component size)
		virtual size_t GetPixelSize() const = 0;

		virtual void GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const = 0;

        // Check if the region defined with pszOrigin and pszOregion is within the dimensions of the memory object
        // If it is out of bounds the function returns CL_INVALID_VALUE. else returns CL_SUCCESS
        // The length of the pszOrigin and pszOregion arrays is 1,2,3 for buffer, 2D image, 3D image respectively.
        virtual cl_err_code CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const = 0;

		// Check if the region defined with pszOrigin and pszOregion is within the dimensions of the memory object
		//	with changed sizes of row and slice
		// If it is out of bounds the function returns CL_INVALID_VALUE. else returns CL_SUCCESS
		// The length of the pszOrigin and pszOregion arrays is 1,2,3 for buffer, 2D image, 3D image respectively.
		virtual cl_err_code CheckBoundsRect( const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const = 0;

		// get pointer to the data of the memory object.
		// it is on the caller responsibility to save the data.
		// if no data available locally on the memory object the function returns NULL.
        // If pszOrigin != NULL, the pointer is set to the beginning point of the origin.
		virtual void * GetBackingStoreData( const size_t * pszOrigin = NULL) const = 0;

        // create resource of memory object for specific device.
		// this pure virtual function need to be implemented in the buffer or image class
		virtual cl_err_code CreateDeviceResource(FissionableDevice* pDevice) = 0;

		// Return device resource of the memory object, associated with give device
		// Return NULL if object was not allocated for the specific device
		virtual cl_err_code GetDeviceDescriptor(FissionableDevice* IN pDevice, IOCLDevMemoryObject* OUT *ppDevObject, OclEvent* OUT *ppEvent) = 0;

		// Maps a memory object region to the host space and returns a pointer to it.
		// The function returns a pointer to the mapped region.
		// If the object is 2D/3D image and pszImageRowPitch and/or pszImageSlicePitch are not NULL, those
		// argument will include the relevant values from the device.
		virtual cl_err_code CreateMappedRegion(
			const FissionableDevice*    IN pDevice,
			cl_map_flags    IN clMapFlags,
			const size_t*   IN pOrigin,
			const size_t*   IN pRegion,
			size_t*         OUT pImageRowPitch,
			size_t*         OUT pImageSlicePitch,
			cl_dev_cmd_param_map* OUT *pMapInfo,
			void*                 OUT *pHostMapDataPtr
			);

		virtual cl_err_code GetMappedRegionInfo(const FissionableDevice* IN pDevice, void* IN mappedPtr, cl_dev_cmd_param_map* OUT *pMapInfo);

		// Release the region pointed by mappedPtr from clDeviceId.
		virtual cl_err_code ReleaseMappedRegion(cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr);

        // In the case when Backing Store region is different from Host Map pointer provided by user
        // we need to synchronize user area with device area after/before each map/unmap command
        //
        virtual cl_err_code SynchDataToHost(   cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr ) = 0;
        virtual cl_err_code SynchDataFromHost( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr ) = 0;

		/******************************************************************************************
		* Function: 	CreateSubBuffer
		* Description:	Creates sub-buffer for specific buffer.
		* Arguments:
		* Author:		Evgeny Fiksman
		* Date:			November 2010
		******************************************************************************************/
		virtual cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
			const void * buffer_create_info, MemoryObject** ppBuffer) = 0;

		/******************************************************************************************
		* Function: 	GetParent
		* Description:	Returns the pointer to parrent object if exists, otherwise NULL.
		* Arguments:	None
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/
		MemoryObject* GetParent() {return m_pParentObject;}

		/******************************************************************************************
		* Function: 	IsSupportedByDevice
		* Description:	Performs logig test if memory object is supported by specific device
		* Arguments:	pDevice - pointer to the device to be testeted.
		* Author:		Rami Jioussy
		* Date:			August 2010
		******************************************************************************************/
		virtual bool IsSupportedByDevice(FissionableDevice* pDevice) = 0;

		// Registers a callback to be called upon MemoryObject Destructor execution and before
		// any resources are being freed.
		// registered callbacks are called in reverse order.
		cl_err_code registerDtorNotifierCallback(mem_dtor_fn pfn_notify, void* pUserData);

		protected:
			MemoryObject(Context * pContext, ocl_entry_points * pOclEntryPoints);
			virtual ~MemoryObject();

			void		NotifyDestruction();

			// Low level mapped region creation function
			virtual	cl_err_code	MemObjCreateDevMappedRegion(const FissionableDevice*,
							cl_dev_cmd_param_map*	cmd_param_map, void** pHostMapDataPtr) = 0;

			virtual	cl_err_code	MemObjReleaseDevMappedRegion(const FissionableDevice*,
				cl_dev_cmd_param_map*	cmd_param_map, void* pHostMapDataPtr) = 0;

			Context*								m_pContext;	            // context to which the memory object belongs

			cl_mem_object_type						m_clMemObjectType;
			cl_image_format							m_clImageFormat;
			cl_mem_flags							m_clFlags;              // memory object's flags
			void*									m_pHostPtr;
			Intel::OpenCL::Utils::AtomicPointer<IOCLDevBackingStore>		m_pBackingStore;        // memory object's backing store
			cl_uint									m_uiNumDim;				// Number of dimension of the memory object
			void*									m_pMemObjData;			// pointer to object memory allocated area

			MemoryObject*							m_pParentObject;		// A pointer to parent memory object
			size_t									m_stOrigin[MAX_WORK_DIM]; // Origin of the sub-buffer(image)

			std::stack<MemDtorNotifyData*>			m_pfnNotifiers;		    // Holds a list of pointers to callbacks upon dtor execution
			Intel::OpenCL::Utils::OclSpinMutex		m_muNotifiers;			// Mutex for accessing m_pfnNotifiers
			Intel::OpenCL::Utils::AtomicCounter		m_mapCount;	            // A counter for the number of times an object has been mapped
			FissionableDevice*						m_pLocation;			// A pointer to device where the latest updated data is located
			std::map<void*, MapParamPerPtr*>		m_mapMappedRegions;		// A map for storage of Mapped Regions
			Intel::OpenCL::Utils::OclSpinMutex		m_muMappedRegions;		// A mutex for accessing Mapped regions
			size_t									m_stMemObjSize;			// Size of the memory object in bytes
	};


	// Declare interface for accessing memory object arrays
	class IMemoryObjectArray
	{
	public:
        /************************************************************************
         * @param index index of component inside the array
         * @return the MemoryObject object representing the component whose index in the
         *  array is index.
         ************************************************************************/
        virtual MemoryObject* GetMemObject(size_t index) = 0;

        /************************************************************************
         * @return number of Image2D objects in this Array
         ************************************************************************/
		virtual size_t GetNumObjects() const = 0;

	};

}}}
