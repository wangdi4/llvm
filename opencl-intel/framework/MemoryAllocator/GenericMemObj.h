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
#include <list>
#include <vector>

// Using namespace here for mutex support in inline functions
using namespace Intel::OpenCL::Utils;

namespace Intel { namespace OpenCL { namespace Framework {

	class Context;
    class GenericMemObjectBackingStore;

    const unsigned int MAX_DEVICE_SHARING_GROUP_ID =    // how many sharing groups are supported
                                    MAX( (unsigned int)CL_DEV_MAX_BUFFER_SHARING_GROUP_ID,
                                         (unsigned int)CL_DEV_MAX_IMAGE_SHARING_GROUP_ID  );

	/**********************************************************************************************
	* Class name:	GenericMemObject
	*
	* Inherit:		MemoryObject
	* Description:	Represents a memory object that operates with multiple devices with either unified and/or descrete memories
	* Author:		Dmitry Kaptsenel
	* Date:			August 2011
	**********************************************************************************************/
	class GenericMemObject: public MemoryObject, public IOCLDevRTMemObjectService
	{
	public:
		GenericMemObject(Context * pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType);

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

        // return TRUE is device can support this sub-buffer - as for alignment and other requirements.
        // assume that all devices do support all sub-buffer alignments.
		bool IsSupportedByDevice(FissionableDevice* pDevice) { return true; }

		cl_err_code CreateSubBuffer(cl_mem_flags clFlags,
                                    cl_buffer_create_type buffer_create_type,
			                        const void * buffer_create_info,
			                        MemoryObject** ppBuffer);

		// Memory object interface
		// Assumed Read/Write is used only for data mirroring for the cases when BackingStore differ from
		// user provided host map buffer
		// This Read/Write methods DO NOT really access device memory!!!!
		cl_err_code ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);
		cl_err_code WriteData(const void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);

        // In the case when Backing Store region is different from Host Map pointer provided by user
        // we need to synchronize user area with device area after/before each map/unmap command
        //
        cl_err_code SynchDataToHost(   cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr );
        cl_err_code SynchDataFromHost( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr );

