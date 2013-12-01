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

///////////////////////////////////////////////////////////////////////////////////////////////////
//  GenericMemObj.cpp
//  Implementation of the MemoryObject Class - Data FSM
//  Created on:      26-Mar-2012
//  Original author: kdmitry
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <Device.h>
#include <algorithm>
#include <assert.h>

#include "cl_logger.h"
#include "GenericMemObj.h"
#include "cl_shared_ptr.hpp"
#include "cl_shutdown.h"

using namespace std;
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

//
// Lock managent - reuse global generic mem-object lock
//
inline 
void GenericMemObject::acquire_data_sharing_lock()
{
    m_global_lock.Lock();
}

inline
SharedPtr<OclEvent>  GenericMemObject::release_data_sharing_lock(DataCopyEventWrapper* returned_event )
{
    m_global_lock.Unlock();

    if (returned_event && returned_event->completionReq)
    {
        assert(NULL != returned_event->ev && "If returned_event->completionReq == true than returned_event->ev cannot be NULL");
        // no races here because pointer to the event was removed from Sharing Group before, 
        // so there is no way to get this pointer in more places
        returned_event->ev->SetEventState(EVENT_STATE_DONE);
        returned_event->ev = NULL;
    }
    
    return (NULL == returned_event) ? (SharedPtr<OclEvent>)NULL : (SharedPtr<OclEvent>)(returned_event->ev);
}

class GenericMemObject::DataSharingAutoLock{
public:
    DataSharingAutoLock( GenericMemObject& container ): m_container(container) 
    { 
        m_container.acquire_data_sharing_lock(); 
    };
    
    ~DataSharingAutoLock() 
    { 
        m_container.release_data_sharing_lock(NULL); 
    };
private:
    GenericMemObject& m_container;
};

//
// Step through copy between groups process
//
void GenericMemObject::drive_copy_between_groups( DataCopyState starting_state, 
                                                  unsigned int  from_grp_id,
                                                  unsigned int  to_grp_id,
                                                  DataCopyEventWrapper* returned_event)
{
    cl_dev_err_code dev_error = CL_DEV_SUCCESS;
    bool            need_event_allocation = false;
    bool            need_event_completion = false;
    
    cl_dev_bs_update_state update_mode;

    SharingGroup& to = m_sharing_groups[ to_grp_id ];

    switch (starting_state)
    {
        case DATA_COPY_STATE_TO_BS:
            {          
                assert( MAX_DEVICE_SHARING_GROUP_ID > from_grp_id );
                if (MAX_DEVICE_SHARING_GROUP_ID <= from_grp_id)
                {
                    return;
                }
                SharingGroup& from = m_sharing_groups[ from_grp_id ];

                dev_error = from.m_dev_mem_obj->clDevMemObjUpdateBackingStore( (void*)to_grp_id, &update_mode );
                assert( CL_DEV_SUCCEEDED(dev_error) && "clDevMemObjUpdateBackingStore() failed" );
                
                if (CL_DEV_FAILED(dev_error))
                {
                    LOG_ERROR(TEXT("Device Object returned error 0x%X during updating to Backing Store"), dev_error);
                    starting_state = DATA_COPY_STATE_INVALID;
                    break;
                }

                starting_state = DATA_COPY_STATE_FROM_BS;

                if (CL_DEV_BS_UPDATE_LAUNCHED == update_mode)
                {
                    // setup lazy copy to BS
                    need_event_allocation = true;

                    // setup the link to the from group
                    to.m_data_copy_from_group = from_grp_id;
                    ++(from.m_data_copy_used_by_others_count);
                    break;
                }
            }
            // fall through

        case DATA_COPY_STATE_FROM_BS:
            m_pBackingStore->SetDataValid(true); // data was copied into BS
            
            dev_error = to.m_dev_mem_obj->clDevMemObjUpdateFromBackingStore( (void*)to_grp_id, &update_mode );
            assert( CL_DEV_SUCCEEDED(dev_error) && "clDevMemObjUpdateFromBackingStore() failed" );
            
            // in parallel to update from BS - finalize lazy copy to BS
            if (MAX_DEVICE_SHARING_GROUP_ID != to.m_data_copy_from_group)
            {
                // lazy copy to BS was performed
                SharingGroup& from = m_sharing_groups[ to.m_data_copy_from_group ];   
                assert( from.m_data_copy_used_by_others_count > 0 );
                --(from.m_data_copy_used_by_others_count);

                if ( from.m_data_copy_invalidate_asap )
                {                   
                    invalidate_data_for_group( from );
                }
                to.m_data_copy_from_group = MAX_DEVICE_SHARING_GROUP_ID;
            }

            // continue normal processing
            if (CL_DEV_FAILED(dev_error))
            {
                LOG_ERROR(TEXT("Device Object returned error 0x%X during updating from Backing Store"), dev_error);
                starting_state = DATA_COPY_STATE_INVALID;
                need_event_completion = true;
                break;
            }

            starting_state = DATA_COPY_STATE_VALID;

            if (CL_DEV_BS_UPDATE_LAUNCHED == update_mode)
            {
                // setup lazy copy from BS
                need_event_allocation = (NULL == to.m_data_copy_in_process_event);
                break;
            }
            // fall through

        case DATA_COPY_STATE_VALID:
            need_event_completion = true;
            break;

        default:
            assert( false && "Invalid state of copy process between sharing groups" );
            need_event_completion = true;
            starting_state = DATA_COPY_STATE_INVALID;
    }

    to.m_data_copy_state = starting_state; // set next state

    assert( ! (need_event_allocation && need_event_completion ));

    if (need_event_allocation)
    {
        // allocate event
        assert( NULL == to.m_data_copy_in_process_event );
        to.m_data_copy_in_process_event = DataCopyEvent::Allocate(GetParentHandle());
        
        assert( NULL != to.m_data_copy_in_process_event );
        if (NULL == to.m_data_copy_in_process_event)
        {
            // error allocating event - assume all is done
            to.m_data_copy_state = DATA_COPY_STATE_INVALID;
        }
    }

    if (returned_event)
    {
        returned_event->ev = to.m_data_copy_in_process_event;

        if (need_event_completion && (NULL != returned_event->ev))
        {
            to.m_data_copy_in_process_event = NULL;
        
            // complete and release event cannot be done inside lock - need to do this after exit from lock
            // problem - event should be set to NULL inside lock!
            assert(false == returned_event->completionReq && "returned_event.completionReq should set to true only once");
            returned_event->completionReq = true;
        }
    }
}
    
