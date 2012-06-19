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
#include <assert.h>

#include "cl_logger.h"
#include "GenericMemObj.h"

using namespace std;
using namespace Intel::OpenCL::Framework;

//
// Internal DataCopyEvent
//
class GenericMemObject::DataCopyEvent : public OclEvent
{
public:
    DataCopyEvent(_cl_context_int* context) : OclEvent(context), m_completion_required(false)
    {
        // AddPendency( NULL ); // Why we need this?
		// Do we need add pendency to Context???
        SetEventState(EVENT_STATE_HAS_DEPENDENCIES); 
    };

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
    bool m_completion_required;
    
	virtual ~DataCopyEvent() {};        

	// A MemObjectEvent object cannot be copied
	DataCopyEvent(const DataCopyEvent&);           // copy constructor
	DataCopyEvent& operator=(const DataCopyEvent&);// assignment operator
};

//
// END OF Internal DataCopyEvent
//

//
// Lock managent - reuse global generic mem-object lock
//
inline 
void GenericMemObject::acquire_data_sharing_lock()
{
    m_global_lock.Lock();
}

inline
OclEvent*  GenericMemObject::release_data_sharing_lock( DataCopyEvent* returned_event )
{
    m_global_lock.Unlock();

    if (returned_event && returned_event->IsCompletionRequired())
    {
        // no races here because pointer to the event was removed from Sharing Group before, 
        // so there is no way to get this pointer in more places
        returned_event->SetComplete();
        returned_event = NULL;
    }

    return returned_event;
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
GenericMemObject::DataCopyEvent* GenericMemObject::drive_copy_between_groups( 
                                                  DataCopyState starting_state, 
                                                  unsigned int  from_grp_id,
                                                  unsigned int  to_grp_id )
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
		to.m_data_copy_in_process_event = new DataCopyEvent(GetParentHandle());
        
        assert( NULL != to.m_data_copy_in_process_event );
        if (NULL == to.m_data_copy_in_process_event)
        {
            // error allocating event - assume all is done
            to.m_data_copy_state = DATA_COPY_STATE_INVALID;
        }
    }

    DataCopyEvent* return_event = to.m_data_copy_in_process_event;

    if (need_event_completion && (NULL != return_event))
    {
        to.m_data_copy_in_process_event = NULL;
        
        // complete and release event cannot be done inside lock - need to do this after exit from lock
        // problem - event should be set to NULL inside lock!
        return_event->SetCompletionRequired();
    }

    return return_event;
}
    
//
// Perform data copy from another sharing group
//
GenericMemObject::DataCopyEvent* GenericMemObject::data_sharing_bring_data_to_sharing_group( unsigned int group_id, 
                                                                                             bool* data_transferred  )
{
    *data_transferred = false;
    
    SharingGroup& my_group  = m_sharing_groups[ group_id ];

    if (my_group.m_data_copy_in_process_event)
    {
        // data copy is in process
        return my_group.m_data_copy_in_process_event;
    }

    if (DATA_COPY_STATE_VALID == my_group.m_data_copy_state)
    {
        // sharing group will bring data to my device later during execution
        return NULL;
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
    return drive_copy_between_groups( starting_state, copy_from, group_id );
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
GenericMemObject::DataCopyEvent* GenericMemObject::data_sharing_fsm_process( bool acquire,
                                                                             unsigned int group_id, 
                                                                             MemObjUsage access )
{
    assert( (access >= 0) && (access < MEMOBJ_USAGES_COUNT) && "Wrong MemObjUsage value" );

    DataCopyEvent* copy_in_process_event = NULL;
    
    bool is_read  = (access != WRITE_ENTIRE);
    bool is_write = (access != READ_ONLY);

    bool data_transferred = false;

    // process reading first 
    if (acquire && is_read)
    {
        copy_in_process_event = data_sharing_bring_data_to_sharing_group(group_id, &data_transferred);
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
            if ((WRITERS_INCREASED == writers_change) && (1 == m_data_valid_state.m_groups_with_active_writers_count))
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
            return NULL;
    }


    m_data_valid_state.m_contains_valid_data = true;    
    return copy_in_process_event;
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

        group.m_data_copy_state = valid ? DATA_COPY_STATE_VALID : DATA_COPY_STATE_INVALID;

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

    assert( (0 == group.m_active_users_count) && "Try to invalidate data while active users exist" );

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

//
// Interface implementation
//

// returns NULL if data is ready and locked on given device, 
// non-NULL if data is in the process of copying. Returned event may be added to dependency list
// by the caller
OclEvent* GenericMemObject::LockOnDevice( IN const FissionableDevice* dev, IN MemObjUsage usage )
{
	assert(NULL != dev && "LockOnDevice called without device" );

    if (m_active_groups_count <= 1)
    {
        // single device memory object - nothing to do
        return NULL;
    }

    DeviceDescriptor* desc = get_device( const_cast<FissionableDevice*>(dev) );

    if (NULL == desc)
    {
        assert( NULL != desc && "LockOnDevice called with wrong device" );
        return NULL;
    }

    DataCopyEvent* returned_event = NULL;

    acquire_data_sharing_lock();
    returned_event = data_sharing_fsm_process( true, (unsigned int)desc->m_sharing_group_id, usage ); 
    return release_data_sharing_lock(returned_event);
}

// release data locking on device. 
void GenericMemObject::UnLockOnDevice( IN const FissionableDevice* dev, IN MemObjUsage usage ) 
{
	assert(NULL != dev && "UnLockOnDevice called without device" );

    if (m_active_groups_count <= 1)
    {
        // single device memory object - nothing to do
        return;
    }

    DeviceDescriptor* desc = get_device( const_cast<FissionableDevice*>(dev) );

    if (NULL == desc)
    {
        assert( NULL != desc && "UnLockOnDevice called with wrong device" );
        return;
    }

    DataSharingAutoLock data_sharing_lock( *this );
    data_sharing_fsm_process( false, (unsigned int)desc->m_sharing_group_id, usage ); 
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
    DataCopyEvent* returned_event = NULL;

    if (CL_DEV_FAILED(dev_error))
    {
        LOG_ERROR(TEXT("Device Object returned error 0x%X during asynchronous updating %s Backing Store"), dev_error, 
                            (DATA_COPY_STATE_TO_BS == grp.m_data_copy_state) ? TEXT("to") : TEXT("from") );
    }

    acquire_data_sharing_lock();
    returned_event = drive_copy_between_groups( grp.m_data_copy_state, grp.m_data_copy_from_group, to_grp_id );
    release_data_sharing_lock(returned_event);

}


