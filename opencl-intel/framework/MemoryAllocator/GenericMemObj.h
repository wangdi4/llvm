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
    class GenericMemObjectSubBuffer;

    const unsigned int MAX_DEVICE_SHARING_GROUP_ID =    // how many sharing groups are supported
                                    MAX( (unsigned int)CL_DEV_MAX_BUFFER_SHARING_GROUP_ID,
                                         (unsigned int)CL_DEV_MAX_IMAGE_SHARING_GROUP_ID  );

    /**********************************************************************************************
    * Class name:    GenericMemObject
    *
    * Inherit:        MemoryObject
    * Description:    Represents a memory object that operates with multiple devices with either unified and/or descrete memories
    * Author:        Dmitry Kaptsenel
    * Date:            August 2011
    **********************************************************************************************/
    class GenericMemObject: public MemoryObject, public IOCLDevRTMemObjectService
    {

    friend class DataCopyJointEvent;

    public:        

        PREPARE_SHARED_PTR(GenericMemObject)

        static SharedPtr<GenericMemObject> Allocate(const SharedPtr<Context>& pContext, cl_mem_object_type clObjType)
        {
            return new GenericMemObject(pContext, clObjType);
        }
    
        // MemoryObject methods

        // initialize the data on the memory object
        // initialize the memory object
        virtual cl_err_code Initialize(
            cl_mem_flags            clMemFlags,
            const cl_image_format*    pclImageFormat,
            unsigned int            dim_count,
            const size_t*            dimension,
            const size_t*            pitches,
            void*                    pHostPtr,
            cl_rt_memobj_creation_flags    creation_flags,
            size_t                         force_alignment = 0
            );

        cl_err_code UpdateHostPtr(cl_mem_flags clMemFlags, void* pHostPtr);

        // returns NULL if data is ready and locked on given device, 
        // non-NULL if data is in the process of copying. Returned event may be added to dependency list
        // by the caller
        virtual cl_err_code LockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT MemObjUsage* pOutActuallyUsage, OUT SharedPtr<OclEvent>& pOutEvent );

        // release data locking on device. 
        // MUST pass the same usage value as set in pOutActuallyUsage during LockOnDevice execution.
        virtual cl_err_code UnLockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage );

        cl_err_code CreateDeviceResource(const SharedPtr<FissionableDevice>& pDevice);
        cl_err_code GetDeviceDescriptor(const SharedPtr<FissionableDevice>& pDevice, IOCLDevMemoryObject* *ppDevObject, SharedPtr<OclEvent>* ppEvent);
        cl_err_code UpdateDeviceDescriptor(const SharedPtr<FissionableDevice>& pDevice, IOCLDevMemoryObject* *ppDevObject);

        // return TRUE is device can support this sub-buffer - as for alignment and other requirements.
        // assume that all devices do support all sub-buffer alignments.
        bool IsSupportedByDevice(const SharedPtr<FissionableDevice>& pDevice) { return true; }

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
        void * GetBackingStoreData( const size_t * pszOrigin = nullptr ) const;

        cl_err_code GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const;

        size_t GetPixelSize() const;

        // Get object pitches. If pitch is irrelevant to the memory object, zero pitch is returned
        size_t GetRowPitchSize() const;
        size_t GetSlicePitchSize() const;

        // IDeviceFissionObserver interface
        cl_err_code NotifyDeviceFissioned(const SharedPtr<FissionableDevice>& parent, size_t count, SharedPtr<FissionableDevice>* children);

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

        unsigned int                        m_active_groups_count; // groups with allocated device objects

        typedef vector< SharedPtr<GenericMemObjectSubBuffer> >            TSubBufferList;

        enum hierarchical_memory_mode
        {
            MEMORY_MODE_NORMAL = 0,
            MEMORY_MODE_OVERLAPPING
        };
        
        GenericMemObject(const SharedPtr<Context>& pContext, cl_mem_object_type clObjType);

        // copy all data required for sub-buffer
        cl_err_code InitializeSubObject(  cl_mem_flags        clMemFlags,
                                          GenericMemObject& parent,
                                          const size_t     origin[MAX_WORK_DIM],
                                          const size_t     region[MAX_WORK_DIM] );

        virtual cl_err_code create_device_object( cl_mem_flags clMemFlags,
                                                  const SharedPtr<FissionableDevice>& dev,
                                                  GenericMemObjectBackingStore* bs,
                                                  IOCLDevMemoryObject** dev_object );

        // set the sharing group ID of destDev in pSharingGroupId.
        bool getDeviceSharingGroupId(const SharedPtr<FissionableDevice>& destDev, unsigned int* pSharingGroupId);

        // Sync the parent data with its sub-buffers data in case that 'isParent' == true or 'm_updateParentList' is not empty.
        // devSharingGroupId is the destination device that 'this' memory object should move.
        // usage - the ask usage memory.
        // return SharedPtr<OclEvent> if during the update, one of the operations was async. otherwise return NULL.
        cl_err_code updateParent(unsigned int devSharingGroupId, MemObjUsage usage, bool isParent, SharedPtr<OclEvent>& pOutEvent);

        // returns NULL if data is ready and locked on given device, 
        // non-NULL if data is in the process of copying. Returned event may be added to dependency list
        // by the caller
        // alreadyLock is default false, if true than do NOT acquire_data_sharing_lock, and lockOnDeviceInt will release the lock for the caller.
        SharedPtr<OclEvent> lockOnDeviceInt( unsigned int devSharingGroupId, IN MemObjUsage usage, bool alreadyLock = false );

        // release data locking on device. 
        // MUST pass the same usage value as LockOnDevice
        void unLockOnDeviceInt( unsigned int devSharingGroupId, IN MemObjUsage usage );

        // Lock the parent m_buffersSyncLock mutex. 
        // (use m_buffersSyncLock only by 'acquireBufferSyncLock()' and 'releaseBufferSyncLock()' methods becuase we should use the parent instance for the whole hierarchical memory group).
        inline void acquireBufferSyncLock() { getParentMemObj().m_buffersSyncLock.Lock(); };

        // m_buffersSyncLock allows recurcive lock.
        // return true if locked recursively
        inline bool isSecondLevelBufferSyncLock() { return getParentMemObj().m_buffersSyncLock.lockedRecursively(); }

        // UnLock the parent m_buffersSyncLock mutex. 
        // (use m_buffersSyncLock only by 'acquireBufferSyncLock()' and 'releaseBufferSyncLock()' methods becuase we should use the parent instance for the whole hierarchical memory group).
        inline void releaseBufferSyncLock() { getParentMemObj().m_buffersSyncLock.Unlock(); };

        // Return the hierarchical_memory_mode of this hierarchical memory group. (use m_hierarchicalMemoryMode only by 'getHierarchicalMemoryMode()' and 'setHierarchicalMemoryMode()' methods)
        inline hierarchical_memory_mode getHierarchicalMemoryMode() 
        { 
            assert((((long)(getParentMemObj().m_hierarchicalMemoryMode == MEMORY_MODE_NORMAL)) || ((long)(getParentMemObj().m_hierarchicalMemoryMode == MEMORY_MODE_OVERLAPPING))) && "m_hierarchicalMemoryMode can be MEMORY_MODE_NORMAL or MEMORY_MODE_OVERLAPPING");
            return (hierarchical_memory_mode)(long)(getParentMemObj().m_hierarchicalMemoryMode); 
        };

        // Set the hierarchical_memory_mode of this hierarchical memory group. (use m_hierarchicalMemoryMode only by 'getHierarchicalMemoryMode()' and 'setHierarchicalMemoryMode()' methods)
        inline void setHierarchicalMemoryMode(hierarchical_memory_mode mode) { getParentMemObj().m_hierarchicalMemoryMode.exchange(mode); };

        // Get m_subBuffersList of this hierarchical memory group. (use 'm_subBuffersList' only by getting it from this method.)
        inline TSubBufferList* getSubBuffersListPtr() { return  &(getParentMemObj().m_subBuffersList); };
        
        // Get m_updateParentList of this hierarchical memory group. (use 'm_updateParentList' only by getting it from this method.)
        inline TSubBufferList* getUpdateParentListPtr() { return &(getParentMemObj().m_updateParentStruct.m_updateParentList); };

        // Get m_updateParentInProcessSubBufferList of this hierarchical memory group. (use 'm_updateParentInProcessSubBufferList' only by getting it from this method.)
        inline TSubBufferList* getSubBuffersInUpdateProcessListPtr() { return &(getParentMemObj().m_updateParentStruct.m_updateParentInProcessSubBufferList); };

        // Get m_eventOfSubBufferInUpdateProcessList of this hierarchical memory group. (use 'm_updateParentInProcessSubBufferList' only by getting it from this method.)
        inline vector< SharedPtr<OclEvent> >* getEventsOfSubBuffersInUpdateProcessListPtr() { return &(getParentMemObj().m_updateParentStruct.m_eventOfSubBufferInUpdateProcessList); };

        // Get m_parentValidSharingGroupIdDuringUpdate of this hierarchical memory group. (use 'm_updateParentInProcessSubBufferList' only by getting it from this method.)
        inline unsigned int getParentValidSharingGroupIdDuringUpdate() { return getParentMemObj().m_updateParentStruct.m_parentValidSharingGroupIdDuringUpdate; };

        // Try to find any for given object valid group using different methods. Return MAX_DEVICE_SHARING_GROUP_ID if not found
        unsigned int getParentValidSharingGroupId() const;
        
        // Set m_parentValidSharingGroupIdDuringUpdate of this hierarchical memory group. (use 'm_updateParentInProcessSubBufferList' only by getting it from this method.)
        inline void setParentValidSharingGroupIdDuringUpdate(unsigned int parentValidSharingGroupId) { getParentMemObj().m_updateParentStruct.m_parentValidSharingGroupIdDuringUpdate = parentValidSharingGroupId; };
            
        void addToUpdateList( TSubBufferList* parentList, GenericMemObjectSubBuffer* pMemObj);

        /* Set m_updateParentFlag of the parent to 'updateParent' value.
           if m_updateParentList of the parent is not empty than call it with 'true' otherwise with 'false' */
        inline void setUpdateParentFlag(bool updateParent) 
        { 
            assert(((false == updateParent) || (getParentMemObj().m_updateParentStruct.m_updateParentList.size() > 0)) && "If the list is empty than we should not update the parent");
            getParentMemObj().m_updateParentStruct.m_updateParentFlag.exchange(updateParent); 
        };

        // retun the parent memory object.
        virtual GenericMemObject& getParentMemObj() { return *this; };

        bool isInUse() {return m_data_valid_state.m_groups_with_active_users_count > 0; };
        
        virtual bool IsZombie() const { return false; }

        /* Return true if m_updateParentFlag != 0 --> if m_updateParentList of the parent is not empty. */
        inline bool getUpdateParentFlag() { return (0 != (long)(getParentMemObj().m_updateParentStruct.m_updateParentFlag));  };

    private:

        // The stages in UpdateParent pipeline.
        enum update_parent_stage
        {
            PARENT_STAGE_MOVE_UPDATE_CHILD_LIST_AND_ZOMBIES_TO_PARENT_DEVICE = 0,
            PARENT_STAGE_MOVE_ALL_CHILDS_TO_PARENT_DEVICE,
            PARENT_STAGE_MOVE_PARENT_TO_DESTINATION_DEVICE,
            PARENT_STAGE_LOCK_CHILDS_ON_DESTINATION_DEVICE,
            PARENT_STAGE_UNLOCK_CHILDS_FROM_DESTINATION_DEVICE,
            PARENT_STAGE_FINAL
        };

        //
        // Internal DataCopyEvent
        //
        class DataCopyEvent : public OclEvent
        {
        public:

            PREPARE_SHARED_PTR(DataCopyEvent)

            static SharedPtr<DataCopyEvent> Allocate(_cl_context_int* context)
            {
                return new GenericMemObject::DataCopyEvent(context);
            }

            // Get the return code of the command associated with the event.
            cl_int     GetReturnCode() const {return 0;}
            cl_err_code    GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
                {return CL_INVALID_OPERATION;}

        protected:

            DataCopyEvent(_cl_context_int* context) : OclEvent(context)
            {
                SetEventState(EVENT_STATE_HAS_DEPENDENCIES); 
            };

            virtual ~DataCopyEvent() {};

        private:

            // A MemObjectEvent object cannot be copied
            DataCopyEvent(const DataCopyEvent&);           // copy constructor
            DataCopyEvent& operator=(const DataCopyEvent&);// assignment operator
        };

        //
        // Internal DataCopyJointEvent - Event that manage the pipeline of UpdateParent process.
        //
        class DataCopyJointEvent : public DataCopyEvent
        {
        public:

            PREPARE_SHARED_PTR(DataCopyJointEvent)

            static SharedPtr<DataCopyJointEvent> Allocate(_cl_context_int* context, unsigned int destDevSharingGroupId, MemObjUsage usage, bool isParent, GenericMemObject* pMemObj, unsigned int parentValidGrpId, SharedPtr<OclEvent>& pOutEvent)
            {
                return new GenericMemObject::DataCopyJointEvent(context, destDevSharingGroupId, usage, isParent, pMemObj, parentValidGrpId, pOutEvent);
            }

            // Set the next stage to exexute in UpdateParent pipeline after all the previous stage events will complete.
            inline void setNextStageToExecute(GenericMemObject::update_parent_stage stage)
            {
                m_nextStage = stage;
            }

            // Return the parent valid sharing group ID that set in event creation.
            inline unsigned int getParentValidSharingGroupID()
            {
                return m_parentValidGroupId;
            }

            // Set the updateChildEventList.
            inline void setUpdateChildEventList(const vector< SharedPtr<OclEvent> >& updateChildEventList)
            {
                assert(m_updateParentEventList.size() == 0 && "The size of m_updateParentList must be 0 when calling to setUpdateChildList()");
                m_updateParentEventList = updateChildEventList;
            }

            // Get the updateChildEventList.
            inline vector< SharedPtr<OclEvent> >& getUpdateChildEventList()
            {
                return m_updateParentEventList;
            }

            // Clear the content of m_updateParentEventList
            inline void resetUpdateChildEventList()
            {
                m_updateParentEventList.clear();
            }

            // Set the parentChildList.
            inline void setParentChildList(const TSubBufferList& parentChildList)
            {
                assert(m_subBuffersList.size() == 0 && "The size of m_subBuffersList must be 0 when calling to setParentChildList()");
                m_subBuffersList = parentChildList;
            }

            // Get the parentChildList.
            inline vector< SharedPtr< GenericMemObjectSubBuffer> >& getParentChildList()
            {
                return m_subBuffersList;
            }

            // Set m_hasZombieUpdate to true. (call it if zombie update done)
            inline void setHasZombieUpdate()
            {
                m_hasZombieUpdate = true;
            }

            // Get the m_hasZombieUpdate.
            inline bool getHasZombieUpdate()
            {
                return m_hasZombieUpdate;
            }

        protected:

            // Overwrite OCLEvent method.
            // Will call when all the events that this event dependent on will complete.
            // It call to the next stage of UpdateParent pipeline.
            virtual void DoneWithDependencies(const SharedPtr<OclEvent>& pEvent)
            {
                cl_err_code errCode = CL_SUCCESS;
                errCode = m_pMemObj->updateParentInt(m_destDevSharingGroupId, m_memoryUsage, m_isParent, this, m_nextStage, m_pOutEvent);
                assert(CL_SUCCESS == errCode && "In case that errCode is not CL_SUCCESS ==> invalidate the event //TODO");
                if (EVENT_STATE_DONE == GetEventState())
                {
                    // we are done, no more callbacks from Devices possible
                    m_pOutEvent = nullptr;
                }
            }

        private:

            DataCopyJointEvent(_cl_context_int* context, unsigned int destDevSharingGroupId, MemObjUsage usage, bool isParent, GenericMemObject* pMemObj, unsigned int parentValidGrpId, SharedPtr<OclEvent>& pOutEvent) : DataCopyEvent(context), 
                m_destDevSharingGroupId(destDevSharingGroupId), m_memoryUsage(usage), m_isParent(isParent), m_pMemObj(pMemObj), m_parentValidGroupId(parentValidGrpId), m_hasZombieUpdate(false), m_pOutEvent(pOutEvent)
            {
            };

            virtual ~DataCopyJointEvent() {};

            // A MemObjectEvent object cannot be copied
            DataCopyJointEvent(const DataCopyJointEvent&);           // copy constructor
            DataCopyJointEvent& operator=(const DataCopyJointEvent&);// assignment operator

            unsigned int                            m_destDevSharingGroupId;
            MemObjUsage                             m_memoryUsage;
            bool                                    m_isParent;
            GenericMemObject*                       m_pMemObj;

            GenericMemObject::update_parent_stage   m_nextStage;
            unsigned int                            m_parentValidGroupId;

            vector< SharedPtr<OclEvent> >           m_updateParentEventList;
            TSubBufferList                          m_subBuffersList;

            bool                                    m_hasZombieUpdate;

            SharedPtr<OclEvent>                     m_pOutEvent;
        };

        //
        // END OF Internal DataCopyEvent
        //

        // Low level mapped region creation function
        virtual    cl_err_code    MemObjCreateDevMappedRegion(const SharedPtr<FissionableDevice>&,
            cl_dev_cmd_param_map*    cmd_param_map, void** pHostMapDataPtr);
        // Low level mapped region release function
        virtual    cl_err_code    MemObjReleaseDevMappedRegion(const SharedPtr<FissionableDevice>&,
            cl_dev_cmd_param_map*    cmd_param_map, void* pHostMapDataPtr, bool force_unmap = false );

        struct DeviceDescriptor
        {
            SharedPtr<FissionableDevice>                m_pDevice;
            size_t                                      m_sharing_group_id;
            size_t                                      m_alignment;

            DeviceDescriptor( const SharedPtr<FissionableDevice>& dev, size_t group, size_t alignment ) :
                    m_pDevice(dev), m_sharing_group_id(group), m_alignment(alignment) {};

            DeviceDescriptor( const DeviceDescriptor& o ) :
                   m_pDevice(o.m_pDevice), m_sharing_group_id(o.m_sharing_group_id), m_alignment(o.m_alignment) {};

        };

        typedef std::list<DeviceDescriptor>                            TDeviceDescList;
        typedef std::list<DeviceDescriptor*>                           TDeviceDescPtrList;
        typedef std::map<const FissionableDevice*,DeviceDescriptor*>   TDevice2DescPtrMap;
        typedef std::vector<const IOCLDeviceAgent*>                    TDevAgentsVector;

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
            
            SharingGroup() : m_dev_mem_obj(nullptr), 
                             m_active_users_count(0),
                             m_active_writers_count(0),
                             m_data_copy_in_process_event(nullptr), 
                             m_data_copy_state(DATA_COPY_STATE_INVALID),
                             m_data_copy_from_group(MAX_DEVICE_SHARING_GROUP_ID),
                             m_data_copy_used_by_others_count(0),
                             m_data_copy_invalidate_asap(false) {};

            // is this Sharing Group in use for this particular Memory Object?
            bool is_activated( void ) const { return (nullptr != m_dev_mem_obj); };

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

        struct DataCopyEventWrapper
        {
            DataCopyEventWrapper() : ev(nullptr), completionReq(false) {};
            SharedPtr<DataCopyEvent> ev;
            bool completionReq;
        };

        // FSM machine code, return non-NULL if operation is in-process
        void           data_sharing_set_init_state( bool valid );
        // read path
        void data_sharing_bring_data_to_sharing_group( unsigned int group_id, bool* data_transferred, DataCopyEventWrapper* returned_event );
        void drive_copy_between_groups( DataCopyState staring_state, 
                                                  unsigned int from_grp_id, 
                                                  unsigned int to_group_id,
                                                  DataCopyEventWrapper* returned_event);
        void           invalidate_data_for_group( SharingGroup& group );
        void           ensure_single_data_copy( unsigned int group_id );

    protected:

        void data_sharing_fsm_process( bool acquire, unsigned int group_id, MemObjUsage access, DataCopyEventWrapper* returned_event );
        // FSM utilities
        void           acquire_data_sharing_lock();
        SharedPtr<OclEvent>      release_data_sharing_lock( DataCopyEventWrapper* returned_event );
        class          DataSharingAutoLock;
    private:

        DataValidState                      m_data_valid_state;    // overall state - sum of all devices
        OclNonReentrantSpinMutex            m_global_lock;         // lock for control structures changes

        cl_err_code allocate_object_for_sharing_group( unsigned int group_id );
        const DeviceDescriptor* get_device( const FissionableDevice* dev ) const;

        DeviceDescriptor* get_device( const FissionableDevice* dev )
        { return const_cast<DeviceDescriptor*>( ((const GenericMemObject*)this)->get_device(dev) ); };

        IOCLDevMemoryObject* device_object( const SharingGroup& group )
            { return group.m_dev_mem_obj; };

        const IOCLDevMemoryObject* device_object( const SharingGroup& group ) const
            { return const_cast<GenericMemObject*>(this)->device_object(group); };

        IOCLDevMemoryObject* device_object( const DeviceDescriptor& dev_desc )
            { return device_object( m_sharing_groups[dev_desc.m_sharing_group_id] ); };

        const IOCLDevMemoryObject* device_object( const DeviceDescriptor& dev_desc ) const
            { return const_cast<GenericMemObject*>(this)->device_object(dev_desc); };

        void          remove_device_objects(void);

        // define the current memory mode status (Of all hierarchical group). Use only the parent instance.
        AtomicCounter                                        m_hierarchicalMemoryMode;

        // Vector of all my sub-buffers. Use only the parent instance.
        TSubBufferList                                        m_subBuffersList;

        struct update_parent_struct
        {
            update_parent_struct() : m_parentValidSharingGroupIdDuringUpdate(MAX_DEVICE_SHARING_GROUP_ID), m_updateParentFlag(0) {};

            // Vector of sub buffers that should update the parent before the next memory object request. Use only the parent instance.
            TSubBufferList                    m_updateParentList;

            // Vector of sub buffers that in update parent process (due to update parent of m_updateParentList or zombies). Use only the parent instance.
            TSubBufferList                    m_updateParentInProcessSubBufferList;

            // Vector of DataCopyEvent(s) for each event that create due to update parent of m_updateParentList or zombies. Use only the parent instance.
            vector< SharedPtr<OclEvent> >    m_eventOfSubBufferInUpdateProcessList;

            unsigned int                    m_parentValidSharingGroupIdDuringUpdate;

            AtomicCounter                    m_updateParentFlag;
        };

        update_parent_struct                                m_updateParentStruct;                        

        // Mutex for parent buffer / sub-buffers sync. Use only the parent instance.
        OclSpinMutex                                        m_buffersSyncLock;

        // Call it when new sub-buffer creates.
        // pSubBuffer is the sub-buffer that created.
        // add pSubBuffer to m_subBuffersList and update m_hierarchicalMemoryMode.
        void addSubBuffer(GenericMemObjectSubBuffer* pSubBuffer);

        struct sub_buffer_region
        {
            sub_buffer_region() : m_origin(0), m_size(0)
            {
            }
            sub_buffer_region(size_t origin, size_t size) : m_origin(origin), m_size(size)
            {
            }
            size_t m_origin;
            size_t m_size;
            bool operator() (sub_buffer_region first, sub_buffer_region second) { return (first.m_origin < second.m_origin); };
        };

        // Call it after erasing sub-buffer(s) from m_subBuffersList.
        // It update m_hierarchicalMemoryMode if needed.
        // Assume that the caller own m_buffersSyncLock mutex
        void updateHierarchicalMemoryMode();

        /* The UpdateParent pipeline implementation */
        cl_err_code updateParentInt(unsigned int destDevSharingGroupId, MemObjUsage usage, bool isParent, SharedPtr<DataCopyJointEvent> dataCopyJointEvent, update_parent_stage stage, SharedPtr<OclEvent>& pOutEvent);

        /* If this buffer is valid on preferedDevice than lock on it, otherwise lock on the first device that it valid on, if it is not valid on all devices than lock on 'preferedDevice'.
           esume that the LockOnDevice() operation will not sent async operation because the device is alreay on the device.
           pDeviceLocked will store the index of the sharing group that the device is locked on. 
           Assume that the caller own m_buffersSyncLock mutex */
        void findValidDeviceAndLock(MemObjUsage usage, unsigned int preferedDevice, unsigned int* pDeviceLocked, SharedPtr<OclEvent>& outEvent);

        /* UnlockOnDevice with sharing group ID = parentValidSharingGroupId, memObj and remove it from parent m_updateParentList. 
           Assume that the caller own m_buffersSyncLock mutex. */
        inline void unlockOnDeviceAndRemoveFromUpdateList(unsigned int parentValidSharingGroupId, GenericMemObjectSubBuffer* pMemObj)
        {
            TSubBufferList* tUpdateParentListPtr = getUpdateParentListPtr();
            unlockOnDeviceAndRemoveFromListInt(tUpdateParentListPtr, parentValidSharingGroupId, pMemObj);
            if (0 == tUpdateParentListPtr->size())
            {
                setUpdateParentFlag(false);
            }
        };

        /* UnlockOnDevice with sharing group ID = parentValidSharingGroupId, memObj and remove it from parent list 'parentList'.
           Assume that the caller own m_buffersSyncLock mutex*/
        void unlockOnDeviceAndRemoveFromListInt(TSubBufferList* parentList, unsigned int parentValidSharingGroupId, GenericMemObjectSubBuffer* pMemObj);
    };

    class GenericMemObjectSubBuffer : public GenericMemObject
    {

    friend class GenericMemObject;

    public:

        PREPARE_SHARED_PTR(GenericMemObjectSubBuffer)

        GenericMemObjectSubBuffer(const SharedPtr<Context>& pContext, cl_mem_object_type clObjType, GenericMemObject& buffer);

        static SharedPtr<GenericMemObjectSubBuffer> Allocate(const SharedPtr<Context>& pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType,
            GenericMemObject& buffer)
        {
            return new GenericMemObjectSubBuffer(pContext, pOclEntryPoints, clObjType, buffer);
        }

        cl_err_code Initialize(
            cl_mem_flags        clMemFlags,
            const cl_image_format*    pclImageFormat,
            unsigned int        dim_count,
            const size_t*        dimension,
            const size_t*       pitches,
            void*                pHostPtr
            );


        cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,
            const void * buffer_create_info, SharedPtr<MemoryObject>* ppBuffer) {return CL_INVALID_MEM_OBJECT;}

        bool IsSupportedByDevice(const SharedPtr<FissionableDevice>& pDevice);

        cl_err_code    GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const;

        // returns NULL if data is ready and locked on given device, 
        // non-NULL if data is in the process of copying. Returned event may be added to dependency list by the caller
        // Overwrite parent implementation.
        virtual cl_err_code LockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT MemObjUsage* pOutActuallyUsage, OUT SharedPtr<OclEvent>& pOutEvent );

        // release data locking on device. 
        // MUST pass the same usage value as LockOnDevice
        // Overwrite parent implementation.
        virtual cl_err_code UnLockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage );

        // SubBuffers are saved inside parent buffer list using SmartPointers so we need to manage it's lifetime by ourselve
        virtual void EnterZombieState( EnterZombieStateLevel call_level ); // override ReferenceCountedObject method
        void RegisteredByParent() 
        { 
            IncZombieCnt(); 
            m_bRegisteredByParent = true; 
        }

    protected:
        
        GenericMemObjectSubBuffer(const SharedPtr<Context>& pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType, GenericMemObject& buffer);

        virtual cl_err_code create_device_object( cl_mem_flags clMemFlags,
                                                  const SharedPtr<FissionableDevice>& dev,
                                                  GenericMemObjectBackingStore* bs,
                                                  IOCLDevMemoryObject** dev_object );

        // Return parent memory object.
        virtual GenericMemObject& getParentMemObj() { return (GenericMemObject&)m_rBuffer; };

        // synch me with my parent
        void ZombieFlashToParent();

        // do not implement
        GenericMemObjectSubBuffer(const GenericMemObjectSubBuffer&);
        GenericMemObjectSubBuffer& operator=(const GenericMemObjectSubBuffer&);

    private:

        const GenericMemObject& m_rBuffer; // the same info is returned by MemoryObject::GetParent() but we do this here for optimization
        bool                    m_bRegisteredByParent;
    };

    class GenericMemObjectBackingStore : public IOCLDevBackingStore
    {
    public:
        GenericMemObjectBackingStore(
                               cl_mem_flags                 clMemFlags,
                               const cl_image_format*       pclImageFormat,
                               unsigned int                 dim_count,
                               const size_t*                dimension,
                               const size_t*                pitches,
                               void*                        pHostPtr,
                               size_t                       alignment,
                               size_t                       preferred_alignment,
                               bool                         used_by_DMA,
                               ClHeap                       heap,
                               cl_rt_memobj_creation_flags  creation_flags,
                               IOCLDevRawMemoryAllocator*   pRawMemoryAllocator);

        // for SubObject
        GenericMemObjectBackingStore(
                               cl_mem_flags                clMemFlags,
                               IOCLDevBackingStore*     parent_ps, // used for raw data only
                               const size_t*            origin,
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

        void*            m_ptr;

        size_t          m_dim_count;
        size_t          m_dimensions[MAX_WORK_DIM];
        size_t            m_pitches[MAX_WORK_DIM-1];
        cl_image_format m_format;
        size_t          m_element_size;

        void*           m_pHostPtr;
        cl_mem_flags    m_user_flags;

        bool            m_data_valid;
        bool            m_used_by_DMA;
        size_t          m_alignment;
        size_t            m_preferred_alignment;
        size_t          m_raw_data_size;

        ClHeap                      m_heap;
        IOCLDevRawMemoryAllocator*  m_pRawMemoryAllocator;

        IOCLDevBackingStore* m_parent;
        AtomicCounter    m_refCount;
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
        MEMCPY_S( dimensions, sizeof(size_t)*MAX_WORK_DIM, m_BS->GetDimentions(), sizeof(size_t)*MAX_WORK_DIM );
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