//
// Perform data copy from another sharing group
//
void GenericMemObject::data_sharing_bring_data_to_sharing_group( unsigned int group_id, 
                                                                 bool* data_transferred,
                                                                 DataCopyEventWrapper* returned_event)
{
    *data_transferred = false;
    
    SharingGroup& my_group  = m_sharing_groups[ group_id ];

    if (my_group.m_data_copy_in_process_event)
    {
        // data copy is in process
        if (returned_event)
        {
            returned_event->ev = my_group.m_data_copy_in_process_event;
        }
        return;
    }

    if (DATA_COPY_STATE_VALID == my_group.m_data_copy_state)
    {
        // sharing group will bring data to my device later during execution
        return;
    }

    unsigned int copy_from = MAX_DEVICE_SHARING_GROUP_ID;
    DataCopyState starting_state;

    if (m_data_valid_state.m_contains_valid_data)
    {
        for (unsigned int i = 0; i < MAX_DEVICE_SHARING_GROUP_ID; ++i)
        {
            if ((i == group_id) || (!m_sharing_groups[i].is_activated()))
            {
                continue;
            }
            
            if (DATA_COPY_STATE_VALID == m_sharing_groups[i].m_data_copy_state)
            {
                copy_from = i;
                break;
            }
        }

        assert( MAX_DEVICE_SHARING_GROUP_ID != copy_from );
        starting_state = (MAX_DEVICE_SHARING_GROUP_ID == copy_from) ? 
                                            DATA_COPY_STATE_FROM_BS : DATA_COPY_STATE_TO_BS;

        if ((DATA_COPY_STATE_TO_BS == starting_state) && m_pBackingStore->IsDataValid())
        {
            // data in BS already valid - skip copy
            starting_state = DATA_COPY_STATE_FROM_BS;
        }
    }
    else
    {
        starting_state = DATA_COPY_STATE_FROM_BS;
    }

    // start directly from Valid if data should be get from BS, that does not contain valid data
    if ((DATA_COPY_STATE_FROM_BS == starting_state) && (!m_pBackingStore->IsDataValid()))
    {
        starting_state = DATA_COPY_STATE_VALID;
    }

    *data_transferred = true;
    drive_copy_between_groups( starting_state, copy_from, group_id, returned_event);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Data Sharing Finite State Machine is described in the "docs\design\mic\MIC OpenCL Device Agent.pdf" 
// document in the top level documents repository, paragraph 
//   "4.6.3. Memory Object Data Validity in the Multiple Devices Case"
//
// Sharing Groups and Backing Store terms are described in the same document in the paragraph
//   "4.6.2. Sharing memory objects between different devices."
//
//////////////////////////////////////////////////////////////////////////////////////////////////////

//
//  Implement data sharing graph 
//
void GenericMemObject::data_sharing_fsm_process( bool acquire,
                                                 unsigned int group_id, 
                                                 MemObjUsage access, 
                                                 DataCopyEventWrapper* returned_event)
{
    assert( (access >= 0) && (access < MEMOBJ_USAGES_COUNT) && "Wrong MemObjUsage value" );
    
    bool is_read  = (access != WRITE_ENTIRE);
    bool is_write = (access != READ_ONLY);

    bool data_transferred = false;

    // process reading first 
    if (acquire && is_read)
    {
        data_sharing_bring_data_to_sharing_group(group_id, &data_transferred, returned_event);
    }

    enum WritersGroupsChange 
    {
        WRITERS_INCREASED = 0,
        WRITERS_NOT_CHANGED,
        WRITERS_DECREASED
    };

    WritersGroupsChange writers_change = WRITERS_NOT_CHANGED;
    

    // is FSM event occured?
    // FSM event occures when new sharing group is activated or deactivated
    SharingGroup& my_group  = m_sharing_groups[ group_id ];

    // calculate global counters
    if (acquire)
    {
        // acquire
        ++my_group.m_active_users_count;
        assert( 0 != my_group.m_active_users_count );

        if ( 1 == my_group.m_active_users_count )
        {
            ++m_data_valid_state.m_groups_with_active_users_count;
        }

        if (is_write)
        {
            ++my_group.m_active_writers_count;
            assert( 0 != my_group.m_active_users_count );

            if ( 1 == my_group.m_active_writers_count )
            {
                ++m_data_valid_state.m_groups_with_active_writers_count;
                writers_change = WRITERS_INCREASED;
            }
        }

		// In case that the user send in parallel buffers with read / write permission but only read is going to be done, We should invalidate the data of the first request in the group that join to MULTIPLE_VALID_WITH_PARALLEL_WRITERS_HISTORY.
		// if (I'm the first in my sharing group) AND (I didn't get updated data) AND (Joining MULTIPLE_VALID_WITH_PARALLEL_WRITERS_HISTOR)Y AND (not only my sharing group is running now in MULTIPLE_VALID_WITH_PARALLEL_WRITERS_HISTORY state)
		// AND (nobody copies data from me now)
		if ((1 == my_group.m_active_users_count) && (false == data_transferred) && (0 < m_data_valid_state.m_groups_with_active_writers_count) 
			&& (m_data_valid_state.m_groups_with_active_writers_count > my_group.m_active_writers_count) && (false == my_group.m_data_copy_invalidate_asap))
		{
			invalidate_data_for_group(my_group);
			if (false == my_group.m_data_copy_invalidate_asap)
			{
				m_pBackingStore->SetDataValid(false);
				data_sharing_bring_data_to_sharing_group(group_id, &data_transferred, returned_event);
			}
		}

        // remove deferred invalidation if it was requested for target group
        my_group.m_data_copy_invalidate_asap = false;

    }
    else
    {
        // release
        assert( 0 < my_group.m_active_users_count );
        --my_group.m_active_users_count;

        if ( 0 == my_group.m_active_users_count )
        {
            --m_data_valid_state.m_groups_with_active_users_count;
        }
        
        if (is_write)
        {
            assert( 0 < my_group.m_active_writers_count );
            --my_group.m_active_writers_count;

            // save latest writer that finished
            m_data_valid_state.m_last_writer_group = group_id;

            if ( 0 == my_group.m_active_writers_count )
            {
                --m_data_valid_state.m_groups_with_active_writers_count;
                writers_change = WRITERS_DECREASED;
            }
        }
    }

    // ensure data is set as valid if it is invalid (not valid and not in the transition process) until now
    // this may be the case with WRITE_ENTIRE
    if ((WRITERS_INCREASED == writers_change) && my_group.is_data_copy_invalid())
    {
        my_group.m_data_copy_state = DATA_COPY_STATE_VALID;
    }

    assert( m_data_valid_state.m_groups_with_active_writers_count <= m_data_valid_state.m_groups_with_active_users_count );
    
    // switch on current state
    switch (m_data_valid_state.m_data_sharing_state)
    {
        case UNIQUE_VALID_COPY:
            if (data_transferred)
            {
                // increased number of valid states
                // it is not important here were Writers increased now or not changed
                if (1 == m_data_valid_state.m_groups_with_active_writers_count)
                {
                    // data was copied to another group for possibly exclusive modification
                    m_data_valid_state.m_data_sharing_state = MULTIPLE_VALID_WITH_SEQUENTIAL_WRITERS_HISTORY;
                }
                else 
                {
                    // data was copied to another group for reading or parallel writing (if writers > 1)
                    m_data_valid_state.m_data_sharing_state = MULTIPLE_VALID_WITH_PARALLEL_WRITERS_HISTORY;
                }
            }
            else
            {
                // asserts that everything is ok if state was not changed
                assert( m_data_valid_state.m_groups_with_active_users_count <= 1 );
                assert( m_data_valid_state.m_groups_with_active_writers_count <= 1 );
            }

            break;

        case MULTIPLE_VALID_WITH_PARALLEL_WRITERS_HISTORY:
            if ((WRITERS_NOT_CHANGED != writers_change) && (1 == m_data_valid_state.m_groups_with_active_writers_count))
            {
                // unique writer added 
                m_data_valid_state.m_data_sharing_state = MULTIPLE_VALID_WITH_SEQUENTIAL_WRITERS_HISTORY;
            }
            break;

        case MULTIPLE_VALID_WITH_SEQUENTIAL_WRITERS_HISTORY:
            if ((WRITERS_INCREASED == writers_change) && (1 < m_data_valid_state.m_groups_with_active_writers_count))
            {
                // multiple parallel writers 
                m_data_valid_state.m_data_sharing_state = MULTIPLE_VALID_WITH_PARALLEL_WRITERS_HISTORY;
            }
            else if (0 == m_data_valid_state.m_groups_with_active_users_count)
            {
                assert( 0 == m_data_valid_state.m_groups_with_active_writers_count );
                assert( MAX_DEVICE_SHARING_GROUP_ID != m_data_valid_state.m_last_writer_group );

                ensure_single_data_copy( m_data_valid_state.m_last_writer_group );
                m_data_valid_state.m_last_writer_group = MAX_DEVICE_SHARING_GROUP_ID;
                m_data_valid_state.m_data_sharing_state = UNIQUE_VALID_COPY;
            }
            break;

        default:
            assert( false && "Unknown Data Sharing FSM state" );
            if (returned_event)
            {
                returned_event->ev = NULL;
            }
            return;
    }


    m_data_valid_state.m_contains_valid_data = true;
}

// setup initial state at creation
void GenericMemObject::data_sharing_set_init_state( bool valid )
{
    m_data_valid_state.m_data_sharing_state = (valid && (m_active_groups_count > 1)) ?
                            MULTIPLE_VALID_WITH_PARALLEL_WRITERS_HISTORY : UNIQUE_VALID_COPY;

    m_data_valid_state.m_contains_valid_data = valid;

    for (unsigned int i = 0; i < MAX_DEVICE_SHARING_GROUP_ID; ++i)
    {
        SharingGroup& group = m_sharing_groups[i];

        if (!group.is_activated())
        {
            continue;
        }

        if (valid)
        {
            group.m_data_copy_state = DATA_COPY_STATE_VALID;            
        }
        else
        {
// BUGBUG: COI BUG 
//            cl_dev_err_code dev_error = group.m_dev_mem_obj->clDevMemObjInvalidateData();
//            assert( CL_DEV_SUCCEEDED(dev_error) && "initial clDevMemObjInvalidateData() failed" );
//            
//            if (CL_DEV_FAILED(dev_error))
//            {
//                LOG_ERROR(TEXT("Device Object returned error 0x%X during initial invalidation"), dev_error);
//            }                    
            
            group.m_data_copy_state = DATA_COPY_STATE_INVALID;
        }
    }
}

// invalidate group data
void GenericMemObject::invalidate_data_for_group( SharingGroup& group )
{
    if (! group.is_data_copy_valid())
    {
        return;
    }
    
    if (group.m_data_copy_used_by_others_count > 0)
    {
        // cannot invalidate - skip
        group.m_data_copy_invalidate_asap = true;
        return;
    }

    group.m_data_copy_invalidate_asap = false;

    assert( (1 >= group.m_active_users_count) && "Try to invalidate data while active users exist (that is not myself)" );

    cl_dev_err_code dev_error = group.m_dev_mem_obj->clDevMemObjInvalidateData();
    assert( CL_DEV_SUCCEEDED(dev_error) && "clDevMemObjInvalidateData() failed" );
    
    if (CL_DEV_FAILED(dev_error))
    {
        LOG_ERROR(TEXT("Device Object returned error 0x%X during invalidation"), dev_error);
    }                    
    
    group.m_data_copy_state = DATA_COPY_STATE_INVALID;
}

//
//  Implement switch from multi-use mode to single-use mode
//
void GenericMemObject::ensure_single_data_copy( unsigned int group_id )
{
    assert( (group_id < MAX_DEVICE_SHARING_GROUP_ID) && (!m_sharing_groups[group_id].is_data_copy_invalid()) && "ensure_single_data_copy called with remaining group invalid" );

    if (m_data_valid_state.m_contains_valid_data)
    {
        // invalidate sharing groups
        for (unsigned int i = 0; i < MAX_DEVICE_SHARING_GROUP_ID; ++i)
        {
            SharingGroup& o_grp = m_sharing_groups[i];

            if ((i == group_id) || (!o_grp.is_activated()))
            {
                continue;
            }

            invalidate_data_for_group( o_grp );
        }

        // data in BS is invalidated
        m_pBackingStore->SetDataValid(false);             
    }
}

bool GenericMemObject::getDeviceSharingGroupId(const SharedPtr<FissionableDevice>& dev, unsigned int* pSharingGroupId)
{
    assert(NULL != dev && "LockOnDevice called without device" );

    DeviceDescriptor* desc = get_device(dev.GetPtr());

    if (NULL == desc)
    {
        assert( NULL != desc && "LockOnDevice called with wrong device" );
        return false;
    }

    *pSharingGroupId = (unsigned int)desc->m_sharing_group_id;
    return true;
}

//
// Interface implementation
//

cl_err_code GenericMemObject::LockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT MemObjUsage* pOutActuallyUsage, OUT SharedPtr<OclEvent>& pOutEvent )
{
    pOutEvent = NULL;
    // The default value of *pOutActuallyUsage is usage.
    *pOutActuallyUsage = usage;
    if (m_active_groups_count <= 1)
    {
        // single device memory object - nothing to do
        return CL_SUCCESS;
    }

    unsigned int devSharingGroupId = 0;
    bool ret = getDeviceSharingGroupId(dev, &devSharingGroupId);
    if (false == ret)
    {
        return CL_INVALID_VALUE;
    }
    acquireBufferSyncLock();
    if (MEMORY_MODE_OVERLAPPING == getHierarchicalMemoryMode())
    {
        *pOutActuallyUsage = READ_WRITE;
    }
    cl_err_code retCode = updateParent(devSharingGroupId, *pOutActuallyUsage, true, pOutEvent);
    if (NULL == pOutEvent)
    {
        releaseBufferSyncLock();
    }
    return retCode;
}

