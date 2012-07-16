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
#include "cl_heap.h"
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

        PREPARE_SHARED_PTR(GenericMemObject);

        static SharedPtr<GenericMemObject> Allocate(SharedPtr<Context> pContext, cl_mem_object_type clObjType)
        {
            return SharedPtr<GenericMemObject>(new GenericMemObject(pContext, clObjType));
        }
	
		// MemoryObject methods

		// initialize the data on the memory object
		// initialize the memory object
		virtual cl_err_code Initialize(
			cl_mem_flags			clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int			dim_count,
			const size_t*			dimension,
			const size_t*			pitches,
			void*					pHostPtr,
			cl_rt_memobj_creation_flags	creation_flags
			);

		cl_err_code UpdateHostPtr(cl_mem_flags	clMemFlags, void* pHostPtr) {return CL_INVALID_OPERATION;}

        // returns NULL id data is ready and locked on given device, 
        // non-NULL if data is in the process of copying. Returned event may be added to dependency list
        // by the caller
        SharedPtr<OclEvent> LockOnDevice( IN ConstSharedPtr<FissionableDevice> dev, IN MemObjUsage usage );

        // release data locking on device. 
        // MUST pass the same usage value as LockOnDevice
        void UnLockOnDevice( IN ConstSharedPtr<FissionableDevice> dev, IN MemObjUsage usage );

		cl_err_code CreateDeviceResource(SharedPtr<FissionableDevice> pDevice);
		cl_err_code GetDeviceDescriptor(SharedPtr<FissionableDevice> pDevice, IOCLDevMemoryObject* *ppDevObject, SharedPtr<OclEvent>* ppEvent);
		cl_err_code UpdateDeviceDescriptor(SharedPtr<FissionableDevice> pDevice, IOCLDevMemoryObject* *ppDevObject);

        // return TRUE is device can support this sub-buffer - as for alignment and other requirements.
        // assume that all devices do support all sub-buffer alignments.
		bool IsSupportedByDevice(SharedPtr<FissionableDevice> pDevice) { return true; }

		cl_err_code CreateSubBuffer(cl_mem_flags clFlags,
                                    cl_buffer_create_type buffer_create_type,
			                        const void * buffer_create_info,
			                        SharedPtr<MemoryObject>* ppBuffer);

		// Memory object interface
		// Assumed Read/Write is used only for data mirroring for the cases when BackingStore differ from
		// user provided host map buffer
		// This Read/Write methods DO NOT really access device memory!!!!
		cl_err_code ReadData(void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);
		cl_err_code WriteData(const void * pData, const size_t * pszOrigin, const size_t * pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);

        // In the case when Backing Store region is different from Host Map pointer provided by user
        // we need to synchronize user area with device area after/before each map/unmap command
        //
		bool        IsSynchDataWithHostRequired( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr ) const;
        cl_err_code SynchDataToHost(   cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr );
        cl_err_code SynchDataFromHost( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr );

		cl_err_code GetDimensionSizes( size_t* pszRegion ) const;
		void GetLayout( OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch ) const;
		cl_err_code CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const;
		cl_err_code CheckBoundsRect( const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const;
		void * GetBackingStoreData( const size_t * pszOrigin = NULL ) const;

		cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const;

		size_t GetPixelSize() const;

		// Get object pitches. If pitch is irrelevant to the memory object, zero pitch is returned
		size_t GetRowPitchSize() const;
		size_t GetSlicePitchSize() const;

		// IDeviceFissionObserver interface
		cl_err_code NotifyDeviceFissioned(SharedPtr<FissionableDevice> parent, size_t count, SharedPtr<FissionableDevice>* children);

		// IOCLDevRTMemObjectService Methods
		cl_dev_err_code GetBackingStore(cl_dev_bs_flags flags, IOCLDevBackingStore* *ppBS);
        cl_dev_err_code GetBackingStore(cl_dev_bs_flags flags, const IOCLDevBackingStore** ppBS) const;
		cl_dev_err_code SetBackingStore(IOCLDevBackingStore* pBS);
		size_t GetDeviceAgentListSize() const;
		const IOCLDeviceAgent* const *GetDeviceAgentList() const;
        virtual ~GenericMemObject();
        cl_mem_object_type GetMemObjectType() const { return GetType(); }
        // Device Agent should notify when long update to/from backing store operations finished.
        //      Pass HANDLE value that was provided to Device Agent when update API was called
        void BackingStoreUpdateFinished( IN void* handle, cl_dev_err_code dev_error );

    protected:
		
        GenericMemObject(SharedPtr<Context> pContext, cl_mem_object_type clObjType);

        // copy all data required for sub-buffer
        cl_err_code InitializeSubObject(  cl_mem_flags		clMemFlags,
                                          GenericMemObject& parent,
                                          const size_t*     origin,
                                          const size_t*     region );

        virtual cl_err_code create_device_object( cl_mem_flags clMemFlags,
                                                  SharedPtr<FissionableDevice> dev,
                                                  GenericMemObjectBackingStore* bs,
                                                  IOCLDevMemoryObject** dev_object );

    private:

        //
        // Internal DataCopyEvent
        //
        class DataCopyEvent : public OclEvent
        {
        public:

            PREPARE_SHARED_PTR(DataCopyEvent);

            static SharedPtr<DataCopyEvent> Allocate(_cl_context_int* context)
            {
                return SharedPtr<DataCopyEvent>(new GenericMemObject::DataCopyEvent(context));
            }

	        // Get the return code of the command associated with the event.
	        cl_int     GetReturnCode() const {return 0;}
	        cl_err_code	GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
		        {return CL_INVALID_OPERATION;}

            void SetCompletionRequired () { m_completion_required = true; };
            bool IsCompletionRequired() const { return m_completion_required; };

            void SetComplete() 
            { 
                NotifyComplete( CL_SUCCESS ); 
                //RemovePendency( NULL );// Why we need this?
            };

        private:

            DataCopyEvent(_cl_context_int* context) : OclEvent(context), m_completion_required(false)
            {
                SetEventState(EVENT_STATE_HAS_DEPENDENCIES); 
            };

            bool m_completion_required;
    
	        virtual ~DataCopyEvent() {};        

	        // A MemObjectEvent object cannot be copied
	        DataCopyEvent(const DataCopyEvent&);           // copy constructor
	        DataCopyEvent& operator=(const DataCopyEvent&);// assignment operator
        };

        //
        // END OF Internal DataCopyEvent
        //

		// Low level mapped region creation function
		virtual	cl_err_code	MemObjCreateDevMappedRegion(SharedPtr<FissionableDevice>,
			cl_dev_cmd_param_map*	cmd_param_map, void** pHostMapDataPtr);
		// Low level mapped region release function
		virtual	cl_err_code	MemObjReleaseDevMappedRegion(SharedPtr<FissionableDevice>,
			cl_dev_cmd_param_map*	cmd_param_map, void* pHostMapDataPtr);

        struct DeviceDescriptor
        {
            SharedPtr<FissionableDevice>              m_pDevice;
            size_t							m_sharing_group_id;
            size_t							m_alignment;

            DeviceDescriptor( SharedPtr<FissionableDevice> dev, size_t group, size_t alignment ) :
                    m_pDevice(dev), m_sharing_group_id(group), m_alignment(alignment) {};

            DeviceDescriptor( const DeviceDescriptor& o ) :
                   m_pDevice(o.m_pDevice), m_sharing_group_id(o.m_sharing_group_id), m_alignment(o.m_alignment) {};

        };

        typedef std::list<DeviceDescriptor>                     TDeviceDescList;
        typedef std::list<DeviceDescriptor*>                    TDeviceDescPtrList;
        typedef std::map<ConstSharedPtr<FissionableDevice>,DeviceDescriptor*>  TDevice2DescPtrMap;
        typedef std::vector<const IOCLDeviceAgent*>             TDevAgentsVector;

        // data copy state
        enum DataCopyState
        {
            DATA_COPY_STATE_INVALID = 0,
            DATA_COPY_STATE_TO_BS,      // waiting for copy from sharing group to Backing Store
            DATA_COPY_STATE_FROM_BS,    // waiting for copy from Backing Store to sharing group
            DATA_COPY_STATE_VALID       // data valid on sharing group
        };

        struct SharingGroup
        {
            TDeviceDescPtrList              m_device_list;
            IOCLDevMemoryObject*            m_dev_mem_obj;

            // usage counters
            unsigned int                    m_active_users_count;         // number of active users inside group
            unsigned int                    m_active_writers_count;       // number of active users with write access
            
            // copy-related
            SharedPtr<DataCopyEvent>        m_data_copy_in_process_event; // for sharing group data use
            DataCopyState                   m_data_copy_state;            // 
            unsigned int                    m_data_copy_from_group;       // if copy is in progress from other sharing group
            unsigned int                    m_data_copy_used_by_others_count;  // user use me for copy
            bool                            m_data_copy_invalidate_asap;  // invalidation is required after copy ends
            
            SharingGroup() : m_dev_mem_obj(NULL), 
                             m_active_users_count(0),
                             m_active_writers_count(0),
                             m_data_copy_in_process_event(NULL), 
                             m_data_copy_state(DATA_COPY_STATE_INVALID),
                             m_data_copy_from_group(MAX_DEVICE_SHARING_GROUP_ID),
                             m_data_copy_used_by_others_count(0),
                             m_data_copy_invalidate_asap(false) {};

            // is this Sharing Group in use for this particular Memory Object?
            bool is_activated( void ) const { return (NULL != m_dev_mem_obj); };

            // is data ready to be used?
            bool is_data_copy_valid( void ) const { return( DATA_COPY_STATE_VALID == m_data_copy_state ); };

            // is data not ready and not in process to become ready?
            bool is_data_copy_invalid( void ) const { return( DATA_COPY_STATE_INVALID == m_data_copy_state ); };

            // is date still not ready but in process of becoming ready?
            bool is_data_in_transition( void ) const { return( !(is_data_copy_valid() || is_data_copy_invalid()) ); };
            
        };

        // Assumption: All devices are added at class initialization time so data may be modified at runtime only
        //             because of lazy sharing groups initialization.
        //             m_global_lock may be taken only 
        //                   - during sharing group lazy initialization
        //                   - sharing finite state machine management
        SharingGroup                        m_sharing_groups[MAX_DEVICE_SHARING_GROUP_ID];

        // parallel structures
        TDevAgentsVector                    m_device_agents;
        TDeviceDescList                     m_device_descriptors;
        TDevice2DescPtrMap                  m_device_2_descriptor_map;

        GenericMemObjectBackingStore*       m_BS;
        unsigned int                        m_active_groups_count; // groups with allocated device objects

        // FSM for data sharing:
        enum DataSharingFSM_State
        {
            UNIQUE_VALID_COPY = 0,                          // either no valid data or owner is unique
            MULTIPLE_VALID_WITH_PARALLEL_WRITERS_HISTORY,   // multiple valid copies, with active writes possibly in parallel
            MULTIPLE_VALID_WITH_SEQUENTIAL_WRITERS_HISTORY, // multiple valid copies, with possible writes but one at a time
            
            DATA_SHARING_FSM_STATES_COUNT   // must be the last - number of different states
        };

        struct DataValidState
        {
            DataSharingFSM_State            m_data_sharing_state;  // current data sharing state
            unsigned int                    m_groups_with_active_users_count;  // how many sgraing groups with active kernels
            unsigned int                    m_groups_with_active_writers_count;
            unsigned int                    m_last_writer_group;
            bool                            m_contains_valid_data; 
            
            DataValidState() :  m_data_sharing_state(UNIQUE_VALID_COPY), 
                                m_groups_with_active_users_count(0), 
                                m_groups_with_active_writers_count(0),
                                m_last_writer_group(MAX_DEVICE_SHARING_GROUP_ID),
                                m_contains_valid_data(false) {};
            DataValidState(const DataValidState& o) : 
                m_data_sharing_state(UNIQUE_VALID_COPY),
                m_groups_with_active_users_count(0),   // there is not active kernels can use this sub-buffer 
                m_groups_with_active_writers_count(0), // during creation
                m_last_writer_group(MAX_DEVICE_SHARING_GROUP_ID),
                m_contains_valid_data(o.m_contains_valid_data) {};

        };

        // FSM machine code, return non-NULL if operation is in-process
        void           data_sharing_set_init_state( bool valid );
        SharedPtr<DataCopyEvent> data_sharing_fsm_process( bool acquire, unsigned int group_id, MemObjUsage access );
        // read path
        SharedPtr<DataCopyEvent> data_sharing_bring_data_to_sharing_group( unsigned int group_id, bool* data_transferred );
        SharedPtr<DataCopyEvent> drive_copy_between_groups( DataCopyState staring_state, 
                                                  unsigned int from_grp_id, 
                                                  unsigned int to_group_id );
        void           invalidate_data_for_group( SharingGroup& group );
        void           ensure_single_data_copy( unsigned int group_id );
        // FSM utilities
        void           acquire_data_sharing_lock();
        SharedPtr<OclEvent>      release_data_sharing_lock( SharedPtr<DataCopyEvent> returned_event );
        class          DataSharingAutoLock;

        DataValidState                      m_data_valid_state;    // overall state - sum of all devices
        OclSpinMutex                        m_global_lock;         // lock for control structures changes

        cl_err_code allocate_object_for_sharing_group( unsigned int group_id );
        const DeviceDescriptor* get_device( ConstSharedPtr<FissionableDevice> dev ) const;

        DeviceDescriptor* get_device( ConstSharedPtr<FissionableDevice> dev )
        { return const_cast<DeviceDescriptor*>( ConstSharedPtr<GenericMemObject>(this)->get_device(dev) ); };

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
		GenericMemObjectSubBuffer(SharedPtr<Context> pContext, cl_mem_object_type clObjType, GenericMemObject& buffer);

        PREPARE_SHARED_PTR(GenericMemObjectSubBuffer);

        static SharedPtr<GenericMemObjectSubBuffer> Allocate(SharedPtr<Context> pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType,
            GenericMemObject& buffer)
        {
            return SharedPtr<GenericMemObjectSubBuffer>(new GenericMemObjectSubBuffer(pContext, pOclEntryPoints, clObjType, buffer));
        }

		cl_err_code Initialize(
			cl_mem_flags		clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int		dim_count,
			const size_t*		dimension,
			const size_t*       pitches,
			void*				pHostPtr
			);


		cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
			const void * buffer_create_info, SharedPtr<MemoryObject>* ppBuffer) {return CL_INVALID_MEM_OBJECT;}

		bool IsSupportedByDevice(SharedPtr<FissionableDevice> pDevice);

        cl_err_code	GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const;

	protected:
        
        GenericMemObjectSubBuffer(SharedPtr<Context> pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType, GenericMemObject& buffer);

        virtual cl_err_code create_device_object( cl_mem_flags clMemFlags,
                                                  SharedPtr<FissionableDevice> dev,
                                                  GenericMemObjectBackingStore* bs,
                                                  IOCLDevMemoryObject** dev_object );

		// do not implement
        GenericMemObjectSubBuffer(const GenericMemObjectSubBuffer&);
        GenericMemObjectSubBuffer& operator=(const GenericMemObjectSubBuffer&);

    private:
        
        const GenericMemObject& m_rBuffer;

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
							   size_t                   alignment,
							   size_t                   preferred_alignment,
							   bool						used_by_DMA,
							   ClHeap					heap,
							   cl_rt_memobj_creation_flags	creation_flags );

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

		size_t                  GetDimCount()    const {return m_dim_count;}
		const size_t*           GetDimentions()  const {return m_dimensions;}
		bool                    IsDataValid()    const {return m_data_valid;}
        void                    SetDataValid(bool value)  { m_data_valid = value; }
		const size_t*           GetPitch()       const {return m_pitches;}
        const cl_image_format&  GetFormat()      const {return m_format;}
        size_t                  GetElementSize() const {return m_element_size;}

        size_t                  GetRequiredAlignment() const {return m_alignment;}

		int AddPendency();
		int RemovePendency();

        // pointer where user should expect the data
        void* GetHostMapPtr( void )  const { return m_pHostPtr ? m_pHostPtr : m_ptr; }
        // same as GetHostMapPtr but NULL if used did not provide this pointer
        void* GetUserProvidedHostMapPtr( void )  const { return m_pHostPtr; }

        // helper function to calculate offsets
        static size_t calculate_offset( size_t elem_size, unsigned int  dim_count,
                                        const size_t  origin[], const size_t  pitches[] );
        static size_t get_element_size( const cl_image_format* format);
        static size_t calculate_size( size_t elem_size, unsigned int  dim_count,
                                        const size_t  dimensions[], const size_t  pitches[] );

	private:
		virtual ~GenericMemObjectBackingStore();

		static void calculate_pitches_and_dimentions( 
									   size_t elem_size, unsigned int  dim_count, 
									   const size_t user_dims[], const size_t user_pitches[],
                                       size_t  dimensions[], size_t  pitches[] );

		void*			m_ptr;

        size_t          m_dim_count;
        size_t          m_dimensions[MAX_WORK_DIM];
		size_t			m_pitches[MAX_WORK_DIM-1];
        cl_image_format m_format;
        size_t          m_element_size;

        void*           m_pHostPtr;
        cl_mem_flags    m_user_flags;

        bool            m_data_valid;
		bool			m_used_by_DMA;
        size_t          m_alignment;
		size_t			m_preferred_alignment;
        size_t          m_raw_data_size;

		ClHeap			m_heap;

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