		void GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const;
		cl_err_code CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const;
		cl_err_code CheckBoundsRect( const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const;
		void * GetBackingStoreData( const size_t * pszOrigin = NULL ) const;

		size_t GetPixelSize() const;

		// Get object pitches. If pitch is irrelevant to the memory object, zero pitch is returned
		size_t GetRowPitchSize() const;
		size_t GetSlicePitchSize() const;

		// IDeviceFissionObserver interface
		cl_err_code NotifyDeviceFissioned(FissionableDevice* parent, size_t count, FissionableDevice** children);

		// IOCLDevRTMemObjectService Methods
		cl_dev_err_code GetBackingStore(cl_dev_bs_flags flags, IOCLDevBackingStore* *ppBS);
		cl_dev_err_code SetBackingStore(IOCLDevBackingStore* pBS);
		size_t GetDeviceAgentListSize() const;
		const IOCLDeviceAgent* const *GetDeviceAgentList() const;

    protected:
		virtual ~GenericMemObject();

        // copy all data required for sub-buffer
        cl_err_code InitializeSubObject(  cl_mem_flags		clMemFlags,
                                          GenericMemObject& parent,
                                          const size_t*     origin,
                                          const size_t*     region );

        virtual cl_err_code create_device_object( cl_mem_flags clMemFlags,
                                                  FissionableDevice* dev,
                                                  GenericMemObjectBackingStore* bs,
                                                  IOCLDevMemoryObject** dev_object );

    private:

		// Low level mapped region creation function
		virtual	cl_err_code	MemObjCreateDevMappedRegion(const FissionableDevice*,
			cl_dev_cmd_param_map*	cmd_param_map, void** pHostMapDataPtr);
		// Low level mapped region release function
		virtual	cl_err_code	MemObjReleaseDevMappedRegion(const FissionableDevice*,
			cl_dev_cmd_param_map*	cmd_param_map, void* pHostMapDataPtr);

        struct DeviceDescriptor
        {
            FissionableDevice*              m_pDevice;
            unsigned int                    m_sharing_group_id;
            unsigned int                    m_alignment;

            // TODO: DK: temporary unused - required for multi-device support
            bool                            m_has_data;
            bool                            m_is_owner;
            OclSpinMutex                    m_lock;

            DeviceDescriptor( FissionableDevice* dev, size_t group, size_t alignment ) :
                    m_pDevice(dev), m_sharing_group_id(group), m_alignment(alignment),
                    m_has_data(false), m_is_owner(false){};

            DeviceDescriptor( const DeviceDescriptor& o ) :
                   m_pDevice(o.m_pDevice), m_sharing_group_id(o.m_sharing_group_id), m_alignment(o.m_alignment),
                   m_has_data(o.m_has_data), m_is_owner(o.m_is_owner) {};

        };

        typedef std::list<DeviceDescriptor>         TDeviceDescList;
        typedef std::list<DeviceDescriptor*>        TDeviceDescPtrList;
        typedef std::vector<const IOCLDeviceAgent*> TDevAgentsVector;

        struct SharingGroup
        {
            TDeviceDescPtrList              m_device_list;

            // TODO: DK: temporary unused - required for multi-device support
            AtomicCounter                   m_num_of_data_containers;
            AtomicCounter                   m_num_of_data_owners;

            IOCLDevMemoryObject*            m_dev_mem_obj;

            SharingGroup() : m_dev_mem_obj(NULL) {};
        };

        // Assumption: All devices are added at class initialization time so data may be modified at runtime only
        //             because of lazy sharing groups initialization.
        //             m_global_lock may be taken only during sharing group lazy initialization.
        SharingGroup                        m_sharing_groups[MAX_DEVICE_SHARING_GROUP_ID];

        // parallel structures
        TDevAgentsVector                    m_device_agents;
        TDeviceDescList                     m_device_descriptors;

        GenericMemObjectBackingStore*       m_BS;
        unsigned int                        m_active_groups_count; // groups with allocated device objects
        OclSpinMutex                        m_global_lock;         // lock for control structures changes

        cl_err_code allocate_object_for_sharing_group( unsigned int group_id );
        const DeviceDescriptor* get_device( FissionableDevice* dev ) const;

        DeviceDescriptor* get_device( FissionableDevice* dev )
                { return const_cast<DeviceDescriptor*>( static_cast<const GenericMemObject*>(this)->get_device(dev) ); };

        IOCLDevMemoryObject* device_object( const SharingGroup& group )
            { return group.m_dev_mem_obj; };

        const IOCLDevMemoryObject* device_object( const SharingGroup& group ) const
            { return const_cast<GenericMemObject*>(this)->device_object(group); };

        IOCLDevMemoryObject* device_object( const DeviceDescriptor& dev_desc )
            { return device_object( m_sharing_groups[dev_desc.m_sharing_group_id] ); };

        const IOCLDevMemoryObject* device_object( const DeviceDescriptor& dev_desc ) const
            { return const_cast<GenericMemObject*>(this)->device_object(dev_desc); };

        void          remove_device_objects(void);

	};

	class GenericMemObjectSubBuffer : public GenericMemObject
	{
	public:
		GenericMemObjectSubBuffer(Context * pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType);

		cl_err_code Initialize(
			cl_mem_flags		clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int		dim_count,
			const size_t*		dimension,
			const size_t*       pitches,
			void*				pHostPtr
			);


		cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
			const void * buffer_create_info, MemoryObject** ppBuffer) {return CL_INVALID_MEM_OBJECT;}

		bool IsSupportedByDevice(FissionableDevice* pDevice);

	protected:
		virtual ~GenericMemObjectSubBuffer();

        virtual cl_err_code create_device_object( cl_mem_flags clMemFlags,
                                                  FissionableDevice* dev,
                                                  GenericMemObjectBackingStore* bs,
                                                  IOCLDevMemoryObject** dev_object );

		// do not implement
        GenericMemObjectSubBuffer(const GenericMemObjectSubBuffer&);
        GenericMemObjectSubBuffer& operator=(const GenericMemObjectSubBuffer&);
	};