cl_err_code GenericMemObject::UnLockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage ) 
{ 
    if (m_active_groups_count <= 1)
    {
        // single device memory object - nothing to do
        return CL_SUCCESS;
    }

    unsigned int devSharingGroupId = 0;
    bool ret = getDeviceSharingGroupId(dev, &devSharingGroupId);
    if (false == ret)
    {
        return CL_INVALID_VALUE;
    }
    unLockOnDeviceInt(devSharingGroupId, usage);
    return CL_SUCCESS;
}

// returns NULL if data is ready and locked on given device, 
// non-NULL if data is in the process of copying. Returned event may be added to dependency list
// by the caller
SharedPtr<OclEvent> GenericMemObject::lockOnDeviceInt( IN unsigned int devSharingGroupId, IN MemObjUsage usage, bool alreadyLock )
{
    DataCopyEventWrapper returned_event;

    if (!alreadyLock)
    {
        acquire_data_sharing_lock();
    }
    data_sharing_fsm_process( true, devSharingGroupId, usage, &returned_event ); 
    return release_data_sharing_lock(&returned_event);
}

// release data locking on device. 
void GenericMemObject::unLockOnDeviceInt( IN unsigned int devSharingGroupId, IN MemObjUsage usage )
{
    DataSharingAutoLock data_sharing_lock( *this );
    data_sharing_fsm_process( false, devSharingGroupId, usage, NULL ); 
}

