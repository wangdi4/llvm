// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include "cl_object.h"
#include "cl_framework.h"
#include "observer.h"

#include <Logger.h>
#include <cl_device_api.h>
#include <stack>
#include <map>
#include "Device.h"
#include "Context.h"

namespace Intel { namespace OpenCL { namespace Framework {

    /**********************************************************************************************
    * Description:    Represents a base memory object interface.
    **********************************************************************************************/
    class MemoryObject;
    class OclEvent;
    class GraphicsApiMemoryObject;

    // This struct contains information about previous maps
    // It will be inserted into the map, indexed by the pointer the device returns
    struct MapParamPerPtr
    {
        cl_dev_cmd_param_map    cmd_param_map;
        size_t                  refCount;
        size_t                  invalidateRefCount;
        bool                    full_object_ovewrite;
        SharedPtr<MemoryObject> mappedMemObj;            // Need it in order to ensure unmapping of this region before releasing the mem object related.
        MapParamPerPtr(const SharedPtr<MemoryObject>& memObj) : refCount(0), invalidateRefCount(0) , full_object_ovewrite(false), mappedMemObj(memObj) {};
    };

    typedef std::pair<mem_dtor_fn,void*> MemDtorNotifyData;

    /*! \define cl_rt_memobj_creation_flags
    * Defines a set of bitflags that modify memory objects creation behavior depending on Framework internal requirements
    */
    /* cl_rt_memobj_creation_flags */
    typedef unsigned int cl_rt_memobj_creation_flags;
    #define CL_RT_MEMOBJ_FORCE_BS (1<<0) //! If HostPtr provided - use it as a backing store and mark BS as CL_DEV_BS_RT_MAPPED

    /**********************************************************************************************
    * Class name:    MemoryObject
    *
    * Description:    Declares a memory object interface
    * Author:        Uri Levy
    * Date:            December 2008
    **********************************************************************************************/
    class MemoryObject : public OCLObject<_cl_mem_int>
    {
    public:

        PREPARE_SHARED_PTR(MemoryObject)
        friend class GraphicsApiMemoryObject;

        /******************************************************************************************
        * Function:     GetInfo
        * Description:    get object specific information (inherited from OCLObject) the function
        *                query the desirable parameter value from the device
        * Arguments:    param_name [in]                parameter's name
        *                param_value_size [inout]    parameter's value size (in bytes)
        *                param_value [out]            parameter's value
        *                param_value_size_ret [out]    parameter's value return size
        * Return value:    CL_SUCCESS - operation succeeded
        * Author:        Uri Levy
        * Date:            December 2008
        ******************************************************************************************/
        virtual cl_err_code    GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const;

        /******************************************************************************************
        * Function:     GetImageInfo
        * Description:    get image specific information. This function extends the GetInfo for memory objects
        *               that are images. Any Non-Image memory object do not implemented this API and return error
        * Arguments:    param_name [in]                parameter's name
        *                param_value_size [inout]    parameter's value size (in bytes)
        *                param_value [out]            parameter's value
        *                param_value_size_ret [out]    parameter's value return size
        * Return value:    CL_SUCCESS - operation succeeded
        * Author:        Arnon Peleg
        * Date:            April 2009
        ******************************************************************************************/
        virtual cl_err_code    GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
            {return CL_INVALID_MEM_OBJECT; }

        // initialize the memory object
        virtual cl_err_code Initialize(
            cl_mem_flags            clMemFlags,
            const cl_image_format*  pclImageFormat,
            unsigned int            dim_count,
            const size_t*           dimension,
            const size_t*           pitches,
            void*                   pHostPtr,
            cl_rt_memobj_creation_flags    creation_flags, // modification flags
            size_t                  force_alignment = 0
            ) = 0;

        // Update the host pointer that is used for the memory object
        virtual cl_err_code UpdateHostPtr(
            cl_mem_flags        clMemFlags,
            void* pHostPtr) = 0;

        // 
        // Ownership and data validity management
        //
        // Memory object may be valid on multiple devices at the same time
        // Memory object should be locked on a device before usage and released 
        // Memory object current position quering does not guarantee that this position will be still valid 
        // after query returns - need to lock before usage also.
        //
        enum MemObjUsage 
        {
            READ_ONLY = 0,      // data will only be read on this device 
            READ_WRITE,         // data may be both read and write
            WRITE_ENTIRE,       // data will be written only, old data is not required

            MEMOBJ_USAGES_COUNT // must be the last 
        };

        // returns NULL id data is ready and locked on given device, 
        // non-NULL if data is in the process of copying. Returned event may be added to dependency list
        // by the caller
        // pOutActuallyUsage is the MemObjUsage that the memory object realy locked on (Can be usage or READ_WRITE)
        virtual cl_err_code LockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT MemObjUsage* pOutActuallyUsage, OUT SharedPtr<OclEvent>& pOutEvent ) = 0;

        // release data locking on device. 
        // MUST pass the same usage value as set in pOutActuallyUsage during LockOnDevice execution.
        virtual cl_err_code UnLockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage ) = 0;