	class GenericMemObjectBackingStore : public IOCLDevBackingStore
	{
	public:
		GenericMemObjectBackingStore(
                               cl_mem_flags		        clMemFlags,
							   const cl_image_format*	pclImageFormat,
							   unsigned int		        dim_count,
							   const size_t*	        dimension,
							   const size_t*            pitches,
							   void*			        pHostPtr,
							   size_t                   alignment );

        // for SubObject
		GenericMemObjectBackingStore(
                               cl_mem_flags		        clMemFlags,
                               IOCLDevBackingStore*     parent_ps, // used for raw data only
                               const size_t*	        origin,
                               const size_t*            region,
                               GenericMemObjectBackingStore&  copy_setting_from );

        bool AllocateData( void );

		void*                   GetRawData()     const {return m_ptr;}
        size_t                  GetRawDataSize() const {return m_raw_data_size;}
        size_t                  GetRawDataOffset( const size_t* origin ) const;
		cl_dev_bs_description   GetRawDataDecription() const
                { return (m_pHostPtr == m_ptr) ? CL_DEV_BS_USER_ALLOCATED : CL_DEV_BS_RT_ALLOCATED; }

		size_t                  GetDimCount()    const {return m_dim_count;}
		const size_t*           GetDimentions()  const {return m_dimensions;}
		bool                    IsDataValid()    const {return m_data_valid;}
		const size_t*           GetPitch()       const {return m_pitches;}
        const cl_image_format&  GetFormat()      const {return m_format;}
        size_t                  GetElementSize() const {return m_element_size;}

        size_t                  GetRequiredAlignment() const {return m_alignment;}

		int AddPendency();
		int RemovePendency();

        // methods used by GenericMemObj
        void  SetDataValid( void )   { m_data_valid = true; }
        bool  IsCopyRequired( void ) const { return m_ptr && m_pHostPtr && (m_pHostPtr != m_ptr); }

        // pointer where user should expect the data
        void* GetHostMapPtr( void )  const { return m_pHostPtr ? m_pHostPtr : m_ptr; }
        // same as GetHostMapPtr but NULL if used did not provide this pointer
        void* GetUserProvidedHostMapPtr( void )  const { return (m_creation_flags & CL_MEM_USE_HOST_PTR) ? m_pHostPtr : NULL; }

        // helper function to calculate offsets
        static size_t calculate_offset( size_t elem_size, unsigned int  dim_count,
                                        const size_t  origin[], const size_t  pitches[] );
        static size_t get_element_size( const cl_image_format* format);
        static size_t calculate_size( size_t elem_size, unsigned int  dim_count,
                                        const size_t  dimensions[], const size_t  pitches[] );

	private:
		virtual ~GenericMemObjectBackingStore();

		void*			m_ptr;

        size_t          m_dim_count;
        size_t          m_dimensions[MAX_WORK_DIM];
		size_t			m_pitches[MAX_WORK_DIM-1];
        cl_image_format m_format;
        size_t          m_element_size;

        void*           m_pHostPtr;
        cl_mem_flags    m_creation_flags;

        bool            m_data_valid;
        size_t          m_alignment;
        size_t          m_raw_data_size;

        IOCLDevBackingStore* m_parent;
		AtomicCounter	m_refCount;
	};


//
// some inline functions
//

inline size_t GenericMemObject::GetPixelSize() const
{
    return m_BS->GetElementSize();
}

inline size_t GenericMemObject::GetRowPitchSize() const
{
    return (m_BS->GetPitch())[0];
}

inline size_t GenericMemObject::GetSlicePitchSize() const
{
    return (m_BS->GetPitch())[1];
}

inline void GenericMemObject::GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const
{
    if (dimensions)
    {
        memcpy( dimensions, m_BS->GetDimentions(), sizeof(size_t)*MAX_WORK_DIM );
    }

    if (rowPitch)
    {
        *rowPitch = GetRowPitchSize();
    }

    if (slicePitch)
    {
        *slicePitch = GetSlicePitchSize();
    }
}

}}}