// Device Agent should notify when long update to/from backing store operations finished.
//      Pass HANDLE value that was provided to Device Agent when update API was called
void GenericMemObject::BackingStoreUpdateFinished( IN void* handle, cl_dev_err_code dev_error )
{
    assert( (m_active_groups_count > 1) && "Asynch GenericMemObject::BackingStoreUpdateFinished is called for single-shraing group mem object" ); 
    
    unsigned int to_grp_id = (unsigned int)(size_t)handle;

    assert( (to_grp_id < MAX_DEVICE_SHARING_GROUP_ID) && "Device Agent passed wrong handle back to the GenericMemObject::BackingStoreUpdateFinished");

    if (to_grp_id >= MAX_DEVICE_SHARING_GROUP_ID)
    {
        return;
    }

    SharingGroup&  grp = m_sharing_groups[to_grp_id];
    DataCopyEventWrapper returned_event;

    if (CL_DEV_FAILED(dev_error))
    {
        LOG_ERROR(TEXT("Device Object returned error 0x%X during asynchronous updating %s Backing Store"), dev_error, 
                            (DATA_COPY_STATE_TO_BS == grp.m_data_copy_state) ? TEXT("to") : TEXT("from") );
    }

    acquire_data_sharing_lock();
    drive_copy_between_groups( grp.m_data_copy_state, grp.m_data_copy_from_group, to_grp_id, &returned_event );
    release_data_sharing_lock(&returned_event);
}

cl_err_code GenericMemObject::updateParent(unsigned int devSharingGroupId, MemObjUsage usage, bool isParent, SharedPtr<OclEvent>& pOutEvent)
{
    // Call to updateParentInt with NULL as dataCopyJointEvent and the initial pipeline stage - MOVE_UPDATE_CHILD_LIST_TO_PARENT_DEVICE
    return updateParentInt(devSharingGroupId, usage, isParent, NULL, PARENT_STAGE_MOVE_UPDATE_CHILD_LIST_AND_ZOMBIES_TO_PARENT_DEVICE, pOutEvent);
}