        //
        // end of ownership and data validity management
        //
     

        // get the type of the memory object
        cl_mem_object_type GetType() const { return m_clMemObjectType; }        

        // get parent context
        SharedPtr<Context> GetContext() { return m_pContext; }
        ConstSharedPtr<Context> GetContext() const { return m_pContext; }

        // get memory object's flags
        cl_mem_flags GetFlags() const { return m_clFlags; }

        // Return CL_SUCCSS if flags are adequate for child buffer, or
        // CL_INVALID_OPERATION if not OK.
        virtual int ValidateChildFlags( const cl_mem_flags childFlags);

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
        virtual cl_err_code ReadData(   void *          pOutData,
                                        const size_t *  pszOrigin,
                                        const size_t *  pszRegion,
                                        size_t          szRowPitch   = 0,
                                        size_t          szSlicePitch = 0) = 0;

        // writes the data from the pOutData into the memory object
        //
        virtual cl_err_code WriteData(  const void *    pOutData,
                                        const size_t *  pszOrigin,
                                        const size_t *  pszRegion,
                                        size_t          szRowPitch   = 0,
                                        size_t          szSlicePitch = 0) = 0;

        // get the total size (in bytes) of the memory object
        size_t GetSize() const {return m_stMemObjSize;}

        // Returns the number of dimensions for the memory object
        cl_uint GetNumDimensions() const {return m_uiNumDim;}
        virtual cl_err_code GetDimensionSizes( size_t* pszRegion ) const = 0;

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
        //    with changed sizes of row and slice
        // If it is out of bounds the function returns CL_INVALID_VALUE. else returns CL_SUCCESS
        // The length of the pszOrigin and pszOregion arrays is 1,2,3 for buffer, 2D image, 3D image respectively.
        virtual cl_err_code CheckBoundsRect( const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const = 0;

        // Check that whole object is covered. 
        bool IsWholeObjectCovered( cl_uint dims, const size_t* pszOrigin, const size_t* pszRegion );

        // get pointer to the data of the memory object.
        // it is on the caller responsibility to save the data.
        // if no data available locally on the memory object the function returns NULL.
        // If pszOrigin != NULL, the pointer is set to the beginning point of the origin.
        virtual void * GetBackingStoreData( const size_t * pszOrigin = NULL) const = 0;

        // create resource of memory object for specific device.
        // this pure virtual function need to be implemented in the buffer or image class
        virtual cl_err_code CreateDeviceResource(const SharedPtr<FissionableDevice>& pDevice) = 0;

        // Return device resource of the memory object, associated with give device
        // Return NULL if object was not allocated for the specific device
        virtual cl_err_code GetDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT *ppDevObject, SharedPtr<OclEvent> OUT *ppEvent) = 0;

        // Return device resource of the memory object, associated with give device
        // This method is call when delayes Gfx Acquire operation is completed
        virtual cl_err_code UpdateDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT *ppDevObject) = 0;

        // Maps a memory object region to the host space and returns a pointer to it.
        // The function returns a pointer to the mapped region.
        // If the object is 2D/3D image and pszImageRowPitch and/or pszImageSlicePitch are not NULL, those
        // argument will include the relevant values from the device.
        //
        // Note: This function DOES NOT performs the mapping itself - it just notifies device about future
        //       mapping and creates a mapping handle!
        virtual cl_err_code CreateMappedRegion(
            SharedPtr<FissionableDevice>    IN  pDevice,              // preferred device
            cl_map_flags                    IN  clMapFlags,
            const size_t*                   IN  pOrigin,
            const size_t*                   IN  pRegion,
            size_t*                         OUT pImageRowPitch,
            size_t*                         OUT pImageSlicePitch,
            cl_dev_cmd_param_map*           OUT *pMapInfo,
            void*                           OUT *pHostMapDataPtr,
            ConstSharedPtr<FissionableDevice>    OUT *pActualMappingDevice
            );

        virtual cl_err_code GetMappedRegionInfo(ConstSharedPtr<FissionableDevice> IN pDevice, void* IN mappedPtr, 
                                                cl_dev_cmd_param_map*    OUT *pMapInfo, 
                                                ConstSharedPtr<FissionableDevice> OUT *pMappedOnDevice,
                                                bool                     OUT *pbWasFullyOverwritten,
                                                bool invalidateRegion = false);

        // Release the region pointed by mappedPtr from clDeviceId.
        virtual cl_err_code ReleaseMappedRegion(cl_dev_cmd_param_map* IN pMapInfo, 
                                                void* IN pHostMapDataPtr, 
                                                bool invalidatedBefore = false );

        virtual void        ReleaseAllMappedRegions();

        virtual cl_err_code UndoMappedRegionInvalidation(cl_dev_cmd_param_map* IN pMapInfo );

        // In the case when Backing Store region is different from Host Map pointer provided by user
        // we need to synchronize user area with device area after/before each map/unmap command
        //
        virtual bool        IsSynchDataWithHostRequired( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr ) const = 0;
        virtual cl_err_code SynchDataToHost(   cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr ) = 0;
        virtual cl_err_code SynchDataFromHost( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr ) = 0;