cl_err_code GenericMemObject::updateParentInt(unsigned int destDevSharingGroupId, MemObjUsage usage, bool isParent, SharedPtr<DataCopyJointEvent> dataCopyJointEvent, update_parent_stage stage, SharedPtr<OclEvent>& pOutEvent)
{
    unsigned int parentValidSharingGroupId = MAX_DEVICE_SHARING_GROUP_ID;
    if (dataCopyJointEvent != NULL)
    {
        acquireBufferSyncLock();
        parentValidSharingGroupId = dataCopyJointEvent->getParentValidSharingGroupID();
    }
    else
    {
        // If this is sub-buffer and there is no buffer in m_updateParentList than finished.
        if ((false == isParent) && (false == getUpdateParentFlag()) && (false == isZombie()))
        {
            return CL_SUCCESS;
        }
    }

    TSubBufferList* pSubBuffersList = getSubBuffersListPtr();
    TSubBufferList* pSubBuffersInUpdateProcessList = getSubBuffersInUpdateProcessListPtr();
    vector< SharedPtr<OclEvent> >* pEventOfSubBufferInUpdateProcessList = getEventsOfSubBuffersInUpdateProcessListPtr();

    // If parentValidSharingGroupId is not valid sharing group ID (only in initial call to updateParentInt)
    if (MAX_DEVICE_SHARING_GROUP_ID == parentValidSharingGroupId)
    {
        if ((PARENT_STAGE_MOVE_UPDATE_CHILD_LIST_AND_ZOMBIES_TO_PARENT_DEVICE == stage) && (NULL == dataCopyJointEvent))
        {
            unsigned int tmp_parent_grp = getParentValidSharingGroupIdDuringUpdate();
            unsigned int preferedSharingGroupId = (tmp_parent_grp == MAX_DEVICE_SHARING_GROUP_ID) ? destDevSharingGroupId : tmp_parent_grp;
            SharedPtr<OclEvent> tEvent = NULL;
            // Find sharing group that the parent device is valid, and lock the parent on it.
            getParentMemObj().findValidDeviceAndLock(isParent ? READ_WRITE : READ_WRITE, preferedSharingGroupId, &parentValidSharingGroupId, tEvent);
#ifdef _DEBUG
            if ((MAX_DEVICE_SHARING_GROUP_ID != tmp_parent_grp) && (parentValidSharingGroupId != preferedSharingGroupId))
            {
                assert(0 && "In case that getParentValidSharingGroupIdDuringUpdate() is valid the lock of the parent must be on the same device");
            }
#endif
            setParentValidSharingGroupIdDuringUpdate(parentValidSharingGroupId);
            if (NULL != tEvent)
            {
                releaseBufferSyncLock();
                if (dataCopyJointEvent == NULL)
                {
                    dataCopyJointEvent = DataCopyJointEvent::Allocate(GetParentHandle(), destDevSharingGroupId, usage, isParent, this, parentValidSharingGroupId, pOutEvent);
                    assert(NULL != dataCopyJointEvent && "Allocation of dataCopyJointEvent failed");
                    if (NULL == dataCopyJointEvent)
                    {
                        return CL_OUT_OF_HOST_MEMORY;
                    }
                }
                dataCopyJointEvent->setNextStageToExecute(PARENT_STAGE_MOVE_UPDATE_CHILD_LIST_AND_ZOMBIES_TO_PARENT_DEVICE);
                // dataCopyJointEvent depends on the event that return from 'lockOnDeviceInt()' call
                dataCopyJointEvent->AddDependentOn(tEvent);
                pOutEvent = dataCopyJointEvent;
                return CL_SUCCESS;
                }
        }
#ifdef _DEBUG
        else
        {
            assert(0 && "Only the first stage in update parent pipe should be with MAX_DEVICE_SHARING_GROUP_ID == parentValidSharingGroupId");
        }
#endif
    }

    // Snapshot vector for m_subBuffersList
    TSubBufferList subBuffersListSnapshot;

    switch (stage)
    {
    case PARENT_STAGE_MOVE_UPDATE_CHILD_LIST_AND_ZOMBIES_TO_PARENT_DEVICE:
    {
        vector< SharedPtr<OclEvent> > updateChildEventList;
        // If some buffers are / were in update process AND I called from event...
        if ((pEventOfSubBufferInUpdateProcessList->size() > 0) && (NULL != dataCopyJointEvent))
        {
            updateChildEventList = dataCopyJointEvent->getUpdateChildEventList();
            // Unlock from device buffer that their event in updateChildEventList.
            vector< SharedPtr<OclEvent> >::iterator iter;
            for (int i = updateChildEventList.size() - 1; i >= 0; i--)
            {
                iter = find(pEventOfSubBufferInUpdateProcessList->begin(), pEventOfSubBufferInUpdateProcessList->end(), updateChildEventList[i]);
                if (pEventOfSubBufferInUpdateProcessList->end() != iter)
                {
                    pEventOfSubBufferInUpdateProcessList->erase(iter);
                }
            }
        }

        TSubBufferList* pUpdateParentList = getUpdateParentListPtr();
        // Lock on parent device all the subBuffer in pUpdateParentList
        for (unsigned int i = 0; i < pUpdateParentList->size(); i++)
        {
            // Because we update the previous command, LockOnDevice must be READ_WRITE. (READ_WRITE is a heaviest)
            SharedPtr<OclEvent> tEvent = pUpdateParentList->at(i)->lockOnDeviceInt(parentValidSharingGroupId, READ_WRITE);
            // if it is async operation add tEvent to m_eventOfSubBufferInUpdateProcessList
            if (NULL != tEvent)
            {
                pEventOfSubBufferInUpdateProcessList->push_back(tEvent);
            }
            // add pUpdateParentList->at(i) to m_updateParentInProcessSubBufferList in order to unlock when all the updates will complete.
            pSubBuffersInUpdateProcessList->push_back(pUpdateParentList->at(i));
        }
        // clear the parent update list
        pUpdateParentList->clear();
        bool hasZombieUpdate = (dataCopyJointEvent == NULL) ? false : dataCopyJointEvent->getHasZombieUpdate();
        // Update the parent with the zombies
        for (int i = pSubBuffersList->size() - 1; i >= 0; i--)
        {
            // if the subBuffer m_subBuffersList[i] is not use any more (released and all the related command completed), than update the parent with its content and remove it from m_subBuffersList.
            // Actually the destructor of this subBuffer will call when the update fully complete.
            if (pSubBuffersList->at(i)->isZombie())
            {
                // Because we update the previous command, LockOnDevice must be READ_WRITE. (READ_WRITE is a heaviest)
                SharedPtr<OclEvent> tEvent = pSubBuffersList->at(i)->lockOnDeviceInt(parentValidSharingGroupId, READ_WRITE);
                // if it is async operation add tEvent to m_eventOfSubBufferInUpdateProcessList
                if (NULL != tEvent) 
                {
                    pEventOfSubBufferInUpdateProcessList->push_back(tEvent);
                }
                // add pSubBuffersList->at(i) to m_updateParentInProcessSubBufferList in order to unlock when all the updates will complete.
                pSubBuffersInUpdateProcessList->push_back(pSubBuffersList->at(i));
                // delete the buffer from pSubBuffersList (the destructor will call when the update will complete)
                pSubBuffersList->erase(pSubBuffersList->begin() + i);
                hasZombieUpdate = true;
            }
        }
        // The update is still in process...
        if (pEventOfSubBufferInUpdateProcessList->size() > 0)
        {
            // Must copy it because it can change after releaseBufferSyncLock()
            vector< SharedPtr<OclEvent> > eventsListSnapshot = *pEventOfSubBufferInUpdateProcessList;
            // We need to wait until all the events in pEventOfSubBufferInUpdateProcessList will notify.
            releaseBufferSyncLock();
            if (NULL == dataCopyJointEvent)
            {
                dataCopyJointEvent = DataCopyJointEvent::Allocate(GetParentHandle(), destDevSharingGroupId, usage, isParent, this, parentValidSharingGroupId, pOutEvent);
             }
            assert(NULL != dataCopyJointEvent && "Allocation of dataCopyJointEvent failed");
            if (NULL == dataCopyJointEvent)
            {
                return CL_OUT_OF_HOST_MEMORY;
            }
            dataCopyJointEvent->resetUpdateChildEventList();
            dataCopyJointEvent->setUpdateChildEventList(eventsListSnapshot);
            if (hasZombieUpdate)
            {
                dataCopyJointEvent->setHasZombieUpdate();
            }
            dataCopyJointEvent->setNextStageToExecute(PARENT_STAGE_MOVE_UPDATE_CHILD_LIST_AND_ZOMBIES_TO_PARENT_DEVICE);
            // dataCopyJointEvent depends on all the events that return from 'lockOnDeviceInt()' call
            dataCopyJointEvent->AddDependentOnMulti(eventsListSnapshot.size(), &(eventsListSnapshot[0]));
             pOutEvent = dataCopyJointEvent;
            return CL_SUCCESS;
        }
        else
        {
            // The UPDATE process completed
            // UnlockOnDevice all the subBuffers that updated the parent.
            for (unsigned int i = 0; i < pSubBuffersInUpdateProcessList->size(); i++)
            {
                pSubBuffersInUpdateProcessList->at(i)->unLockOnDeviceInt(parentValidSharingGroupId, READ_WRITE);
            }
            // Update HierarchicalMemoryMode if needed (Do it by one thread only - the first one that reach here [by checking if pSubBuffersInUpdateProcessList->size() > 0])
            bool doUpdateHierarchicalMemoryMode = ((hasZombieUpdate) && (pSubBuffersInUpdateProcessList->size() > 0));
            size_t numOfSubBuffers = pSubBuffersList->size();
            pSubBuffersInUpdateProcessList->clear();
            if (doUpdateHierarchicalMemoryMode || (numOfSubBuffers != pSubBuffersList->size()))
            {
                updateHierarchicalMemoryMode();
            }
            if (NULL != dataCopyJointEvent)
            {
                dataCopyJointEvent->resetUpdateChildEventList();
             }
            setUpdateParentFlag(false);
            // invaldiate parent valid sharing group ID that stored in parent object.
            setParentValidSharingGroupIdDuringUpdate(MAX_DEVICE_SHARING_GROUP_ID);
            if (false == isParent)
            {
                // If 'this' is sub-buffer mem object.
                // Unlock parent
                getParentMemObj().unLockOnDeviceInt(parentValidSharingGroupId, READ_WRITE);
                // If dataCopyJointEvent is NULL than return NULL and the caller will call to LockOnDeviceInt of itself.
                if (NULL == dataCopyJointEvent)
                {
                    return CL_SUCCESS;
                }
                // We trigerred async operation (that already complete) --> we should lock the child buffer in destination device.
                SharedPtr<OclEvent> tEvent = lockOnDeviceInt(destDevSharingGroupId, usage);
                releaseBufferSyncLock();
                if (NULL != tEvent)
                {
                    dataCopyJointEvent->setNextStageToExecute(PARENT_STAGE_FINAL);
                    dataCopyJointEvent->AddDependentOn(tEvent);
                 }
                else
                {
                    dataCopyJointEvent->SetEventState(EVENT_STATE_DONE); 
                }
                pOutEvent = dataCopyJointEvent;
                return CL_SUCCESS;
            }
        }
        // fall through
    }
    case PARENT_STAGE_MOVE_ALL_CHILDS_TO_PARENT_DEVICE:
    {
        assert(true == isParent && "Only parent can reach this stage");
        // Lock on parent valid device all the sub-buffers
        vector< SharedPtr<OclEvent> > eventsList;
        for (unsigned int i = 0; i < pSubBuffersList->size(); i++)
        {
            subBuffersListSnapshot.push_back(pSubBuffersList->at(i));
            SharedPtr<OclEvent> tEvent = pSubBuffersList->at(i)->lockOnDeviceInt(parentValidSharingGroupId, usage);
            if (NULL != tEvent)
            {
                eventsList.push_back(tEvent);
            }
        }
        // If at least one of the sub-buffers lock is async...
        if (eventsList.size() > 0)
        {
            releaseBufferSyncLock();
            if (dataCopyJointEvent == NULL)
            {
                dataCopyJointEvent = DataCopyJointEvent::Allocate(GetParentHandle(), destDevSharingGroupId, usage, isParent, this, parentValidSharingGroupId, pOutEvent);
                assert(NULL != dataCopyJointEvent && "Allocation of dataCopyJointEvent failed");
                if (NULL == dataCopyJointEvent)
                {
                    return CL_OUT_OF_HOST_MEMORY;
                }
            }
            dataCopyJointEvent->setParentChildList(subBuffersListSnapshot);
            dataCopyJointEvent->setNextStageToExecute(PARENT_STAGE_MOVE_PARENT_TO_DESTINATION_DEVICE);
            // dataCopyJointEvent depends on all the events that return from 'lockOnDeviceInt()' call
            dataCopyJointEvent->AddDependentOnMulti(eventsList.size(), &(eventsList[0]));
            pOutEvent = dataCopyJointEvent;
            return CL_SUCCESS;
        }
        // fall through
    }
    case PARENT_STAGE_MOVE_PARENT_TO_DESTINATION_DEVICE:
    {
        assert(true == isParent && "Only parent can reach this stage");
        // this == parent memory object.
        if ((parentValidSharingGroupId != destDevSharingGroupId) || (READ_WRITE != usage))
        {
            unLockOnDeviceInt(parentValidSharingGroupId, READ_WRITE);
            // Lock the parent on destination device.
            SharedPtr<OclEvent> tEvent = lockOnDeviceInt(destDevSharingGroupId, usage);
            if (NULL != tEvent)
            {
                releaseBufferSyncLock();
                if (dataCopyJointEvent == NULL)
                {
                    dataCopyJointEvent = DataCopyJointEvent::Allocate(GetParentHandle(), destDevSharingGroupId, usage, isParent, this, parentValidSharingGroupId, pOutEvent);
                    assert(NULL != dataCopyJointEvent && "Allocation of dataCopyJointEvent failed");
                    if (NULL == dataCopyJointEvent)
                    {
                        return CL_OUT_OF_HOST_MEMORY;
                    }
                    dataCopyJointEvent->setParentChildList(subBuffersListSnapshot);
                }
                dataCopyJointEvent->setNextStageToExecute(PARENT_STAGE_LOCK_CHILDS_ON_DESTINATION_DEVICE);
                // dataCopyJointEvent depends on the event that return from 'lockOnDeviceInt()' call
                dataCopyJointEvent->AddDependentOn(tEvent);
                pOutEvent = dataCopyJointEvent;
                return CL_SUCCESS;
            }
        }
        // fall through
    }
    case PARENT_STAGE_LOCK_CHILDS_ON_DESTINATION_DEVICE:
    {
        assert(true == isParent && "Only parent can reach this stage");
        // this == parent memory object.
        if (parentValidSharingGroupId != destDevSharingGroupId)
        {
            // Unlock all childs from 'parentValidSharingGroupId' device and lock on 'destDevSharingGroupId' (the new location of the parent).
            if ((subBuffersListSnapshot.size() == 0) && (dataCopyJointEvent != NULL))
            {
                subBuffersListSnapshot = dataCopyJointEvent->getParentChildList();
            }
            vector< SharedPtr<OclEvent> > eventsList;
            // for each sub-buffer Unlock from parentValidSharingGroupId and Lock on parent destination sharing group id
            for (unsigned int i = 0; i < subBuffersListSnapshot.size(); i++)
            {
                subBuffersListSnapshot[i]->unLockOnDeviceInt(parentValidSharingGroupId, usage);
                SharedPtr<OclEvent> tEvent = subBuffersListSnapshot[i]->lockOnDeviceInt(destDevSharingGroupId, usage);
                if (NULL != tEvent)
                {
                    eventsList.push_back(tEvent);
                }
            }
            if (eventsList.size() > 0)
            {
                releaseBufferSyncLock();
                if (dataCopyJointEvent == NULL)
                {
                    dataCopyJointEvent = DataCopyJointEvent::Allocate(GetParentHandle(), destDevSharingGroupId, usage, isParent, this, parentValidSharingGroupId, pOutEvent);
                    assert(NULL != dataCopyJointEvent && "Allocation of dataCopyJointEvent failed");
                    if (NULL == dataCopyJointEvent)
                    {
                        return CL_OUT_OF_HOST_MEMORY;
                    }
                    dataCopyJointEvent->setParentChildList(subBuffersListSnapshot);
                }
                dataCopyJointEvent->setNextStageToExecute(PARENT_STAGE_UNLOCK_CHILDS_FROM_DESTINATION_DEVICE);
                // dataCopyJointEvent depends on all the events that return from 'lockOnDeviceInt()' call
                dataCopyJointEvent->AddDependentOnMulti(eventsList.size(), &(eventsList[0]));
                pOutEvent = dataCopyJointEvent;
                return CL_SUCCESS;
            }
        }
        // fall through
    }
    case PARENT_STAGE_UNLOCK_CHILDS_FROM_DESTINATION_DEVICE:
    {
        assert(true == isParent && "Only parent can reach this stage");
        // this == parent memory object.
        // Unlock all childs from parent 'destDevSharingGroupId' device.
        if ((subBuffersListSnapshot.size() == 0) && (dataCopyJointEvent != NULL))
        {
            subBuffersListSnapshot = dataCopyJointEvent->getParentChildList();
        }
        for (unsigned int i = 0; i < subBuffersListSnapshot.size(); i++)
        {
            subBuffersListSnapshot[i]->unLockOnDeviceInt(destDevSharingGroupId, usage);
        }
        // fall through
    }
    case PARENT_STAGE_FINAL:
    {
        // If one of the stages was async than notify the events that we return to command.
        if (NULL != dataCopyJointEvent)
        {
            releaseBufferSyncLock();
            dataCopyJointEvent->SetEventState(EVENT_STATE_DONE);
        }
        break;
    }
    default:
        assert(0 && "Unrecognized stage");
    }

    return CL_SUCCESS;
}

void GenericMemObject::findValidDeviceAndLock(MemObjUsage usage, unsigned int preferedDevice, unsigned int* pDeviceLocked, SharedPtr<OclEvent>& outEvent)
{
    acquire_data_sharing_lock();
    unsigned int validId = MAX_DEVICE_SHARING_GROUP_ID;
    for (unsigned int i = 0; i < MAX_DEVICE_SHARING_GROUP_ID; ++i)
    {
        // if activate and valid
        if ((m_sharing_groups[i].is_activated()) && (! m_sharing_groups[i].is_data_copy_invalid()))
        {
            // if the first device that valid
            if (MAX_DEVICE_SHARING_GROUP_ID == validId)
            {
                validId = i;
            }
            // if current device is 'preferedDevice' than change validId to be the current device.
            if (i == preferedDevice)
            {
                validId = i;
                break;
            }
        }
    }
    // If didn't find valid device, than validId is preferedDevice
    if (MAX_DEVICE_SHARING_GROUP_ID == validId)
    {
        validId = preferedDevice;
    }
    *pDeviceLocked = validId;
    // Lock the buffer on sharing group validId
    outEvent = lockOnDeviceInt(validId, usage, true);
}

void GenericMemObject::unlockOnDeviceAndRemoveFromListInt(TSubBufferList* parentList, unsigned int parentValidSharingGroupId, GenericMemObjectSubBuffer* pMemObj)
{
    // It must unlock with READ_WRITE as the lock.
    pMemObj->unLockOnDeviceInt(parentValidSharingGroupId, READ_WRITE);
    SharedPtr<GenericMemObjectSubBuffer> tSharedPtrMemObj(pMemObj);
    TSubBufferList::iterator it = find(parentList->begin(), parentList->end(), tSharedPtrMemObj);
    if (it != parentList->end())
    {
        parentList->erase(it);
    }
    assert(find(parentList->begin(), parentList->end(), tSharedPtrMemObj) == parentList->end() && "Error probably more than one instance of the same MemObject exist in parentList");
}