        /******************************************************************************************
        * Function:     CreateSubBuffer
        * Description:    Creates sub-buffer for specific buffer.
        * Arguments:
        * Author:        Evgeny Fiksman
        * Date:            November 2010
        ******************************************************************************************/
        virtual cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
            const void * buffer_create_info, SharedPtr<MemoryObject>* ppBuffer) = 0;

        /******************************************************************************************
        * Function:     GetParent
        * Description:    Returns the pointer to parrent object if exists, otherwise NULL.
        * Arguments:    None
        * Author:        Rami Jioussy
        * Date:            August 2010
        ******************************************************************************************/
        SharedPtr<MemoryObject> GetParent() {return m_pParentObject;}
        ConstSharedPtr<MemoryObject> GetParent() const {return m_pParentObject;}

        /******************************************************************************************
        * Function:     SetParent
        * Description:  Sets the parrent object. This method should not be called if a parent object already exists.
        * Arguments:    pParentObject the parrent object to set
        * Author:       Aharon Abramson
        * Date:         September 2014
        ******************************************************************************************/
        void SetParent(const SharedPtr<MemoryObject>& pParentObject)
        {
            assert(0 == m_pParentObject);
            m_pParentObject = pParentObject;
        }

        /******************************************************************************************
        * Function:     IsSupportedByDevice
        * Description:    Performs logig test if memory object is supported by specific device
        * Arguments:    pDevice - pointer to the device to be testeted.
        * Author:        Rami Jioussy
        * Date:            August 2010
        ******************************************************************************************/
        virtual bool IsSupportedByDevice(const SharedPtr<FissionableDevice>& pDevice) = 0;

        // Registers a callback to be called upon MemoryObject Destructor execution and before
        // any resources are being freed.
        // registered callbacks are called in reverse order.
        cl_err_code registerDtorNotifierCallback(mem_dtor_fn pfn_notify, void* pUserData);

        // We need to trace all objects that were mapped in order to remove all mappings in the case of shutdown
        // Implement own life cycle management
        virtual void EnterZombieState( EnterZombieStateLevel call_level );

        // returns the address of the host pointer or NULL if there is none
        const void* GetHostPtr() const { return m_pHostPtr; }

        protected:
            MemoryObject(SharedPtr<Context> pContext);
            virtual ~MemoryObject();

            void        NotifyDestruction();

            // Low level mapped region creation function
            virtual    cl_err_code    MemObjCreateDevMappedRegion(const SharedPtr<FissionableDevice>&,
                            cl_dev_cmd_param_map*    cmd_param_map, void** pHostMapDataPtr) = 0;

            virtual    cl_err_code    MemObjReleaseDevMappedRegion(const SharedPtr<FissionableDevice>&,
                            cl_dev_cmd_param_map*    cmd_param_map, void* pHostMapDataPtr, bool force_unmap = false) = 0;

            typedef std::multimap<void*, MapParamPerPtr*>   Addr2MapRegionMultiMap;

            SharedPtr<Context>                          m_pContext;                 // context to which the memory object belongs

            cl_mem_object_type                          m_clMemObjectType;
            cl_image_format                             m_clImageFormat;
            cl_mem_flags                                m_clFlags;                  // memory object's flags
            void*                                       m_pHostPtr;
            Intel::OpenCL::Utils::AtomicPointer<IOCLDevBackingStore> m_pBackingStore;// memory object's backing store
            cl_uint                                     m_uiNumDim;                 // Number of dimension of the memory object
            void*                                       m_pMemObjData;              // pointer to object memory allocated area

            SharedPtr<MemoryObject>                     m_pParentObject;            // A pointer to parent memory object
            size_t                                      m_stOrigin[MAX_WORK_DIM];   // Origin of the sub-buffer(image)

            std::stack<MemDtorNotifyData*>              m_pfnNotifiers;             // Holds a list of pointers to callbacks upon dtor execution
            Intel::OpenCL::Utils::OclSpinMutex          m_muNotifiers;              // Mutex for accessing m_pfnNotifiers
            Intel::OpenCL::Utils::AtomicCounter         m_mapCount;                 // A counter for the number of times an object has been mapped
            Addr2MapRegionMultiMap                      m_mapMappedRegions;         // A map for storage of Mapped Regions
            SharedPtr<FissionableDevice>                m_pMappedDevice;            // A device that manages mapped regions
            Intel::OpenCL::Utils::OclSpinMutex          m_muMappedRegions;          // A mutex for accessing Mapped regions
            size_t                                      m_stMemObjSize;             // Size of the memory object in bytes
            volatile mutable bool                       m_bRegisteredInContextModule;// this memory object has an additional reference from context_module
    };


    // Declare interface for accessing memory object arrays
    class IMemoryObjectArray
    {
    public:

        /************************************************************************
         * @return number of Image2D objects in this Array
         ************************************************************************/
        virtual size_t GetNumObjects() const = 0;

    };

}}}