void GenericMemObject::addToUpdateList( TSubBufferList* parentList, GenericMemObjectSubBuffer* pMemObj)
{
    SharedPtr<GenericMemObjectSubBuffer> tSharedPtrGenericMemObj(pMemObj);
    // If not exist...
    if (find(parentList->begin(), parentList->end(), tSharedPtrGenericMemObj) == parentList->end())
    {
        parentList->push_back(pMemObj);
        pMemObj->setUpdateParentFlag(true);
    }
}

unsigned int GenericMemObject::getParentValidSharingGroupId() const
{
    GenericMemObject& parent = const_cast<GenericMemObject*>(this)->getParentMemObj();

    // try to search across all groups
    for (unsigned int i = 0; i < MAX_DEVICE_SHARING_GROUP_ID; ++i)
    {
        if (!parent.m_sharing_groups[i].is_activated())
        {
            continue;
        }
        
        if (DATA_COPY_STATE_VALID == parent.m_sharing_groups[i].m_data_copy_state)
        {
            return i;
        }
    }

    // get the first valid
    for (unsigned int i = 0; i < MAX_DEVICE_SHARING_GROUP_ID; ++i)
    {
        if (!parent.m_sharing_groups[i].is_activated())
        {
            continue;
        }
        return i;
    }

    // nothing found ????
    assert( false && "buffer without any device????" );
    return MAX_DEVICE_SHARING_GROUP_ID;
}


////////////////////////////////////////// GenericMemObjectSubBuffer ///////////////////////////////////////////

cl_err_code GenericMemObjectSubBuffer::LockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT MemObjUsage* pOutActuallyUsage, OUT SharedPtr<OclEvent>& pOutEvent )
{
    pOutEvent = NULL;
    // SubBuffers always lock with MemObjUsage = usage
    *pOutActuallyUsage = usage;
    if (m_active_groups_count <= 1)
    {
        // single device memory object - nothing to do
        return CL_SUCCESS;
    }
    unsigned int devSharingGroupId = 0;
    bool ret = getDeviceSharingGroupId(dev, &devSharingGroupId);
    if (false == ret)
    {
        return CL_INVALID_VALUE;
    }
    // Call to update parent first
    SharedPtr<OclEvent> retEvent = NULL;
    acquireBufferSyncLock();
    cl_err_code errCode = updateParent(devSharingGroupId, usage, false, retEvent);
    // If retEvent is NULL it means that the parent updated without calling to async commands so we can continue with locking this buffer on devSharingGroupId.
    // otherwise the updateParent pipe line will call to lockOnDeviceInt of this buffer.
    if ((NULL == retEvent) && (CL_SUCCESS == errCode))
    {
        retEvent = lockOnDeviceInt(devSharingGroupId, usage);
        releaseBufferSyncLock();
    }
    pOutEvent = retEvent;
    return CL_SUCCESS;
}

cl_err_code GenericMemObjectSubBuffer::UnLockOnDevice( IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage )
{
    if (m_active_groups_count <= 1)
    {
        // single device memory object - nothing to do
        return CL_SUCCESS;
    }

    unsigned int devSharingGroupId = 0;
    bool ret = getDeviceSharingGroupId(dev, &devSharingGroupId);
    if (true == ret)
    {
        acquireBufferSyncLock();
        // Copy of UnlockOnDeviceInt
        DataSharingAutoLock data_sharing_lock( *this );
        data_sharing_fsm_process( false, devSharingGroupId, usage, NULL ); 
        // unLockOnDeviceInt
        if (MEMORY_MODE_NORMAL == getHierarchicalMemoryMode())
        {
            releaseBufferSyncLock();
            return CL_SUCCESS; 
        }
        TSubBufferList* pUpdateParentList = getUpdateParentListPtr();
        // If we are in overlap mode, than we shall add this buffer to m_updateParentList in order to update the parent in next memory request.
        if ((MEMORY_MODE_OVERLAPPING == getHierarchicalMemoryMode()) && (!isInUse()))
        {
            addToUpdateList( pUpdateParentList, this );
        }
        releaseBufferSyncLock();
    }
    return CL_SUCCESS;
}

void GenericMemObjectSubBuffer::ZombieFlashToParent()
{
    if (m_active_groups_count <= 1)
    {
        // single device memory object - nothing to do
        return;
    }

    acquireBufferSyncLock();

    if ((getUpdateParentFlag() && isSecondLevelBufferSyncLock()) || (IsShuttingDown()))
    {
        // Ooooops! We entered zombie mode during parent update and from inside thread that already holds 
        //          BufferSyncLock lock. As with current implementation this may happen only because of races between 
        //          parent update and SubBuffer deletion and only at the very end of updating process.
        //          We assume that there is no need in additional parent update, but we need to remove ourselve from
        //          parent sub-buffers list to force real sub-buffer removal
        TSubBufferList* pSubBuffersList = getSubBuffersListPtr();
        SharedPtr<GenericMemObjectSubBuffer> Me = this;
        for (TSubBufferList::iterator it = pSubBuffersList->begin(); it != pSubBuffersList->end(); ++it)
        {
            if ((*it) == Me)
            {
                pSubBuffersList->erase( it );
                break;
            }
        }

        releaseBufferSyncLock();
        return;
    }

    unsigned int devSharingGroupId = getParentValidSharingGroupId();

    assert((MAX_DEVICE_SHARING_GROUP_ID != devSharingGroupId) && "parent does not have valid devices ???" );

    // Call to update parent first
    SharedPtr<OclEvent> retEvent = NULL;

    cl_err_code errCode = updateParent(devSharingGroupId, READ_ONLY, false, retEvent);
    // If retEvent is not NULL it means that asynch operation was launched. We need to wait until it's end.
    // This operation will also loclOnDeviceInt() on our sub-buffer because we are the initializers, so we need to unlock
    if ((NULL != retEvent) && (CL_SUCCESS == errCode))
    {
        retEvent->Wait();
        acquireBufferSyncLock();
        unLockOnDeviceInt( devSharingGroupId, READ_ONLY );
    }
    releaseBufferSyncLock();
}

