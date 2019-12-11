// INTEL CONFIDENTIAL
//
// Copyright 2008-2018 Intel Corporation.
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

#include "enqueue_commands.h"
#include "ocl_event.h"
#include "command_queue.h"
#include "kernel.h"
#include "sampler.h"
#include "events_manager.h"
#include "cl_sys_defines.h"
#include "MemoryAllocator/MemoryObject.h"
#include <Logger.h>
#include "context_module.h"
#include "cl_shared_ptr.hpp"
#include "framework_proxy.h"
#include "svm_buffer.h"
#include "usm_buffer.h"

//For debug
#include <stdio.h>
#if defined (_WIN32)
#include <windows.h>
#include <process.h>
#else
#include <malloc.h>
#endif
#include <assert.h>
#include "execution_module.h"
#include "ocl_itt.h"
#include "cl_sys_info.h"

#if defined(USE_ITT)
#if defined(_M_X64)
    #define ITT_SIZE_T_METADATA_TYPE    __itt_metadata_u64
#else
    #define ITT_SIZE_T_METADATA_TYPE    __itt_metadata_u32
#endif
#endif

#define WRITE_MEM_OBJ_ALLOC_ALIGNMENT 128

using namespace Intel::OpenCL::Framework;

using namespace Intel::OpenCL::Utils;

/******************************************************************
 *
 ******************************************************************/
Command::Command( const SharedPtr<IOclCommandQueueBase>& cmdQueue ):
    m_Event(QueueEvent::Allocate(cmdQueue)),
    m_clDevCmdListId(0),
    m_pDevice(nullptr),
    m_pCommandQueue(cmdQueue),
    m_returnCode(CL_SUCCESS),
    m_bIsBeingDeleted(false),
    m_bEventDetached(false),
    m_memory_objects_acquired(false)    
{
    memset(&m_DevCmd, 0, sizeof(cl_dev_cmd_desc));
    m_Event->SetCommand(this);

    assert(m_pCommandQueue);
    m_pDevice = m_pCommandQueue->GetDefaultDevice();

    m_pGpaCommand = nullptr;
    INIT_LOGGER_CLIENT(TEXT("Command Logger Client"),LL_DEBUG);
}

/******************************************************************
 *
 ******************************************************************/
Command::~Command()
{
    m_bIsBeingDeleted = true;
    m_pDevice->GetDeviceAgent()->clDevReleaseCommand(&m_DevCmd);
    m_pDevice = nullptr;
    GPA_DestroyCommand();
    m_pCommandQueue = nullptr;  
    if (m_bEventDetached)
    {
        assert( 0 == m_Event.GetRefCnt() );

        /* m_Event has already been destroyed in DetachEventSharedPtr() - it's deletion triggered the call to ~Command(), 
            so we need to nullify it without decreasing its reference counter, otherwise we would decrement a
            reference counter of an object that had already been destroyed. */
        m_Event.NullifyWithoutDecRefCnt();        
    }
    else
    {
        assert( 1 == m_Event.GetRefCnt() );

        // In this case Command is being deleted explicitly, not through the event, so we need to free the event ourselves.
        m_Event = nullptr;
    }
    assert( (false == m_memory_objects_acquired) && "RelinquishMemoryObjects() was not called!");
    assert( (false == m_memory_objects_acquired) && "RelinquishMemoryObjects() was not called!");
    RELEASE_LOGGER_CLIENT;
}

cl_err_code Command::EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    if (nullptr != cpEeventWaitList && nullptr != pEvent && pEvent >= cpEeventWaitList && pEvent < &cpEeventWaitList[uNumEventsInWaitList])
    {
        // the spec says we should check this, but doesn't actually specify the exact error code
        return CL_INVALID_EVENT;
    }
    // 'this' may disapper during Enqueue if it was successful!
    return GetCommandQueue().StaticCast<IOclCommandQueueBase>()->EnqueueCommand( this, bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent, apiLogger);
}

/******************************************************************
 *
 * Since the command holds the shared pointer to the event that is responsible for its own deletion, 
 * we need to do this here to break this cycle.
 *
 ******************************************************************/
void inline Command::DetachEventSharedPtr()
{
    m_bEventDetached = true;
    m_Event.DecRefCnt();
}

/******************************************************************
 *
 ******************************************************************/
//Todo: remove clCmdId param
cl_err_code Command::NotifyCmdStatusChanged(cl_dev_cmd_id clCmdId, cl_int iCmdStatus, cl_int iCompletionResult, cl_ulong ulTimer)
{
#if defined(USE_ITT)
    ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();

    /// unique ID to pass all tasks, and markers.
    __itt_id ittID;
    if ((nullptr != pGPAData) && (pGPAData->bUseGPA)) 
    {
        ittID = __itt_id_make(&ittID, (unsigned long long)this);
        __itt_id_create(pGPAData->pDeviceDomain, ittID);
#if defined(USE_ITT_INTERNAL)        
        static __thread __itt_string_handle* pTaskName = nullptr;
        if ( nullptr == pTaskName )
        {
          pTaskName = __itt_string_handle_create("Command::NotifyCmdStatusChanged");
        }
        __itt_task_begin(pGPAData->pDeviceDomain, __itt_null, __itt_null, pTaskName);            
#endif
    }
#endif
    cl_err_code res = CL_SUCCESS;
    switch(iCmdStatus)
    {
    case CL_QUEUED:
        // Nothing to do, not expected to be here at all
        assert( 0 && "Unexpected state CL_QUEUED was notified");
        break;

    case CL_SUBMITTED:
        // Nothing to do, not expected to be here at all
        m_Event->SetProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, ulTimer);
        m_Event->SetEventState(EVENT_STATE_ISSUED_TO_DEVICE);
        LogDebugA("Command - SUBMITTED TO DEVICE  : %s (Id: %d)", GetCommandName(), m_Event->GetId());
        break;

    case CL_RUNNING:
        LogDebugA("Command - RUNNING  : %s (Id: %d)", GetCommandName(), m_Event->GetId());
        m_Event->AddProfilerMarker("RUNNING", ITT_SHOW_RUNNING_MARKER);
        if ( m_Event->m_bProfilingEnabled )
        {
            m_Event->SetProfilingInfo(CL_PROFILING_COMMAND_START, ulTimer);
        }
        m_Event->SetEventState(EVENT_STATE_EXECUTING_ON_DEVICE);
        break;

    case CL_ENDED_RUNNING:
        if ( m_Event->m_bProfilingEnabled )
        {
            m_Event->SetProfilingInfo(CL_PROFILING_COMMAND_END, ulTimer);
        }
        m_Event->SetEventState(EVENT_STATE_DONE_EXECUTING_ON_DEVICE);
        break;

    case CL_COMPLETE:
        assert(EVENT_STATE_DONE != m_Event->GetEventState());
        if (m_Event->m_bProfilingEnabled)
        {
            if (m_Event->GetEventState() != EVENT_STATE_DONE_EXECUTING_ON_DEVICE)
            {
                m_Event->SetProfilingInfo(CL_PROFILING_COMMAND_END, ulTimer);
            }
            m_Event->SetProfilingInfo(CL_PROFILING_COMMAND_COMPLETE, ulTimer);
        }      
        
        // Complete command,
        // do that before set event, since side effect of SetEvent(black) may be deleting of this instance.
        // Is error
        if (CL_FAILED(iCompletionResult))
        {
              LogErrorA("Command - DONE - Failure  : %s (Id: %d)", GetCommandName(), m_Event->GetId());
        }
        else
        {
              LogDebugA("Command - DONE - SUCCESS : %s (Id: %d)", GetCommandName(), m_Event->GetId());
        }
        m_returnCode = iCompletionResult;
        res = CommandDone();
        m_Event->SetEventState(EVENT_STATE_DONE);

        m_Event->AddProfilerMarker("COMPLETED", ITT_SHOW_COMPLETED_MARKER);
        DetachEventSharedPtr();    
        
        break;

    default:
        break;
    }

#if defined(USE_ITT) && defined(USE_ITT_INTERNAL)
    if ( (nullptr != pGPAData) && pGPAData->bUseGPA )
    {
      __itt_task_end(pGPAData->pDeviceDomain);
    }
#endif
    
    return res;
}

cl_err_code    Command::GetMemObjectDescriptor(const SharedPtr<MemoryObject>& pMemObj, IOCLDevMemoryObject* *ppMemObj)
{
    if (0 == pMemObj)
    {
        ppMemObj = nullptr;
    }
    else
    {
        SharedPtr<OclEvent> pObjEvent;
        cl_err_code res = pMemObj->GetDeviceDescriptor(m_pDevice, ppMemObj, &pObjEvent);
        if ( CL_FAILED(res) )
        {
            return res;
        }

        // No Event was created, we have the descriptor
        if ( CL_NOT_READY == res )
        {
            assert( 0 != pObjEvent);
            m_Event->AddDependentOn(pObjEvent);
        }
    }
    return CL_SUCCESS;
}

// return CL_SUCCESS if ready and succeeded, CL_NOT_READY if not ready yet and succeeded, other error code in case of error
// arg Must be reference because it content might be change during LockOnDevice.
inline
cl_err_code Command::AcquireSingleMemoryObject( MemoryObjectArg& arg, const SharedPtr<FissionableDevice>& pDev  )
{
    assert( 0 != arg.pMemObj );
    SharedPtr<OclEvent> mem_event = nullptr;
    // initialize arg.access_rights_realy_used to be arg.access_rights (It can change to READ_WRITE during LockOnDevice() operation)
    arg.access_rights_realy_used = arg.access_rights;
    cl_err_code errCode = arg.pMemObj->LockOnDevice( pDev, arg.access_rights, &arg.access_rights_realy_used, mem_event );

    if (CL_SUCCESS != errCode)
    {
        return errCode;
    }

    if (0 != mem_event)
    {
        m_Event->AddDependentOn( mem_event );
    }

    return (0 != mem_event) ? CL_NOT_READY : errCode;
}

cl_err_code Command::AcquireMemoryObjects( MemoryObjectArgList& argList, const SharedPtr<FissionableDevice>& pDev )
{
    if ( m_memory_objects_acquired )
    {
        return CL_SUCCESS;
    }

    m_memory_objects_acquired = true;

    const SharedPtr<FissionableDevice>& targetDevice = (0==pDev)?m_pDevice:pDev;

    cl_err_code retErrCode = CL_SUCCESS;
    cl_err_code errCode = CL_SUCCESS;

    MemoryObjectArgList::iterator it     = argList.begin();
    MemoryObjectArgList::iterator it_end = argList.end();

    for (; it != it_end; ++it )
    {
        MemoryObjectArg& arg = *it;
        errCode = AcquireSingleMemoryObject( arg, targetDevice );
        if ((CL_SUCCESS != errCode) && ((CL_SUCCESS == retErrCode) || (CL_NOT_READY == retErrCode)))
        {
            retErrCode = errCode;
        }
    }

    return retErrCode;
}

void Command::RelinquishMemoryObjects( MemoryObjectArgList& argList, const SharedPtr<FissionableDevice>& pDev )
{
    if ( !m_memory_objects_acquired )
    {
        return;
    } 

    m_memory_objects_acquired = false;
    
    const SharedPtr<FissionableDevice>& targetDevice = (0==pDev)?m_pDevice:pDev;

    MemoryObjectArgList::const_iterator it     = argList.begin();
    MemoryObjectArgList::const_iterator it_end = argList.end();

    for (; it != it_end; ++it )
    {
        const MemoryObjectArg& arg = *it;  
        arg.pMemObj->UnLockOnDevice( targetDevice, arg.access_rights_realy_used ); 
    }
}

inline
void Command::prepare_command_descriptor( cl_dev_cmd_type type, void* params, size_t params_size )
{
    m_DevCmd.id             = (cl_dev_cmd_id)m_Event->GetId(); // event ID is set inside queue, so we cannot save it in constructor
    m_DevCmd.type           = type;
    m_DevCmd.param_size     = params_size;
    m_DevCmd.params         = params;
    m_DevCmd.isFPGAEmulator = m_pCommandQueue->GetContext()->IsFPGAEmulator();

    m_DevCmd.profiling   = (m_pCommandQueue->IsProfilingEnabled() ? true : false );
    m_DevCmd.data         = static_cast<ICmdStatusChangedObserver*>(this);
}

cl_err_code Command::Cancel()
{
    LogDebugA("Command - Cancel for %s (Id: %d)", GetCommandName(), m_Event->GetId());

    NotifyCmdStatusChanged(0, CL_COMPLETE, CL_DEVICE_NOT_AVAILABLE, Intel::OpenCL::Utils::HostTime());

    return CL_SUCCESS;
}

void Command::GPA_InitCommand()
{
#if defined (USE_ITT)
    ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();
    if ((nullptr != pGPAData) && (pGPAData->bUseGPA) && (pGPAData->bEnableContextTracing))
    {
        m_pGpaCommand = nullptr;
        m_pGpaCommand = new ocl_gpa_command();
        if (nullptr != m_pGpaCommand)
        {
            // Create task name strings
            const char* commandName = GPA_GetCommandName();
            if (nullptr != commandName)
            {
                m_pGpaCommand->m_strCmdName = __itt_string_handle_create(commandName);
            }
        }
    }
#endif // ITT
}

void Command::GPA_DestroyCommand()
{
#if defined (USE_ITT)
    ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();
    if ((nullptr != pGPAData) && (pGPAData->bUseGPA) && (pGPAData->bEnableContextTracing))
    {
        // not an error, the internal m_pGpaCommand depends on global pGPAData is active.
        delete m_pGpaCommand;
    }
#endif // ITT
}

cl_dev_cmd_desc* Command::GetDeviceCommandDescriptor()
{
    if ( GetExecutionType() == DEVICE_EXECUTION_TYPE )
        return &m_DevCmd;

    return nullptr;
}
/******************************************************************
 * function to be used by all commands that need to write/read data
 ******************************************************************/
void MemoryCommand::create_dev_cmd_rw(
    cl_uint                uiDimCount,
    void*               pPtr,
    const size_t*       pszMemObjOrigin,
    const size_t*       pszPtrOrigin,
    const size_t*       pszRegion,
    size_t              szPtrRowPitch,
    size_t              szPtrSlicePitch,
    size_t              szMemObjRowPitch,
    size_t              szMemObjSlicePitch,
    cl_dev_cmd_type     clCmdType
    )
{
        // Create Read command
        m_rwParams.ptr = pPtr;

        cl_uint i;
        for( i=0; i<MAX_WORK_DIM; i++ )
        {
            m_rwParams.origin[i] = pszMemObjOrigin[i];
            m_rwParams.region[i] = pszRegion[i];

            if (pszPtrOrigin)
            {
                m_rwParams.ptr_origin[i] = pszPtrOrigin[i];
            }
            else
            {
                m_rwParams.ptr_origin[i] = 0;
            }

        }

        m_rwParams.pitch[0] = szPtrRowPitch;
        m_rwParams.pitch[1] = szPtrSlicePitch;

        m_rwParams.memobj_pitch[0] = szMemObjRowPitch;
        m_rwParams.memobj_pitch[1] = szMemObjSlicePitch;

        m_rwParams.dim_count = uiDimCount;

        prepare_command_descriptor(clCmdType, &m_rwParams, sizeof(cl_dev_cmd_param_rw));
}

/******************************************************************
 *
 ******************************************************************/
CopyMemObjCommand::CopyMemObjCommand(
                  const SharedPtr<IOclCommandQueueBase>& cmdQueue,
                  ocl_entry_points *    pOclEntryPoints,
                        const SharedPtr<MemoryObject>&   pSrcMemObj,
                        const SharedPtr<MemoryObject>&   pDstMemObj,
                        const size_t*   szSrcOrigin,
                        const size_t*   szDstOrigin,
                        const size_t*   szRegion,
                        const size_t    szSrcRowPitch    = 0,
                        const size_t    szSrcSlicePitch = 0,
                        const size_t    szDstRowPitch    = 0,
                        const size_t    szDstSlicePitch    = 0):
    MemoryCommand(cmdQueue),
    m_pSrcMemObj(pSrcMemObj),
    m_pDstMemObj(pDstMemObj),
    m_szSrcRowPitch(szSrcRowPitch),
    m_szSrcSlicePitch(szSrcSlicePitch),
    m_szDstRowPitch(szDstRowPitch),
    m_szDstSlicePitch(szDstSlicePitch)
{
    for( int i=0; i<MAX_WORK_DIM; i++ )
    {
        m_szSrcOrigin[i] = szSrcOrigin[i];
    }

    for( int i=0; i<MAX_WORK_DIM; i++ )
    {
        m_szDstOrigin[i] = szDstOrigin[i];
    }

    for( int i=0; i<MAX_WORK_DIM; i++ )
    {
        m_szRegion[i] = szRegion[i];
    }

    m_uiSrcNumDims = m_pSrcMemObj->GetNumDimensions();
    m_uiDstNumDims = m_pDstMemObj->GetNumDimensions();
}

/******************************************************************
 *
 ******************************************************************/
CopyMemObjCommand::~CopyMemObjCommand()
{
}

/******************************************************************
 * Just mark the memory object as use. The actual copy is determined on execution.
 ******************************************************************/
cl_err_code CopyMemObjCommand::Init()
{
    cl_err_code res = m_pDstMemObj->CreateDeviceResource(m_pDevice);
    if( CL_FAILED(res))
    {
        return res;
    }

    // Initialize GPA data
    GPA_InitCommand();

    bool override_target = m_pDstMemObj->IsWholeObjectCovered(m_uiDstNumDims, m_szDstOrigin, m_szRegion);

    AddToMemoryObjectArgList( m_MemOclObjects, m_pSrcMemObj, MemoryObject::READ_ONLY );
    AddToMemoryObjectArgList( m_MemOclObjects, m_pDstMemObj, override_target ? MemoryObject::WRITE_ENTIRE : MemoryObject::READ_WRITE );

    return CL_SUCCESS;
}

/******************************************************************
 * Copy memory object asks device to perform copy only if both objects are on
 * the same device. Else, it read from 1 device and write to other.
 * Either ways, the location of the destination data remain the same, unless
 * it was never allocated before.
 *
 ******************************************************************/
cl_err_code CopyMemObjCommand::Execute()
{
    cl_err_code res = AcquireMemoryObjects(m_MemOclObjects);
    if ( CL_SUCCESS != res )
    {
        return res;
    }
        

    /// at this phase we know the m_pDstMemObj is valid on target device
    res = CopyOnDevice(m_pDevice);

    return res;
}

/******************************************************************
 * Use device copy command to copy between the buffers.
 * Pre condition for this function is that the 2 buffers are allocated
 * in the device.
 ******************************************************************/
cl_err_code CopyMemObjCommand::CopyOnDevice(const SharedPtr<FissionableDevice>& pDevice)
{
    cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_dev_cmd_param_copy* pCopyParams   = &m_copyParams;

    SharedPtr<OclEvent> pSrcObjEvent;
    cl_err_code clErr = m_pSrcMemObj->GetDeviceDescriptor(pDevice, &(pCopyParams->srcMemObj), &pSrcObjEvent);
    if ( CL_FAILED(clErr) )
    {
        return clErr;
    }
    SharedPtr<OclEvent> pDstObjEvent;
    clErr = m_pDstMemObj->GetDeviceDescriptor(pDevice, &(pCopyParams->dstMemObj), &pDstObjEvent);
    if ( CL_FAILED(clErr) )
    {
        return clErr;
    }

    pCopyParams->src_dim_count  = m_uiSrcNumDims;
    pCopyParams->dst_dim_count  = m_uiDstNumDims;

    pCopyParams->src_pitch[0] = m_szSrcRowPitch;
    pCopyParams->src_pitch[1] = m_szSrcSlicePitch;
    pCopyParams->dst_pitch[0] = m_szDstRowPitch;
    pCopyParams->dst_pitch[1] = m_szDstSlicePitch;

    // set all values
    for( int i=0; i< MAX_WORK_DIM; i++ )
    {
        pCopyParams->src_origin[i]  = m_szSrcOrigin[i];
        pCopyParams->dst_origin[i]  = m_szDstOrigin[i];
        pCopyParams->region[i]      = m_szRegion[i];
    }

    prepare_command_descriptor( CL_DEV_CMD_COPY, pCopyParams, sizeof(cl_dev_cmd_param_copy));

    // Sending 1 command to the device where the buffer is located now
    // Color will be changed only when command is submitted in the device
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event->GetId());
    cl_dev_err_code devErr = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
    return CL_DEV_SUCCEEDED(devErr) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
}

/******************************************************************
 * This function copies the data from the clSrcDeviceId device
 * to the dst buffer local memory.
 *
 ******************************************************************/
cl_err_code CopyMemObjCommand::CommandDone()
{
    RelinquishMemoryObjects(m_MemOclObjects);
    m_MemOclObjects.clear();
    m_pSrcMemObj = nullptr;
    m_pDstMemObj = nullptr;
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferCommand::CopyBufferCommand(
      const SharedPtr<IOclCommandQueueBase>& cmdQueue,
      ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pSrcBuffer,
            const SharedPtr<MemoryObject>&   pDstBuffer,
            const size_t    pszSrcOrigin[3],
            const size_t    pszDstOrigin[3],
            const size_t    pszRegion[3]
            ): CopyMemObjCommand(cmdQueue, pOclEntryPoints, pSrcBuffer, pDstBuffer, pszSrcOrigin, pszDstOrigin, pszRegion)
{
    m_commandType = CL_COMMAND_COPY_BUFFER;
}

/******************************************************************
 *
 ******************************************************************/
 CopyBufferCommand::~CopyBufferCommand()
{
}


 /******************************************************************
 *
 ******************************************************************/
CopyBufferRectCommand::CopyBufferRectCommand(
      const SharedPtr<IOclCommandQueueBase>& cmdQueue,
      ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pSrcBuffer,
            const SharedPtr<MemoryObject>&   pDstBuffer,
            const size_t    pszSrcOrigin[3],
            const size_t    pszDstOrigin[3],
            const size_t    pszRegion[3],
            const size_t    szSrcRowPitch,
            const size_t    szSrcSlicePitch,
            const size_t    szDstRowPitch,
            const size_t    szDstSlicePitch
            ): CopyMemObjCommand(cmdQueue, pOclEntryPoints, pSrcBuffer, pDstBuffer, pszSrcOrigin,
                pszDstOrigin, pszRegion,szSrcRowPitch, szSrcSlicePitch, szDstRowPitch, szDstSlicePitch)
{
    m_uiSrcNumDims = MAX_WORK_DIM;
    m_uiDstNumDims = MAX_WORK_DIM;
    m_commandType = CL_COMMAND_COPY_BUFFER_RECT;
}

/******************************************************************
 *
 ******************************************************************/
 CopyBufferRectCommand::~CopyBufferRectCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
CopyImageCommand::CopyImageCommand(
    const SharedPtr<IOclCommandQueueBase>& cmdQueue,
    ocl_entry_points *    pOclEntryPoints,
    const SharedPtr<MemoryObject>&   pSrcImage,
    const SharedPtr<MemoryObject>&   pDstImage,
    const size_t*   pszSrcOrigin,
    const size_t*   pszDstOrigin,
    const size_t*   pszRegion
    ): CopyMemObjCommand(cmdQueue, pOclEntryPoints, pSrcImage, pDstImage, pszSrcOrigin, pszDstOrigin, pszRegion)
{
    m_commandType = CL_COMMAND_COPY_IMAGE;
    pSrcImage->GetLayout(nullptr, &m_szSrcRowPitch, &m_szSrcSlicePitch);
}

/******************************************************************
 *
 ******************************************************************/
CopyImageCommand::~CopyImageCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferToImageCommand::CopyBufferToImageCommand(
    const SharedPtr<IOclCommandQueueBase>& cmdQueue,
    ocl_entry_points *    pOclEntryPoints,
    const SharedPtr<MemoryObject>&   pSrcBuffer,
    const SharedPtr<MemoryObject>&   pDstImage,
    size_t          pszSrcOffset[3],
    const size_t*   pszDstOrigin,
    const size_t*   pszDstRegion
    ): CopyMemObjCommand(cmdQueue, pOclEntryPoints, pSrcBuffer, pDstImage, pszSrcOffset, pszDstOrigin, pszDstRegion)
{
    m_commandType = CL_COMMAND_COPY_BUFFER_TO_IMAGE;
}

/******************************************************************
 *
 ******************************************************************/
CopyBufferToImageCommand::~CopyBufferToImageCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
CopyImageToBufferCommand::CopyImageToBufferCommand(
    const SharedPtr<IOclCommandQueueBase>& cmdQueue,
    ocl_entry_points *    pOclEntryPoints,
    const SharedPtr<MemoryObject>&   pSrcImage,
    const SharedPtr<MemoryObject>&   pDstBuffer,
    const size_t*   pszSrcOrigin,
    const size_t*   pszSrcRegion,
    size_t          pszDstOffset[3]
    ): CopyMemObjCommand(cmdQueue, pOclEntryPoints, pSrcImage, pDstBuffer, pszSrcOrigin, pszDstOffset, pszSrcRegion)
{
    m_commandType = CL_COMMAND_COPY_IMAGE_TO_BUFFER;
}

/******************************************************************
 *
 ******************************************************************/
CopyImageToBufferCommand::~CopyImageToBufferCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
MapBufferCommand::MapBufferCommand(    const SharedPtr<IOclCommandQueueBase>& cmdQueue, ocl_entry_points *    pOclEntryPoints,
                                    const SharedPtr<MemoryObject>& pBuffer, cl_map_flags clMapFlags, size_t szOffset, size_t szCb):
    MapMemObjCommand(cmdQueue, pOclEntryPoints, pBuffer, clMapFlags, nullptr, nullptr, nullptr, nullptr)
{
    m_szOrigin[0] = szOffset;
    m_szRegion[0] = szCb;
}

/******************************************************************
 *
 ******************************************************************/
MapBufferCommand::~MapBufferCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
MapImageCommand::MapImageCommand(
     const SharedPtr<IOclCommandQueueBase>&  cmdQueue,
     ocl_entry_points *     pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pImage,
            cl_map_flags    clMapFlags,
            const size_t*   pOrigin,
            const size_t*   pRegion,
            size_t*         pszImageRowPitch,
            size_t*         pszImageSlicePitch
            ):
MapMemObjCommand(cmdQueue, pOclEntryPoints, pImage, clMapFlags, pOrigin, pRegion, pszImageRowPitch, pszImageSlicePitch)
{
}

/******************************************************************
 *
 ******************************************************************/
MapImageCommand::~MapImageCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
MapMemObjCommand::MapMemObjCommand(
      const SharedPtr<IOclCommandQueueBase>& cmdQueue,
      ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pMemObj,
            cl_map_flags    clMapFlags,
            const size_t*   pOrigin,
            const size_t*   pRegion,
            size_t*         pszImageRowPitch,
            size_t*         pszImageSlicePitch
            ):
    Command(cmdQueue),
    m_clMapFlags(clMapFlags),
    m_pszImageRowPitch(pszImageRowPitch),
    m_pszImageSlicePitch(pszImageSlicePitch),
    m_pHostDataPtr(nullptr),
    m_pActualMappingDevice(nullptr),
    m_ExecutionType( DEVICE_EXECUTION_TYPE ),
    m_pOclEntryPoints(pOclEntryPoints),
    m_pPostfixCommand(nullptr),
    m_bResourcesAllocated(false)
{
    for( cl_uint i =0; i<MAX_WORK_DIM; i++)
    {
        if ( nullptr != pOrigin )
            m_szOrigin[i] = pOrigin[i];
        else
            m_szOrigin[i] = 0;

        if ( nullptr != pRegion )
            m_szRegion[i] = pRegion[i];
        else
            m_szRegion[i] = 1;
    }

    AddToMemoryObjectArgList( m_MemOclObjects, pMemObj, MemoryObject::READ_WRITE );
}

/******************************************************************
 *
 ******************************************************************/
MapMemObjCommand::~MapMemObjCommand()
{
    if (m_bResourcesAllocated)
    {
        // Init was done, but execute was not called
        if (nullptr != m_pPostfixCommand)
        {
            delete m_pPostfixCommand;
        }
        
        assert(m_MemOclObjects.size() == 1);
        m_MemOclObjects[0].pMemObj->ReleaseMappedRegion( m_pMappedRegion, m_pHostDataPtr );
    }
    
    // In case that the map command run on different device that enqueued than change the device to m_pActualMappingDevice in order to release the command object from the device it created.
    if ((m_pActualMappingDevice != m_pDevice) && (0 != m_pActualMappingDevice))
    {
        m_pDevice = m_pActualMappingDevice;
        m_pActualMappingDevice = nullptr;
    }
}

/******************************************************************
 * On command initilazation a pointer to the mapped region is returned
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::Init()
{
    cl_err_code res;
    assert(m_MemOclObjects.size() == 1);
    const SharedPtr<MemoryObject>& pMemObj = m_MemOclObjects[0].pMemObj;

    res = pMemObj->CreateDeviceResource(m_pDevice);
    if( CL_FAILED(res))
    {
        assert(0);
        return res;
    }

    ConstSharedPtr<FissionableDevice> actual_dev = nullptr;
    
    // Get pointer to the device
    cl_err_code err = pMemObj->CreateMappedRegion(m_pDevice, m_clMapFlags, m_szOrigin, m_szRegion, m_pszImageRowPitch, m_pszImageSlicePitch,
                                                    &m_pMappedRegion, &m_pHostDataPtr, &actual_dev ); 
    if ( CL_FAILED(err) )
    {
        assert(0);
        // Case of error
        return CL_MEM_OBJECT_ALLOCATION_FAILURE;
    }

    m_pActualMappingDevice = const_cast<FissionableDevice*>(actual_dev.GetPtr());
    
    // check whether postfix command should be run to update user mirror area
    if ((0 == (CL_MAP_WRITE_INVALIDATE_REGION & m_pMappedRegion->flags))    &&        // region was not mapped for overriding by host
        (pMemObj->IsSynchDataWithHostRequired( m_pMappedRegion, m_pHostDataPtr )))
    {
        m_pPostfixCommand = new PrePostFixRuntimeCommand( this, PrePostFixRuntimeCommand::POSTFIX_MODE, GetCommandQueue() );

        if (nullptr != m_pPostfixCommand)
        {
            err = m_pPostfixCommand->Init();
            if ( CL_FAILED(err) )
            {
                delete m_pPostfixCommand;
                m_pPostfixCommand = nullptr;
            }
        }
        else
        {
            err = CL_OUT_OF_HOST_MEMORY;
        }

        if (nullptr == m_pPostfixCommand)
        {
            pMemObj->ReleaseMappedRegion( m_pMappedRegion, m_pHostDataPtr );
            assert(0);
            return err;
        }
    }

    if ((CL_MAP_WRITE_INVALIDATE_REGION & m_clMapFlags) && 
        (pMemObj->IsWholeObjectCovered(pMemObj->GetNumDimensions(), m_szOrigin, m_szRegion)))
    {
        assert(m_MemOclObjects.size() == 1);
        m_MemOclObjects[0].access_rights = MemoryObject::WRITE_ENTIRE;
    }

    // Initialize GPA data
    GPA_InitCommand();

    m_bResourcesAllocated = true;
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::Execute()
{
    assert( (RUNTIME_EXECUTION_TYPE != m_ExecutionType) && "MapMemObjCommand::Execute() called after it was switch to RUNTIME mode" );

    if (RUNTIME_EXECUTION_TYPE == m_ExecutionType)
    {
        return CL_NOT_READY;
    }

    cl_err_code res = AcquireMemoryObjects( m_MemOclObjects, m_pActualMappingDevice);
    if ( CL_SUCCESS != res )
    {
        return res;
    }
    
    cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    // Prepare command.
    // Anyhow we send the map command to the device though  we expect that on write
    // there is nothing to do, and on read the device may need to copy from device memory to host memory
    prepare_command_descriptor( CL_DEV_CMD_MAP, m_pMappedRegion, sizeof(cl_dev_cmd_param_map));

    // Change status of the command to Gray before handle by the device
    // Color will be changed only when command is submitted in the device
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event->GetId());
    cl_dev_cmd_list device_cmd_list = m_clDevCmdListId;
    if (m_pActualMappingDevice != m_pDevice)
    {
        // cross device mode
        m_ExecutionType = RUNTIME_EXECUTION_TYPE;
        // use hidden queue if map to another device 
        device_cmd_list = nullptr;
        // ensure we will exist after device will call for completion so that runtime scheduler will see us
        //m_Event->AddPendency(this);
    }
        
    cl_dev_err_code errDev = m_pActualMappingDevice->GetDeviceAgent()->clDevCommandListExecute(
                                        device_cmd_list,
                                        &m_pDevCmd, 1);
    //m_Event->RemovePendency(this);

    cl_err_code err = CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
    
    if ((CL_SUCCESS == err) && (RUNTIME_EXECUTION_TYPE == m_ExecutionType))
    {
        // do not allow command batching until command completion
        err = CL_NOT_READY;
    }
    
    return err;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::CommandDone()
{
    if (m_pPostfixCommand)
    {
        // error enqueue or no enqueue at all
        m_pPostfixCommand->ErrorDone();
        delete m_pPostfixCommand;
        m_pPostfixCommand = nullptr;
    }

    RelinquishMemoryObjects( m_MemOclObjects, m_pActualMappingDevice );
    // Don't remove buffer pendency, the buffer should be alive at least until unmap is done.

    m_bResourcesAllocated = false;
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MapMemObjCommand::EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    // 'this' may disapper during Enqueue if it was successful!
    cl_err_code err = CL_SUCCESS;

    if (m_pPostfixCommand)
    {
        cl_event        intermediate_pEvent;
        EventsManager*    event_manager = GetCommandQueue()->GetEventsManager();

        PrePostFixRuntimeCommand* postfix  = m_pPostfixCommand;
        m_pPostfixCommand = nullptr;    // in the case 'this' will disappear

        // 'this' may disapper after the self-enqueue is successful!
        // First command should be BLOCKING
        err = Command::EnqueueSelf( CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, &intermediate_pEvent, apiLogger);
        if (CL_FAILED(err))
        {
            LogErrorA("Command - Command::EnqueueSelf: %s (Id: %d) failed, Err: %x", GetCommandName(), m_Event->GetId(), err);
            // enqueue unsuccessful - 'this' still alive
            m_pPostfixCommand = postfix; // restore
            return err;
        }

        err = postfix->EnqueueSelf( bBlocking, 1, &intermediate_pEvent, pEvent, apiLogger);
        if (CL_FAILED(err))
        {
            LogErrorA("Command - ostfix->EnqueueSelf: %s (Id: %d) failed, Err: %x", postfix->GetCommandName(), m_Event->GetId(), err);
            // oops, unsuccessfull, but we need to schedule postfix in any case as user need to get back
            // pEvent and be able to make other commands dependent on it
            if (nullptr != pEvent)
            {
                // add manually and leave postfix floating
                postfix->ErrorEnqueue( &intermediate_pEvent, pEvent, err );
            }
            else
            {
                // 'this' may not exist already - remove postfix manually
                postfix->ErrorDone();
                delete postfix;
            }
        }
    
        // release intermediate event
        event_manager->ReleaseEvent( intermediate_pEvent );

        // return success in any case as the original command fired ok
        return CL_SUCCESS;
    }
    else
    {
        // 'this' may disapper after the self-enqueue is successful!
        return Command::EnqueueSelf( bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent, apiLogger);
    }
}


/******************************************************************
 *
 ******************************************************************/
cl_err_code    MapMemObjCommand::PostfixExecute()
{
    cl_err_code err;

#if defined(USE_ITT)

    ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();
    /// unique ID to pass all tasks, and markers.
    __itt_id ittID;
    
    if ((nullptr != pGPAData) && (pGPAData->bUseGPA))
    {
        ittID = __itt_id_make(&ittID, (unsigned long long)this);
        __itt_id_create(pGPAData->pDeviceDomain, ittID);
    
        cl_mem_obj_descriptor*    pMemObj;
        size_t regionInBytes[MAX_WORK_DIM] = {m_pMappedRegion->region[0],m_pMappedRegion->region[1],m_pMappedRegion->region[2]};    

        // Calculate each region size in bytes
        m_pMappedRegion->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pMemObj);
        for (unsigned int i = 0 ; i < m_pMappedRegion->dim_count ; ++i)
        {
            regionInBytes[i] *= pMemObj->uiElementSize;
        }

        char pMarkerString[ITT_TASK_NAME_LEN];
        SPRINTF_S(pMarkerString, ITT_TASK_NAME_LEN, "Sync Data Postfix - %s", GetCommandName());
        __itt_string_handle* pMarker = __itt_string_handle_create(pMarkerString);
        #if defined(USE_GPA)
        // Start Sync Data GPA task
        __itt_set_track(nullptr);
        #endif
        __itt_task_begin(pGPAData->pDeviceDomain, ittID, __itt_null, pMarker);

        // Add region metadata to the Sync Data task
        __itt_metadata_add(pGPAData->pDeviceDomain, ittID, pGPAData->pSizeHandle, ITT_SIZE_T_METADATA_TYPE, m_pMappedRegion->dim_count, regionInBytes);
    }
#endif // ITT
    
    assert(m_MemOclObjects.size() == 1);
    err = m_MemOclObjects[0].pMemObj->SynchDataToHost( m_pMappedRegion, m_pHostDataPtr );

#if defined(USE_ITT)
    if ((nullptr != pGPAData) && (pGPAData->bUseGPA))
    {
        #if defined(USE_GPA)
        // End Sync Data GPA task
        __itt_set_track(nullptr);
        #endif
        __itt_task_end(pGPAData->pDeviceDomain);

        __itt_id_destroy(pGPAData->pDeviceDomain, ittID);
    } 
#endif // ITT

    return err;
}

/******************************************************************
 *
 ******************************************************************/
UnmapMemObjectCommand::UnmapMemObjectCommand(const SharedPtr<IOclCommandQueueBase>& cmdQueue, ocl_entry_points* pOclEntryPoints, 
                                             const SharedPtr<MemoryObject>& pMemObject, void* pMappedPtr):
    Command(cmdQueue),
    m_pMappedPtr(pMappedPtr),
    m_pActualMappingDevice(nullptr),
    m_ExecutionType( DEVICE_EXECUTION_TYPE ),
    m_pPrefixCommand(nullptr),
    m_pOclEntryPoints(pOclEntryPoints),
    m_bResourcesAllocated(false)
{
    AddToMemoryObjectArgList( m_MemOclObjects, pMemObject, MemoryObject::READ_WRITE );
}

/******************************************************************
 *
 ******************************************************************/
UnmapMemObjectCommand::~UnmapMemObjectCommand()
{
    if (m_bResourcesAllocated)
    {
        // Init was done, but execute was not called
        if (nullptr != m_pPrefixCommand)
        {
            delete m_pPrefixCommand;
        }
        
        assert(m_MemOclObjects.size() == 1);
        m_MemOclObjects[0].pMemObj->UndoMappedRegionInvalidation(m_pMappedRegion);
    }

    // In case that the unmap command run on different device that enqueued than change the device to m_pActualMappingDevice in order to release the command object from the device it created.
    if ((m_pActualMappingDevice != m_pDevice) && (0 != m_pActualMappingDevice))
    {
        m_pDevice = m_pActualMappingDevice;
        m_pActualMappingDevice = nullptr;
    }
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::Init()
{
    ConstSharedPtr<FissionableDevice> actual_dev;
    assert(m_MemOclObjects.size() == 1);
    const SharedPtr<MemoryObject>& pMemObj = m_MemOclObjects[0].pMemObj;
    
    /* First check the the region has been mapped - just get the 1st mapped region, the user should
        handle code with multiple map/unmap commands */
    bool bDiscardPreviousData = false;
    cl_err_code err = pMemObj->GetMappedRegionInfo(m_pDevice, m_pMappedPtr, 
                                                   &m_pMappedRegion, &actual_dev, &bDiscardPreviousData, true);

    if (CL_FAILED(err))
    {
        return err;
    }

    if (bDiscardPreviousData)
    {
        assert(m_MemOclObjects.size() == 1);
        m_MemOclObjects[0].access_rights = MemoryObject::WRITE_ENTIRE;
    }
   
    m_pActualMappingDevice = const_cast<FissionableDevice*>(actual_dev.GetPtr());

    // check whether postfix command should be run to update user mirror area
    if ((0 != ((CL_MAP_WRITE|CL_MAP_WRITE_INVALIDATE_REGION) & m_pMappedRegion->flags)) && // region was mapped for writing on host 
        pMemObj->IsSynchDataWithHostRequired( m_pMappedRegion, m_pMappedPtr ))
    {
        m_pPrefixCommand = new PrePostFixRuntimeCommand( this, PrePostFixRuntimeCommand::PREFIX_MODE, GetCommandQueue() );

        if (nullptr != m_pPrefixCommand)
        {
            err = m_pPrefixCommand->Init();
            if ( CL_FAILED(err) )
            {
                delete m_pPrefixCommand;
                m_pPrefixCommand = nullptr;
            }
        }
        else
        {
            err = CL_OUT_OF_HOST_MEMORY;
        }

        if (nullptr == m_pPrefixCommand)
        {
            pMemObj->UndoMappedRegionInvalidation(m_pMappedRegion);
            
            assert(0);
            return err;
        }
    }

    // Initialize GPA data
    GPA_InitCommand();

    m_bResourcesAllocated = true;
    return err;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::Execute()
{
    assert( (RUNTIME_EXECUTION_TYPE != m_ExecutionType) && "UnmapMemObjectCommand::Execute() called after it was switch to RUNTIME mode" );

    if (RUNTIME_EXECUTION_TYPE == m_ExecutionType)
    {
        return CL_NOT_READY;
    }

    cl_err_code res = AcquireMemoryObjects(m_MemOclObjects, m_pActualMappingDevice);
    if ( CL_SUCCESS != res )
    {
        return res;
    }
        
    cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;

    // Create and send unmap command
    prepare_command_descriptor( CL_DEV_CMD_UNMAP, m_pMappedRegion, sizeof(cl_dev_cmd_param_map));

    // Color will be changed only when command is submitted in the device
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event->GetId());

    // we need to synchronize backing store data with host map data, but this
    // should be done immediately before command execution on device to support
    // native kernels that may access mapped object data
    // In order to do this we override Command::NotifyCmdStatusChanged(CL_RUNNING)
    // which is called by device immediately before execution start.
    
    cl_dev_cmd_list device_cmd_list = m_clDevCmdListId;
    if (m_pActualMappingDevice != m_pDevice)
    {
        // cross device mode
        m_ExecutionType = RUNTIME_EXECUTION_TYPE;
        // use hidden queue if map to another device 
        device_cmd_list = nullptr;
        // ensure we will exist after device will call for completion so that runtime scheduler will see us
        //m_Event->AddPendency(this);
    }
        
    cl_dev_err_code errDev = m_pActualMappingDevice->GetDeviceAgent()->clDevCommandListExecute(
                                        device_cmd_list,
                                        &m_pDevCmd, 1);
    
    cl_err_code err = CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;

    if ((CL_SUCCESS == err) && (RUNTIME_EXECUTION_TYPE == m_ExecutionType))
    {
        // do not allow command batching until command completion
        err = CL_NOT_READY;
    }

    return err;    
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code    UnmapMemObjectCommand::PrefixExecute()
{
    cl_err_code err;

#if defined(USE_ITT)
    
    ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();
    /// unique ID to pass all tasks, and markers.
    __itt_id ittID;

    if ((nullptr != pGPAData) && (pGPAData->bUseGPA))
    {
        ittID = __itt_id_make(&ittID, (unsigned long long)this);
        __itt_id_create(pGPAData->pDeviceDomain, ittID);
        cl_mem_obj_descriptor*    pMemObj;
        size_t regionInBytes[MAX_WORK_DIM] = {m_pMappedRegion->region[0],m_pMappedRegion->region[1],m_pMappedRegion->region[2]};    

        // Calculate each region size in bytes
        m_pMappedRegion->memObj->clDevMemObjGetDescriptor(CL_DEVICE_TYPE_CPU, 0, (cl_dev_memobj_handle*)&pMemObj);
        for (unsigned int i = 0 ; i < m_pMappedRegion->dim_count ; ++i)
        {
            regionInBytes[i] *= pMemObj->uiElementSize;
        }

        char pMarkerString[ITT_TASK_NAME_LEN];
        SPRINTF_S(pMarkerString, ITT_TASK_NAME_LEN, "Sync Data Prefix - %s", GetCommandName());
        __itt_string_handle* pMarker = __itt_string_handle_create(pMarkerString);

        #if defined(USE_GPA)
        // Start Sync Data GPA task
        __itt_set_track(nullptr);
        #endif
        __itt_task_begin(pGPAData->pDeviceDomain, ittID, __itt_null, pMarker);

        // Add region metadata to the Sync Data task
        __itt_metadata_add(pGPAData->pDeviceDomain, ittID, pGPAData->pSizeHandle, ITT_SIZE_T_METADATA_TYPE, m_pMappedRegion->dim_count, regionInBytes);
    }
#endif // ITT

    assert(m_MemOclObjects.size() == 1);
    err = m_MemOclObjects[0].pMemObj->SynchDataFromHost( m_pMappedRegion, m_pMappedPtr );

#if defined(USE_ITT)
    if ((nullptr != pGPAData) && (pGPAData->bUseGPA))
    {
        // End Sync Data GPA task
        #if defined(USE_GPA)
        __itt_set_track(nullptr);
        #endif
        __itt_task_end(pGPAData->pDeviceDomain);

        __itt_id_destroy(pGPAData->pDeviceDomain, ittID);
    } 
#endif // ITT

    return err;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::CommandDone()
{
    cl_err_code errVal;

    if (m_pPrefixCommand)
    {
        m_pPrefixCommand->ErrorDone();
        m_pPrefixCommand = nullptr;
    }

    // Here we do the actual operation off releasing the mapped region.
    assert(m_MemOclObjects.size() == 1);
    errVal = m_MemOclObjects[0].pMemObj->ReleaseMappedRegion(m_pMappedRegion, m_pMappedPtr, true);
    m_pMappedRegion = nullptr;

    RelinquishMemoryObjects(m_MemOclObjects, m_pActualMappingDevice);

    m_bResourcesAllocated = false;
    return errVal;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code UnmapMemObjectCommand::EnqueueSelf(cl_bool bBlocking, cl_uint uNumEventsInWaitList, const cl_event* cpEeventWaitList, cl_event* pEvent, ApiLogger* apiLogger)
{
    cl_err_code err;

    if (m_pPrefixCommand)
    {
        cl_event               intermediate_pEvent;
        EventsManager*    event_manager = GetCommandQueue()->GetEventsManager();

        // The first command is always NON-BLOCKING
        err = m_pPrefixCommand->EnqueueSelf( CL_FALSE, uNumEventsInWaitList, cpEeventWaitList, &intermediate_pEvent, apiLogger);
        if (CL_FAILED(err))
        {
            return err;
        }

        // prefix starts it own life
        m_pPrefixCommand = nullptr;

        // 'this' may disapper during Enqueue if it was successful!
        err = Command::EnqueueSelf( bBlocking, 1, &intermediate_pEvent, pEvent, apiLogger);

        // release intermediate event
        event_manager->ReleaseEvent( intermediate_pEvent );
    }
    else
    {
        // 'this' may disapper during Enqueue if it was successful!
        err = Command::EnqueueSelf( bBlocking, uNumEventsInWaitList, cpEeventWaitList, pEvent, apiLogger);
    }

    return err;
}

/******************************************************************
 *
 ******************************************************************/
NativeKernelCommand::NativeKernelCommand(
    const SharedPtr<IOclCommandQueueBase>& cmdQueue,
    ocl_entry_points *    pOclEntryPoints,
    void              (CL_CALLBACK*pUserFnc)(void *),
           void*               pArgs,
           size_t              szCbArgs,
           cl_uint             uNumMemObjects,
           SharedPtr<MemoryObject>*      ppMemObjList,
           const void**        ppArgsMemLoc):
    Command(cmdQueue),
    m_pUserFnc(pUserFnc),
    m_pArgs(pArgs),
    m_szCbArgs(szCbArgs),
    m_uNumMemObjects(uNumMemObjects),
    m_ppMemObjList(ppMemObjList),
    m_ppArgsMemLoc(ppArgsMemLoc)
{
}

/******************************************************************
 *
 ******************************************************************/
NativeKernelCommand::~NativeKernelCommand()
{
}

/******************************************************************
 * On init the command validates the input buffers and creates new args list
 * the contains the device handlers of the buffers.
 ******************************************************************/
cl_err_code NativeKernelCommand::Init()
{
    cl_err_code res = CL_SUCCESS;
    // Create new arg list
    // Expect same size for cl_mem and cl_dev_mem
    STATIC_ASSERT(sizeof(cl_mem) == sizeof(cl_dev_memobj_handle*));
    if (sizeof(cl_mem) != sizeof(cl_dev_memobj_handle*))
    {
        return CL_INVALID_KERNEL_ARGS;
    }
    char*   pNewArgs = new char[m_szCbArgs];
    if(nullptr == pNewArgs)
    {
         return CL_OUT_OF_HOST_MEMORY;
    }

    // Now copy the whole buffer
    MEMCPY_S(pNewArgs, m_szCbArgs, m_pArgs, m_szCbArgs);

    size_t *ppNewArgsOffset = nullptr;
    if (m_uNumMemObjects > 0)
    {
        ppNewArgsOffset = new size_t[m_uNumMemObjects];
        if(nullptr == ppNewArgsOffset)
        {
            delete []pNewArgs;
            return CL_OUT_OF_HOST_MEMORY;
        }
    }

    cl_uint i;
    for( i=0; i < m_uNumMemObjects; i++ )
    {
        // Check that mem object is allocated on device, if not allocate resource
        const SharedPtr<MemoryObject>& pMemObj = m_ppMemObjList[i];
        // Check that mem object is allocated on device, if not allocate resource
        res = pMemObj->CreateDeviceResource(m_pDevice);
        if( CL_FAILED(res))
        {
            break;
        }

        size_t stObjOffset = (size_t)((char*)(m_ppArgsMemLoc[i]) - (char*)m_pArgs);
        cl_dev_memobj_handle* pNewMemObjLocation = (cl_dev_memobj_handle*)(pNewArgs + stObjOffset);

        // Set the new args list
        IOCLDevMemoryObject* clDevMemHndl;
        SharedPtr<OclEvent> pObjEvent;
        res = pMemObj->GetDeviceDescriptor(m_pDevice, &clDevMemHndl, &pObjEvent);
        if( CL_FAILED(res))
        {
            break;
        }
        *((cl_dev_memobj_handle*)pNewMemObjLocation) = clDevMemHndl;
        ppNewArgsOffset[i] = stObjOffset;

        AddToMemoryObjectArgList( m_MemOclObjects, pMemObj, MemoryObject::READ_WRITE );
    }

    // Need to rollback in case of error
    if ( CL_FAILED(res) )
    {
        delete []pNewArgs;
        delete []ppNewArgsOffset;
        return res;
    }

    //
    // Prepare the device command
    //
    cl_dev_cmd_param_native* pNativeKernelParam = &m_nativeParams;

    pNativeKernelParam->args     = m_szCbArgs;
    pNativeKernelParam->argv     = pNewArgs;
    pNativeKernelParam->func_ptr       = m_pUserFnc;
    pNativeKernelParam->mem_num  = m_uNumMemObjects;
    pNativeKernelParam->mem_offset = ppNewArgsOffset;

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NativeKernelCommand::Execute()
{
    cl_err_code res = AcquireMemoryObjects(m_MemOclObjects);
    if ( CL_SUCCESS != res )
    {
        return res;
    }
    
    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event->GetId());

    // Fill command descriptor
    cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    prepare_command_descriptor( CL_DEV_CMD_EXEC_NATIVE, &m_nativeParams, sizeof(cl_dev_cmd_param_native));

    return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NativeKernelCommand::CommandDone()
{
    // Clean resources

    //Can be null of out of memory encountered during init
    if (nullptr != m_nativeParams.argv)
    {
        char* temp = (char*)m_nativeParams.argv;
        delete[] temp;
    }
    if (nullptr != m_nativeParams.mem_offset)
    {
        delete[] m_nativeParams.mem_offset;
    }

    RelinquishMemoryObjects(m_MemOclObjects);
    delete []m_ppMemObjList;

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
NDRangeKernelCommand::NDRangeKernelCommand(
    const SharedPtr<IOclCommandQueueBase>& cmdQueue,
    ocl_entry_points*     pOclEntryPoints,
    const SharedPtr<Kernel>&   pKernel,
    cl_uint         uiWorkDim,
    const size_t*   cpszGlobalWorkOffset,
    const size_t*   cpszGlobalWorkSize,
    const size_t*   cpszLocalWorkSize
    ):
Command(cmdQueue),
m_pKernel(pKernel),
m_pDeviceKernel(nullptr),
m_uiWorkDim(uiWorkDim),
m_cpszGlobalWorkOffset(cpszGlobalWorkOffset),
m_cpszGlobalWorkSize(cpszGlobalWorkSize),
m_cpszLocalWorkSize(cpszLocalWorkSize)
{
}

/******************************************************************
 *
 ******************************************************************/
NDRangeKernelCommand::~NDRangeKernelCommand()
{
}

/******************************************************************
 *
 ******************************************************************/

cl_err_code NDRangeKernelCommand::Init()
{
    cl_err_code res = CL_SUCCESS;
    // We have to use init to create a snapshot of the buffer kernels on enqueue
    // Thus, we also create and set the device command appropriately as much as we can.

    // CL_INVALID_PROGRAM_EXECUTABLE if there is no successfully built program
    // executable available for device associated with command_queue.
    m_pDeviceKernel = m_pKernel->GetDeviceKernel( m_pDevice.GetPtr() );
    // CL_INVALID_PROGRAM_EXECUTABLE if there is no successfully built program
    // executable available for device associated with command_queue.
    if (nullptr == m_pDeviceKernel)
    {
        return CL_INVALID_PROGRAM_EXECUTABLE;
    }

    // Create args snapshot
    const KernelArg* pArg    = nullptr;
    size_t stTotalLocalSize  = m_pKernel->GetTotalLocalSize();

    cl_device_svm_capabilities svmCaps;
    bool svmSupported = GetDevice()->GetSVMCapabilities( &svmCaps );
    if ( svmSupported )
    {
        bool bFineGrain = m_pKernel->IsSvmFineGrainSystem();
        if ( bFineGrain && !(svmCaps & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM) )
            return CL_INVALID_OPERATION;

        if ( !bFineGrain )
        {
            size_t nonArgSvmBuffersCount = m_pKernel->GetNonArgSvmBuffersCount();
            if (nonArgSvmBuffersCount > 0)
            {
                std::vector<SharedPtr<SVMBuffer> > nonArgSvmBufs;
                m_pKernel->GetNonArgSvmBuffers(nonArgSvmBufs);
                if (nonArgSvmBufs.size() > 0)
                {
                    m_nonArgSvmBuffersVec.resize(nonArgSvmBufs.size());
                    for (size_t i = 0; i < nonArgSvmBufs.size(); i++)
                    {
                        AddToMemoryObjectArgList( m_MemOclObjects, nonArgSvmBufs[i], MemoryObject::READ_WRITE);
                        res = GetMemObjectDescriptor(nonArgSvmBufs[i], &m_nonArgSvmBuffersVec[i]);
                        if (CL_FAILED(res))
                        {
                            return res;
                        }
                    }
                }
            }
        }
    }

    // Indirect USM allocations that are not passed as kernel arguments
    std::vector<SharedPtr<USMBuffer> > nonArgUsmBufs;
    m_pKernel->GetNonArgUsmBuffers(nonArgUsmBufs);
    if (!nonArgUsmBufs.empty())
    {
        cl_unified_shared_memory_capabilities_intel hostCaps =
            GetDevice()->GetUSMCapabilities(
            CL_DEVICE_HOST_MEM_CAPABILITIES_INTEL);
        cl_unified_shared_memory_capabilities_intel deviceCaps =
            GetDevice()->GetUSMCapabilities(
            CL_DEVICE_DEVICE_MEM_CAPABILITIES_INTEL);
        cl_unified_shared_memory_capabilities_intel sharedSingleCaps =
            GetDevice()->GetUSMCapabilities(
            CL_DEVICE_SINGLE_DEVICE_SHARED_MEM_CAPABILITIES_INTEL);
        bool hostSupported = hostCaps & CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL;
        bool deviceSupported = deviceCaps &
                               CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL;
        bool sharedSupported =
            sharedSingleCaps & CL_UNIFIED_SHARED_MEMORY_ACCESS_INTEL;
        m_nonArgUsmBuffersVec.resize(nonArgUsmBufs.size());
        for (size_t i = 0; i < nonArgUsmBufs.size(); i++)
        {
            SharedPtr<USMBuffer> &buf = nonArgUsmBufs[i];
            cl_unified_shared_memory_type_intel type = buf->GetType();
            // filter out invalid combinations
            switch (type)
            {
            case CL_MEM_TYPE_HOST_INTEL:
                if (!hostSupported || !m_pKernel->IsUsmIndirectHost())
                    return CL_INVALID_OPERATION;
                break;
            case CL_MEM_TYPE_DEVICE_INTEL:
                if (!deviceSupported || !m_pKernel->IsUsmIndirectDevice())
                    return CL_INVALID_OPERATION;
                break;
            case CL_MEM_TYPE_SHARED_INTEL:
                if (!sharedSupported || !m_pKernel->IsUsmIndirectShared())
                    return CL_INVALID_OPERATION;
                break;
            default:
                break;
            }
            AddToMemoryObjectArgList(m_MemOclObjects, buf,
                                     MemoryObject::READ_WRITE);
            res = GetMemObjectDescriptor(buf, &m_nonArgUsmBuffersVec[i]);
            if (CL_FAILED(res))
                return res;
        }
    }

    //
    // Query kernel info to validate input params
    //
    size_t szCompiledWorkGroupMaxSize = m_pDeviceKernel->GetKernelWorkGroupSize();
    if( szCompiledWorkGroupMaxSize == 0 )
    {
      // Kernel cannot run if its max. possible work-group size is zero.
      return CL_OUT_OF_RESOURCES;
    }

    const size_t* szCompliedWorkGroupSize = m_pDeviceKernel->GetKernelCompileWorkGroupSize();

    // If the work-group size is not specified in kernel using the above attribute qualifier (0, 0,0)
    // is returned in szComplieWorkGroupSize
    if( ! ( (0 == szCompliedWorkGroupSize[0]) &&
            (0 == szCompliedWorkGroupSize[1]) &&
            (0 == szCompliedWorkGroupSize[2])))
    {
        // case kernel using the __attribute__((reqd_work_group_size(X, Y, Z))) qualifier in program source.
        if (  nullptr == m_cpszLocalWorkSize )
        {
            return CL_INVALID_WORK_GROUP_SIZE;
        }
        else
        {
            for( unsigned int ui=0; ui<m_uiWorkDim; ui++)
            {
                if( szCompliedWorkGroupSize[ui] != m_cpszLocalWorkSize[ui] )
                {
                    return CL_INVALID_WORK_GROUP_SIZE;
                }
            }

        }
    }


    if( nullptr != m_cpszLocalWorkSize )
    {
        size_t szDeviceMaxWorkGroupSize = m_pDevice->GetMaxWorkGroupSize();
        size_t szWorkGroupSize = 1;
        for( unsigned int ui=0; ui<m_uiWorkDim; ui++)
        {
            szWorkGroupSize *= m_cpszLocalWorkSize[ui];
        }
        if( szWorkGroupSize > szDeviceMaxWorkGroupSize )
        {
            /* CL_INVALID_WORK_GROUP_SIZE if local_work_size is specified and the total number of work-items
             * in the work-group computed as local_work_size[0] * local_work_size[work_dim - 1] is greater than
             * the value specified by CL_DEVICE_MAX_WORK_GROUP_SIZE in table 4.3.
             */
            return CL_INVALID_WORK_GROUP_SIZE;
        }

        if (szWorkGroupSize > szCompiledWorkGroupMaxSize)
        {
            // according to spec this is not an invalid WG size error, but it will be manifested
            // as out of resources.
            return CL_OUT_OF_RESOURCES;
        }

        cl_uint uiMaxWorkItemDim = m_pDevice->GetMaxWorkItemDimensions();
        if (uiMaxWorkItemDim == 0)
        {
            /* CL_INVALID_WORK_ITEM_SIZE if the number of work-items specified in any of
             * local_work_size[0], local_work_size[work_dim - 1] is greater than the corresponding
             * values specified by CL_DEVICE_MAX_WORK_ITEM_SIZES[0], CL_DEVICE_MAX_WORK_ITEM_SIZES[work_dim - 1].
             */
            return CL_INVALID_WORK_ITEM_SIZE;
        }

        const size_t* pszMaxWorkItemSizes = m_pDevice->GetMaxWorkItemSizes();

        for( unsigned int ui =0; ui<m_uiWorkDim; ui++)
        {
            if( m_cpszLocalWorkSize[ui] > pszMaxWorkItemSizes[ui])
            {
                return CL_INVALID_WORK_ITEM_SIZE;
            }
        }
    }

    // Check for uses_global_work_offset(0) m_pDeviceKernel attribute
    cl_bool blCanUseGlobalWorkOffset = m_pDeviceKernel->CanUseGlobalWorkOffset();

    if (!blCanUseGlobalWorkOffset && nullptr != m_cpszGlobalWorkOffset)
    {
        return CL_INVALID_GLOBAL_OFFSET;
    }

    cl_ulong stImplicitSize = m_pDeviceKernel->GetKernelLocalMemSize();
    stImplicitSize += stTotalLocalSize;
    if ( stImplicitSize > m_pDevice->GetMaxLocalMemorySize() )
    {
        res = CL_OUT_OF_RESOURCES;
    }

    if ( CL_FAILED(res) )
    {
        return res;
    }

    const size_t uiDispatchSize = m_pDeviceKernel->GetKernelDispatchBufferSize();
    const size_t uiDispatchAlignment = m_pDeviceKernel->GetKernelArgBufferAlignment();

    // Setup Kernel parameters
    char* pDispatchBuffer = nullptr;
    cl_dev_cmd_param_kernel* pKernelParam = &m_kernelParams;
    if ( uiDispatchSize > 0 )
    {
        pDispatchBuffer = (char*) ALIGNED_MALLOC(uiDispatchSize, uiDispatchAlignment);
        if ( nullptr == pDispatchBuffer )
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        // Get device specific offset of arguments in argument buffer
        const size_t uiArgOffset = m_pDeviceKernel->GetArgumentOffset();
        const size_t uiArgSize = uiDispatchSize - uiArgOffset;
        if ( uiArgSize > 0 )
        {
            MEMCPY_S( pDispatchBuffer+uiArgOffset, uiArgSize, m_pKernel->GetArgsBlob(), uiArgSize );

            const SharedPtr<Context>& pContext = m_pKernel->GetContext();

            size_t szMemArgCount        = m_pKernel->GetKernelMemoryArgsCount();
            char* pArgValues = pDispatchBuffer + uiArgOffset;

            // Update memory object information
            for(size_t i=0; i< szMemArgCount; i++)
            {
                pArg = m_pKernel->GetKernelMemoryArg(i);
                MemoryObject* pMemObj = nullptr;
                // may it be an SVM object?
                if (pArg->IsSvmPtr())
                {
                    SharedPointerArg* pSvmPtr;
                    pArg->GetValue(sizeof(SharedPointerArg*), &pSvmPtr);
                    pMemObj = static_cast<MemoryObject*>( pSvmPtr );
                }
                else if (pArg->IsUsmPtr())
                {
                    SharedPointerArg* pUsmPtr;
                    pArg->GetValue(sizeof(SharedPointerArg*), &pUsmPtr);
                    pMemObj = static_cast<MemoryObject*>(pUsmPtr);
                }
                else
                {
                    // Create buffer resources here if not available.
                    cl_mem clMemId;
                    pArg->GetValue(sizeof(cl_mem), &clMemId);

                    if ( nullptr == clMemId )
                    {
                        assert((pArg->IsBuffer() || pArg->IsSvmPtr() ||
                                pArg->IsUsmPtr()) && "nullptr values is allowed "
                                "only for buffers, SVM and USM pointers");
                        continue;
                    }

                    pMemObj = pContext->GetMemObjectPtr(clMemId);
                    if (nullptr == pMemObj)
                    {
                        return CL_INVALID_KERNEL_ARGS;
                    }
                }

                assert(nullptr != pMemObj && "Memory object is not supposed to be NULL on this stage");
                if( nullptr==pMemObj )
                {
                    continue;
                }

                // increment refcount of memory object and save it
                // TODO: Check why we always pass READ_WRITE as usage. Why we need it at all
                AddToMemoryObjectArgList( m_MemOclObjects, pMemObj, MemoryObject::READ_WRITE);

                // Mark as used
                res = pMemObj->CreateDeviceResource(m_pDevice);
                if( CL_FAILED(res))
                {
                    assert( 0 && "CreateDeviceResource() supposed to success" );
                    return res;
                }                

                // Get location in the command parameters
                IOCLDevMemoryObject* *devObjSrc = (IOCLDevMemoryObject**)(pArgValues + pArg->GetOffset());
                res = GetMemObjectDescriptor(pMemObj, devObjSrc);
                if ( CL_FAILED(res) )
                {
                    assert( 0 && "GetMemObjectDescriptor() supposed to success" );
                    return res;
                }
            }
        } // uiArgSize > 0
    } // uiDispatchSize > 0

    pKernelParam->arg_size = uiDispatchSize;
    pKernelParam->arg_values = (void*)pDispatchBuffer;
    pKernelParam->ppNonArgSvmBuffers = m_nonArgSvmBuffersVec.empty() ?
        nullptr : &m_nonArgSvmBuffersVec[0];
    pKernelParam->uiNonArgSvmBuffersCount = m_nonArgSvmBuffersVec.size();

    pKernelParam->ppNonArgUsmBuffers = m_nonArgUsmBuffersVec.empty() ?
        nullptr : &m_nonArgUsmBuffersVec[0];
    pKernelParam->uiNonArgUsmBuffersCount = m_nonArgUsmBuffersVec.size();

    // Fill specific command values
    pKernelParam->work_dim = m_uiWorkDim;
    for( cl_uint i=0; i < m_uiWorkDim; i++)
    {
        pKernelParam->glb_wrk_offs[i] = (nullptr != m_cpszGlobalWorkOffset) ? m_cpszGlobalWorkOffset[i] : 0;
        pKernelParam->glb_wrk_size[i] = m_cpszGlobalWorkSize[i];
        // If m_cpszLocalWorkSize == NULL, set to 0. Agent is expected to handle lcl_wrk_size 0 as NULL
        if (nullptr == m_cpszLocalWorkSize)
        {
            pKernelParam->lcl_wrk_size[UNIFORM_WG_SIZE_INDEX][i] =
            pKernelParam->lcl_wrk_size[NONUNIFORM_WG_SIZE_INDEX][i] = 0;
        }
        else
        {
          size_t tailSize = m_cpszGlobalWorkSize[i] % m_cpszLocalWorkSize[i];
          pKernelParam->lcl_wrk_size[UNIFORM_WG_SIZE_INDEX][i] = m_cpszLocalWorkSize[i];
          pKernelParam->lcl_wrk_size[NONUNIFORM_WG_SIZE_INDEX][i] = 0 < tailSize ?
                                                                        tailSize :
                                                                        m_cpszLocalWorkSize[i];
        }
    }

    // Set GPA data
    GPA_InitCommand();

    return CL_SUCCESS;
}

/******************************************************************
 * TODO: Move buffer handles to init
 ******************************************************************/
cl_err_code NDRangeKernelCommand::Execute()
{
    // Set location
    cl_err_code res = AcquireMemoryObjects(m_MemOclObjects);
    if ( CL_SUCCESS != res )
    {
        return res;
    }

    cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    // Fill command descriptor
    prepare_command_descriptor(CL_DEV_CMD_EXEC_KERNEL, &m_kernelParams, sizeof(cl_dev_cmd_param_kernel));

    m_kernelParams.kernel = m_pDeviceKernel->GetId();

    // Color will be changed only when command is submitted in the device

    // Sending the queue command
    // TODO: Handle the case were buffers are located in different device.

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event->GetId());

    return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &m_pDevCmd, 1);
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code NDRangeKernelCommand::CommandDone()
{
    // Clear all resources
    // Remove object pendencies
    RelinquishMemoryObjects(m_MemOclObjects);

    m_MemOclObjects.clear();

    // Delete local command
    ALIGNED_FREE(m_kernelParams.arg_values);

    return CL_SUCCESS;
}


/******************************************************************
 *
 ******************************************************************/
void NDRangeKernelCommand::GPA_WriteCommandMetadata()
{ 
#if defined (USE_GPA)
    ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();
    
    if ((nullptr != pGPAData) && (pGPAData->bUseGPA))
    {
        // Set custom track 
        __itt_set_track(m_pCommandQueue->GPA_GetQueue()->m_pTrack);
        __itt_metadata_add(pGPAData->pContextDomain, m_pGpaCommand->m_CmdId, pGPAData->pWorkDimensionHandle, __itt_metadata_u32, 1,&m_uiWorkDim);

        GPA_WriteWorkMetadata(m_cpszGlobalWorkSize, pGPAData->pGlobalWorkSizeHandle);
        GPA_WriteWorkMetadata(m_cpszLocalWorkSize, pGPAData->pLocalWorkSizeHandle);
        GPA_WriteWorkMetadata(m_cpszGlobalWorkOffset, pGPAData->pGlobalWorkOffsetHandle);
    }
#endif // GPA
}
/******************************************************************
 *
 ******************************************************************/
#if defined (USE_GPA)
void NDRangeKernelCommand::GPA_WriteWorkMetadata(const size_t* pWorkMetadata, __itt_string_handle* keyStrHandle) const
{ 
    ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();

    if ((nullptr != pGPAData) && (pGPAData->bUseGPA) && (pWorkMetadata != nullptr))
    {
        ocl_gpa_data* pGPAData = m_pCommandQueue->GetGPAData();

        // Set custom track 
        __itt_set_track(m_pCommandQueue->GPA_GetQueue()->m_pTrack);

        // Make sure all metadata is 64 bit, and not platform dependant (size_t)
        cl_ulong metaData64[MAX_WORK_DIM];
        for (unsigned int i = 1 ; i < m_uiWorkDim ; ++i)
        {
            metaData64[i] = pWorkMetadata[i];
        }

        // Write Metadata to trace
        __itt_metadata_add(pGPAData->pContextDomain, __itt_null, keyStrHandle, __itt_metadata_u64, m_uiWorkDim, metaData64);
    }
}
#endif // GPA

/******************************************************************
 * Command: ReadBufferCommand
 * The functions below implement the Read Buffer functinoality
 *
 ******************************************************************/
ReadBufferCommand::ReadBufferCommand(const SharedPtr<IOclCommandQueueBase>& cmdQueue, ocl_entry_points* pOclEntryPoints, const SharedPtr<MemoryObject>& pBuffer, const size_t pszOffset[3], const size_t pszCb[3], void* pDst)
:ReadMemObjCommand(cmdQueue, pOclEntryPoints, pBuffer, pszOffset, pszCb, 0, 0, pDst)
{
    m_commandType = CL_COMMAND_READ_BUFFER;
}

//////////////////
ReadBufferCommand::~ReadBufferCommand()
{
}


/******************************************************************
 *
 ******************************************************************/
ReadBufferRectCommand::ReadBufferRectCommand(
    const SharedPtr<IOclCommandQueueBase>& cmdQueue,
    ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&     pBuffer,
            const size_t      szBufferOrigin[3],
            const size_t      szDstOrigin[3],
            const size_t      szRegion[3],
            const size_t      szBufferRowPitch,
            const size_t      szBufferSlicePitch,
            const size_t      szDstRowPitch,
            const size_t      szDstSlicePitch,
            void*             pDst
            ):ReadMemObjCommand(cmdQueue, pOclEntryPoints, pBuffer, szBufferOrigin, szRegion, szBufferRowPitch, szBufferSlicePitch, pDst, szDstOrigin, szDstRowPitch, szDstSlicePitch)
{
    m_commandType = CL_COMMAND_READ_BUFFER_RECT;
}

ReadBufferRectCommand::~ReadBufferRectCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
ReadImageCommand::ReadImageCommand(
                                   const SharedPtr<IOclCommandQueueBase>& cmdQueue,
                                   ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pImage,
            const size_t*   pszOrigin,
            const size_t*   pszRegion,
            size_t          szRowPitch,
            size_t          szSlicePitch,
            void*           pDst)
:ReadMemObjCommand(cmdQueue, pOclEntryPoints, pImage, pszOrigin, pszRegion, 0, 0, pDst, nullptr, szRowPitch, szSlicePitch)
{
    m_commandType = CL_COMMAND_READ_IMAGE;
}

/******************************************************************
 *
 ******************************************************************/
ReadImageCommand::~ReadImageCommand()
{
}
/******************************************************************
 *
 ******************************************************************/
ReadMemObjCommand::ReadMemObjCommand(
    const SharedPtr<IOclCommandQueueBase>& cmdQueue,
    ocl_entry_points *    pOclEntryPoints,
    const SharedPtr<MemoryObject>&   pMemObj,
    const size_t*   pszOrigin,
    const size_t*   pszRegion,
    size_t          szRowPitch,
    size_t          szSlicePitch,
    void*           pDst,
    const size_t*    pszDstOrigin,
    const size_t    szDstRowPitch,
    const size_t    szDstSlicePitch
    ):
    MemoryCommand(cmdQueue),
    m_szMemObjRowPitch(szRowPitch),
    m_szMemObjSlicePitch(szSlicePitch),
    m_pDst(pDst),
    m_szDstRowPitch(szDstRowPitch),
    m_szDstSlicePitch(szDstSlicePitch)
{
//    size_t uiDimCount = m_pMemObj->GetNumDimensions();
    AddToMemoryObjectArgList( m_MemOclObjects, pMemObj, MemoryObject::READ_ONLY );

    // Set region
    for( cl_uint i =0; i<MAX_WORK_DIM; i++)
    {
        m_szOrigin[i] = pszOrigin[i];
        m_szRegion[i] = pszRegion[i];

        if (pszDstOrigin)
        {
            m_szDstOrigin[i] = pszDstOrigin[i];
        }
        else
        {
            m_szDstOrigin[i] = 0;
        }
    }

    if (pMemObj->GetType() != CL_MEM_OBJECT_BUFFER)
    {
        if( 0 == szDstRowPitch  )
        {
            m_szDstRowPitch = pszRegion[0] * pMemObj->GetPixelSize();
        }
        if( 0 == szDstSlicePitch )
        {
            m_szDstSlicePitch = m_szDstRowPitch * pszRegion[1];
        }
    }
}

/******************************************************************
 *
 ******************************************************************/
ReadMemObjCommand::~ReadMemObjCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadMemObjCommand::Init()
{
    assert(m_MemOclObjects.size() == 1 && "Memory object list must be == 1");
    const SharedPtr<MemoryObject>& pMemObj = m_MemOclObjects[0].pMemObj;

    cl_err_code res = pMemObj->CreateDeviceResource(m_pDevice);
    if( CL_FAILED(res))
    {
        return res;
    }

    // Initialize GPA data
    GPA_InitCommand();

    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadMemObjCommand::Execute()
{
    cl_dev_cmd_desc *pDevCmd = &m_DevCmd;

    assert(m_MemOclObjects.size() == 1);
    m_MemOclObjects[0].access_rights = MemoryObject::READ_ONLY;
    const SharedPtr<MemoryObject>& pMemObj = m_MemOclObjects[0].pMemObj;
    
    cl_err_code res = GetMemObjectDescriptor(pMemObj, &m_rwParams.memObj);
    if ( CL_FAILED(res) )
    {
        assert(0 && "ReadMemObjCommand::Execute() Failed: on a call to GetMemObjectDescriptor");
        return res;
    }

    res = AcquireMemoryObjects(m_MemOclObjects);
    if ( CL_SUCCESS != res )
    {
        return res;
    }

    create_dev_cmd_rw(
        m_commandType == CL_COMMAND_READ_BUFFER_RECT ? MAX_WORK_DIM  : pMemObj->GetNumDimensions(),
        m_pDst, m_szOrigin, m_szDstOrigin, m_szRegion, m_szDstRowPitch, m_szDstSlicePitch, m_szMemObjRowPitch, m_szMemObjSlicePitch,
        CL_DEV_CMD_READ );

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event->GetId());
    // Sending 1 command to the device where the buffer is located now

    return m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &pDevCmd, 1);

}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ReadMemObjCommand::CommandDone()
{
    RelinquishMemoryObjects(m_MemOclObjects);
    m_MemOclObjects.clear();
    return CL_SUCCESS;
}


/******************************************************************
 *
 ******************************************************************/
TaskCommand::TaskCommand( const SharedPtr<IOclCommandQueueBase>& cmdQueue, ocl_entry_points* pOclEntryPoints, const SharedPtr<Kernel>& pKernel ):
    NDRangeKernelCommand(cmdQueue, pOclEntryPoints, pKernel, 1, nullptr, &m_szStaticWorkSize, &m_szStaticWorkSize),
    m_szStaticWorkSize(1)
{

}

/******************************************************************
 *
 ******************************************************************/
TaskCommand::~TaskCommand()
{
}

/******************************************************************
 * initiate NDRangeKernel and change the device command type
 ******************************************************************/
cl_err_code TaskCommand::Init()
{
    cl_dev_cmd_desc *m_pDevCmd = &m_DevCmd;
    cl_err_code res = NDRangeKernelCommand::Init();
    if ( CL_SUCCEEDED (res) )
    {
        m_pDevCmd->type = CL_DEV_CMD_EXEC_KERNEL;
    }
    return res;
}

/******************************************************************
 *
 ******************************************************************/
WriteBufferCommand::WriteBufferCommand(const SharedPtr<IOclCommandQueueBase>& cmdQueue, ocl_entry_points* pOclEntryPoints, cl_bool bBlocking, const SharedPtr<MemoryObject>& pBuffer, const size_t* pszOffset, const size_t* pszCb, const void* cpSrc)
: WriteMemObjCommand(cmdQueue, pOclEntryPoints, bBlocking, pBuffer, pszOffset, pszCb, 0, 0, cpSrc)
{
    m_commandType = CL_COMMAND_WRITE_BUFFER;
}


/******************************************************************
 *
 ******************************************************************/
WriteBufferCommand::~WriteBufferCommand()
{
}



/******************************************************************
 *  FillBufferCommand
 ******************************************************************/
FillBufferCommand::FillBufferCommand(const SharedPtr<IOclCommandQueueBase>& cmdQueue, ocl_entry_points *pOclEntryPoints, const SharedPtr<MemoryObject>&pBuffer, const void *pattern, size_t pattern_size, size_t offset, size_t size)
: FillMemObjCommand(cmdQueue, pOclEntryPoints, pBuffer, offset, size, pattern, pattern_size)
{
    m_commandType = CL_COMMAND_FILL_BUFFER;
}


FillBufferCommand::~FillBufferCommand()
{
}



/******************************************************************
 *  FillImageCommand
 ******************************************************************/
FillImageCommand::FillImageCommand( const SharedPtr<IOclCommandQueueBase>& cmdQueue, ocl_entry_points *    pOclEntryPoints, const SharedPtr<MemoryObject>&   pImg, const void *pattern, size_t pattern_size, const cl_uint num_of_dimms, const size_t *offset, const size_t *size)
: FillMemObjCommand(cmdQueue, pOclEntryPoints, pImg, offset, size, num_of_dimms, pattern, pattern_size)
{
    m_commandType = CL_COMMAND_FILL_IMAGE;
}

FillImageCommand::~FillImageCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
WriteBufferRectCommand::WriteBufferRectCommand(
        const SharedPtr<IOclCommandQueueBase>& cmdQueue,
        ocl_entry_points *    pOclEntryPoints,
            cl_bool            bBlocking,
            const SharedPtr<MemoryObject>&     pBuffer,
            const size_t      szBufferOrigin[3],
            const size_t      szSrcOrigin[3],
            const size_t      szRegion[3],
            const size_t      szBufferRowPitch,
            const size_t      szBufferSlicePitch,
            const size_t      szDstRowPitch,
            const size_t      szDstSlicePitch,
            const void*       pDst
            ):
    WriteMemObjCommand(cmdQueue, pOclEntryPoints, bBlocking, pBuffer, szBufferOrigin, szRegion, szBufferRowPitch, szBufferSlicePitch, pDst, szSrcOrigin, szDstRowPitch, szDstSlicePitch)
{
    m_commandType = CL_COMMAND_WRITE_BUFFER_RECT;
}

WriteBufferRectCommand::~WriteBufferRectCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
WriteImageCommand::WriteImageCommand(
    const SharedPtr<IOclCommandQueueBase>& cmdQueue,
    ocl_entry_points *    pOclEntryPoints,
    cl_bool            bBlocking,
    const SharedPtr<MemoryObject>&   pImage,
    const size_t*   pszOrigin,
    const size_t*   pszRegion,
    size_t          szRowPitch,
    size_t          szSlicePitch,
    const void *    cpSrc
    ): WriteMemObjCommand(cmdQueue, pOclEntryPoints, bBlocking, pImage,pszOrigin, pszRegion, 0, 0, cpSrc, nullptr, szRowPitch, szSlicePitch)
{
    m_commandType = CL_COMMAND_WRITE_IMAGE;
}
/******************************************************************
 *
 ******************************************************************/
WriteImageCommand::~WriteImageCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
WriteMemObjCommand::WriteMemObjCommand(
    const SharedPtr<IOclCommandQueueBase>& cmdQueue,
    ocl_entry_points *    pOclEntryPoints,
    cl_bool            bBlocking,
    const SharedPtr<MemoryObject>&   pMemObj,
    const size_t*   pszOrigin,
    const size_t*   pszRegion,
    size_t          szRowPitch,
    size_t          szSlicePitch,
    const void *    cpSrc,
    const size_t*   pszSrcOrigin,
    const size_t    szSrcRowPitch,
    const size_t    szSrcSlicePitch
    ):
    MemoryCommand(cmdQueue),
    m_bBlocking(bBlocking),
    m_szMemObjRowPitch(szRowPitch),
    m_szMemObjSlicePitch(szSlicePitch),
    m_cpSrc(cpSrc),
    m_szSrcRowPitch(szSrcRowPitch),
    m_szSrcSlicePitch(szSrcSlicePitch)
{
    AddToMemoryObjectArgList( m_MemOclObjects, pMemObj, MemoryObject::READ_WRITE );
    // Set region
    for( cl_uint i =0; i<MAX_WORK_DIM; i++)
    {
        m_szOrigin[i] = pszOrigin[i];
        m_szRegion[i] = pszRegion[i];

        if (pszSrcOrigin)
        {
            m_szSrcOrigin[i] = pszSrcOrigin[i];
        }
        else
        {
            m_szSrcOrigin[i] = 0;
        }
    }

    if (pMemObj->GetType() != CL_MEM_OBJECT_BUFFER)
    {
        if( 0 == szSrcRowPitch  )
        {
            // Get original image pitch
            m_szSrcRowPitch = pszRegion[0]*pMemObj->GetPixelSize();
        }
        if( 0 == szSrcSlicePitch )
        {
            // Get original image pitch
            m_szSrcSlicePitch = m_szSrcRowPitch*pszRegion[1];
        }
    }
    else
    {
        // For buffers, if not set row_pitch == slice_pitch == lenght
        if ( 0 == m_szSrcRowPitch )
        {
            m_szSrcRowPitch = m_szRegion[0];
        }
        if ( 0 == m_szSrcSlicePitch )
        {
            m_szSrcSlicePitch = m_szRegion[0];
        }
    }
}

/******************************************************************
 *
 ******************************************************************/
WriteMemObjCommand::~WriteMemObjCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteMemObjCommand::Init()
{
    assert(m_MemOclObjects.size() == 1 && "Memory object list must be == 1");
    const SharedPtr<MemoryObject>& pMemObj = m_MemOclObjects[0].pMemObj;
    
    // If we are blocking command, we need to allocate internal buffer
    if ( m_bBlocking )
    {
        SMemCpyParams sCpyParam;
        // We need to allocate only the amount to being copied
        size_t sizeToAlloc = pMemObj->GetPixelSize();
        for(size_t dim=0; dim<MAX_WORK_DIM; ++dim)
        {
            sizeToAlloc *= m_szRegion[dim];
            sCpyParam.vRegion[dim] = m_szRegion[dim];
        }

        // nmeraey: changing this allocation alignment from 64 to 128 to fix CSSD100016136
        //          anyway, couldn't know why 128 would fix it!
        m_pTempBuffer = ALIGNED_MALLOC(sizeToAlloc, WRITE_MEM_OBJ_ALLOC_ALIGNMENT);
        if ( nullptr == m_pTempBuffer )
        {
            LogErrorA("Can't allocate temporary storage for blockng command (%s)", GetCommandName());
            return CL_OUT_OF_HOST_MEMORY;
        }

        // Copy data
        sCpyParam.vRegion[0] *= pMemObj->GetPixelSize();
        sCpyParam.uiDimCount = m_commandType == CL_COMMAND_WRITE_BUFFER_RECT ? MAX_WORK_DIM  : pMemObj->GetNumDimensions();
        sCpyParam.pDst = (cl_char*)m_pTempBuffer;
        sCpyParam.vDstPitch[0] = sCpyParam.vRegion[0];
        sCpyParam.vDstPitch[1] = sCpyParam.vDstPitch[0] * m_szRegion[1];
        sCpyParam.pSrc = (cl_char*)m_cpSrc + m_szSrcOrigin[0]*pMemObj->GetPixelSize() + m_szSrcOrigin[1]*m_szSrcRowPitch + m_szSrcOrigin[2]*m_szSrcSlicePitch;
        sCpyParam.vSrcPitch[0] = m_szSrcRowPitch;
        sCpyParam.vSrcPitch[1] = m_szSrcSlicePitch;

        clCopyMemoryRegion(&sCpyParam);

        // Need to update source origin and pitch, now we will write from temporary buffer
        m_szSrcRowPitch = sCpyParam.vDstPitch[0];
        m_szSrcSlicePitch = sCpyParam.vDstPitch[1];
        memset(m_szSrcOrigin, 0, sizeof(m_szSrcOrigin));
    }

    // Allocate
    cl_err_code res = pMemObj->CreateDeviceResource(m_pDevice);
    if( CL_FAILED(res))
    {
        ALIGNED_FREE(m_pTempBuffer);
        m_pTempBuffer = nullptr;
        return res;
    }

    // Initialize GPA data
    GPA_InitCommand();
    assert(m_MemOclObjects.size() == 1);
    if (pMemObj->IsWholeObjectCovered(
                            CL_COMMAND_WRITE_BUFFER_RECT ? MAX_WORK_DIM  : m_MemOclObjects[0].pMemObj->GetNumDimensions(),
                            m_szOrigin, m_szRegion))
    {
        m_MemOclObjects[0].access_rights = MemoryObject::WRITE_ENTIRE;
    }
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteMemObjCommand::Execute()
{  
    cl_dev_cmd_desc *pDevCmd = &m_DevCmd;
    assert(m_MemOclObjects.size() == 1 && "We expect single memory object in the command");
    const SharedPtr<MemoryObject>& pMemObj = m_MemOclObjects[0].pMemObj;

    cl_err_code res = GetMemObjectDescriptor(pMemObj, &m_rwParams.memObj);
    if ( CL_FAILED(res) )
    {
        assert(0 && "WriteMemObjCommand::Execute() Failed: on a call to GetMemObjectDescriptor");
        ALIGNED_FREE(m_pTempBuffer);
        m_pTempBuffer = nullptr;
        return res;
    }

    res = AcquireMemoryObjects(m_MemOclObjects);
    if ( CL_NOT_READY == res )
    {
        return res;
    }

    if ( CL_SUCCESS != res )
    {
        ALIGNED_FREE(m_pTempBuffer);
        m_pTempBuffer = nullptr;
        return res;
    }

    create_dev_cmd_rw(
            m_commandType == CL_COMMAND_WRITE_BUFFER_RECT ? MAX_WORK_DIM  : pMemObj->GetNumDimensions(),
            m_bBlocking ? m_pTempBuffer : (void*)m_cpSrc,
            m_szOrigin, m_szSrcOrigin, m_szRegion, m_szSrcRowPitch, m_szSrcSlicePitch, m_szMemObjRowPitch, m_szMemObjSlicePitch,
            CL_DEV_CMD_WRITE ) ;

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event->GetId());
    // Sending 1 command to the device where the buffer is located now
    cl_dev_err_code errDev = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, &pDevCmd, 1);
    if ( CL_DEV_FAILED(errDev) )
    {
        ALIGNED_FREE(m_pTempBuffer);
        m_pTempBuffer = nullptr;
    }
    //m_Event->RemovePendency(this);
    return CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code WriteMemObjCommand::CommandDone()
{
    if ( m_bBlocking && (nullptr != m_pTempBuffer) )
    {
        ALIGNED_FREE(m_pTempBuffer);
        m_pTempBuffer = nullptr;
    }

    RelinquishMemoryObjects(m_MemOclObjects);
    m_MemOclObjects.clear();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code RuntimeCommand::Execute()
{
    LogDebugA("Command - DONE  : %s (Id: %d)", GetCommandName(), m_Event->GetId());
    CommandDone();
    m_Event->SetEventState(EVENT_STATE_DONE);
    DetachEventSharedPtr();
    return m_returnCode;
}


/******************************************************************
 *
 ******************************************************************/
FillMemObjCommand::FillMemObjCommand(
        const SharedPtr<IOclCommandQueueBase>& cmdQueue,
        ocl_entry_points *    pOclEntryPoints,
        const SharedPtr<MemoryObject>&   pMemObj,
        const size_t*   pszOffset,
        const size_t*   pszRegion,
        const cl_uint   numOfDimms,
        const void*     pattern,
        const size_t    pattern_size
    ) :
    Command(cmdQueue),
    m_numOfDimms(numOfDimms), m_pattern_size(pattern_size),
    m_internalErr(CL_SUCCESS)
{
    m_commandType = CL_DEV_CMD_FILL_IMAGE;

    // Set region
    for( int i=0 ; i<MAX_WORK_DIM ; ++i)
    {
        m_szOffset[i] = pszOffset[i];
        m_szRegion[i] = pszRegion[i];
    }

    MEMCPY_S(m_pattern, m_pattern_size, pattern, m_pattern_size);
    AddToMemoryObjectArgList( m_MemOclObjects, pMemObj, MemoryObject::READ_WRITE );
}

FillMemObjCommand::FillMemObjCommand(
            const SharedPtr<IOclCommandQueueBase>& cmdQueue,
            ocl_entry_points *    pOclEntryPoints,
            const SharedPtr<MemoryObject>&   pMemObj,
            const size_t    pszOffset,
            const size_t    pszRegion,
            const void*     pattern,
            const size_t    pattern_size
        ) :
        Command(cmdQueue),
        m_numOfDimms(1), m_pattern_size(pattern_size),
        m_internalErr(CL_SUCCESS)
{
    m_commandType = CL_DEV_CMD_FILL_BUFFER;

    // Set region
    m_szOffset[0] = pszOffset;
    m_szRegion[0] = pszRegion;

    MEMCPY_S(m_pattern, m_pattern_size, pattern, m_pattern_size);
    AddToMemoryObjectArgList( m_MemOclObjects, pMemObj, MemoryObject::READ_WRITE );
}

/******************************************************************
 *
 ******************************************************************/
FillMemObjCommand::~FillMemObjCommand()
{
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code FillMemObjCommand::Init()
{
    if (CL_SUCCESS != m_internalErr)
    {
        return m_internalErr;
    }

    assert(m_MemOclObjects.size() == 1);
    const SharedPtr<MemoryObject>& pMemObj = m_MemOclObjects[0].pMemObj;
    // Allocate
    m_internalErr = pMemObj->CreateDeviceResource(m_pDevice);
    if( CL_FAILED(m_internalErr) )
    {
        return m_internalErr;
    }

    m_MemOclObjects[0].access_rights = (pMemObj->IsWholeObjectCovered( m_numOfDimms, m_szOffset, m_szRegion )) ?
                                            MemoryObject::WRITE_ENTIRE : MemoryObject::READ_WRITE;

    // Initialize GPA data
    GPA_InitCommand();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code FillMemObjCommand::Execute()
{
    if (CL_SUCCESS != m_internalErr)
    {
        return m_internalErr;
    }

    /// memory object resides on target device, update it
    SharedPtr<OclEvent> pObjEvent;
    assert(m_MemOclObjects.size() == 1);
    const SharedPtr<MemoryObject>& pMemObj = m_MemOclObjects[0].pMemObj;
    cl_err_code clErr = pMemObj->GetDeviceDescriptor(m_pDevice, &(m_fillCmdParams.memObj), &pObjEvent);
    if ( CL_FAILED(clErr) )
    {
        return clErr;
    }

    cl_err_code res = AcquireMemoryObjects(m_MemOclObjects);
    if ( CL_SUCCESS != res )
    {
        return res;
    }

    m_fillCmdParams.dim_count    = m_numOfDimms;
    for( int i=0 ; i<MAX_WORK_DIM ; ++i)
    {
        m_fillCmdParams.offset[i] = m_szOffset[i];
        m_fillCmdParams.region[i] = m_szRegion[i];
    }
    // No need to copy m_pattern's content, since we are still in the host.
    assert(MAX_PATTERN_SIZE    >= m_pattern_size && "Trying to use a pattern larger than possible");
    m_fillCmdParams.pattern_size = m_pattern_size;
    MEMCPY_S(m_fillCmdParams.pattern, m_fillCmdParams.pattern_size, m_pattern, m_fillCmdParams.pattern_size);

    //FillMemObject::Create(TaskDispatcher* pTD, cl_dev_cmd_desc* pCmd, ITaskBase* *pTask)
    prepare_command_descriptor((m_commandType == CL_DEV_CMD_FILL_BUFFER) ? CL_DEV_CMD_FILL_BUFFER : CL_DEV_CMD_FILL_IMAGE, 
                                &m_fillCmdParams, sizeof(cl_dev_cmd_param_fill));

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event->GetId());

    // Sending 1 command to the device where the buffer is located now
    cl_dev_cmd_desc* cmdPList[1] = {&m_DevCmd};
    cl_dev_err_code errDev = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, cmdPList, 1);


    return CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code FillMemObjCommand::CommandDone()
{
    RelinquishMemoryObjects(m_MemOclObjects);
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
MigrateSVMMemCommand::MigrateSVMMemCommand(
    const SharedPtr<IOclCommandQueueBase>&  cmdQueue,
    ContextModule*         pContextModule,
    cl_mem_migration_flags clFlags,
    cl_uint                uNumMemObjects,
    const void**           pMemObjects,
    const size_t*          sizes):
    Command(cmdQueue), m_pMemObjects(pMemObjects),
    m_pSizes(sizes), m_pContextModule( pContextModule )
{
    assert( 0 != uNumMemObjects );
    assert( nullptr != pMemObjects );
    assert( nullptr != pContextModule );

    memset( &m_migrateCmdParams, 0, sizeof(cl_dev_cmd_param_migrate));
    m_migrateCmdParams.flags    = clFlags;
    m_migrateCmdParams.mem_num  = uNumMemObjects;
}

/******************************************************************
 *
 ******************************************************************/
MigrateSVMMemCommand::~MigrateSVMMemCommand()
{
    if (nullptr != m_migrateCmdParams.memObjs)
    {
        delete [] m_migrateCmdParams.memObjs;
        m_migrateCmdParams.memObjs = nullptr;
    }
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateSVMMemCommand::Init()
{
    // Allocate
    m_migrateCmdParams.memObjs = new IOCLDevMemoryObject*[m_migrateCmdParams.mem_num];

    if (nullptr == m_migrateCmdParams.memObjs)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    MemoryObject::MemObjUsage access = (0 != (m_migrateCmdParams.flags & CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)) ?
                            MemoryObject::WRITE_ENTIRE : MemoryObject::READ_ONLY; // use READ_ONLY for optimization

    SharedPtr<Context> pQueueContext = m_pContextModule->GetContext(m_pCommandQueue->GetParentHandle());
    for (cl_uint i = 0; i < m_migrateCmdParams.mem_num; i++)
    {
        SharedPtr<SVMBuffer> pMemObj = pQueueContext->GetSVMBufferContainingAddr(const_cast<void*>(m_pMemObjects[i]));
        if (0 == pMemObj)
        {
            return CL_INVALID_VALUE;
        }
        if (nullptr != m_pSizes && !pMemObj->IsContainedInBuffer(m_pMemObjects[i], m_pSizes[i]))
        {
            return CL_INVALID_VALUE;
        }

        cl_err_code cl_err = pMemObj->CreateDeviceResource(m_pDevice);
        if (CL_FAILED(cl_err))
        {
            return cl_err;
        }

        cl_err = GetMemObjectDescriptor(pMemObj, &( m_migrateCmdParams.memObjs[i]));
        if (CL_FAILED(cl_err))
        {
            return cl_err;
        }

        AddToMemoryObjectArgList(m_MemOclObjects, pMemObj, access);
    }

    // Initialize GPA data
    GPA_InitCommand();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateSVMMemCommand::Execute()
{
    cl_err_code res = AcquireMemoryObjects(m_MemOclObjects);
    if ( CL_SUCCESS != res )
    {
        return res;
    }

    prepare_command_descriptor(CL_DEV_CMD_SVM_MIGRATE, &m_migrateCmdParams, sizeof(cl_dev_cmd_param_migrate));

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event->GetId());

    // Sending 1 command to the target device
    cl_dev_cmd_desc* cmdPList[1] = {&m_DevCmd};
    cl_dev_err_code errDev = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, cmdPList, 1);
    return CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateSVMMemCommand::CommandDone()
{
    RelinquishMemoryObjects(m_MemOclObjects);
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
MigrateMemObjCommand::MigrateMemObjCommand(
        const SharedPtr<IOclCommandQueueBase>&  cmdQueue,
        ocl_entry_points *     pOclEntryPoints,
        ContextModule*         pContextModule,
        cl_mem_migration_flags clFlags,
        cl_uint                uNumMemObjects,
        const cl_mem*          pMemObjects
    ): 
    Command(cmdQueue),
    m_pMemObjects(pMemObjects), m_pContextModule( pContextModule )
{
    assert( 0 != uNumMemObjects );
    assert( nullptr != pMemObjects );
    assert( nullptr != pContextModule );

    memset( &m_migrateCmdParams, 0, sizeof(cl_dev_cmd_param_migrate));
    m_migrateCmdParams.flags    = clFlags;
    m_migrateCmdParams.mem_num  = uNumMemObjects;
}

/******************************************************************
 *
 ******************************************************************/
MigrateMemObjCommand::~MigrateMemObjCommand()
{
    if (nullptr != m_migrateCmdParams.memObjs)
    {
        delete [] m_migrateCmdParams.memObjs;
        m_migrateCmdParams.memObjs = nullptr;
    }
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateMemObjCommand::Init()
{
    // Allocate
    m_migrateCmdParams.memObjs = new IOCLDevMemoryObject*[m_migrateCmdParams.mem_num];

    if (nullptr == m_migrateCmdParams.memObjs)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    MemoryObject::MemObjUsage access = (0 != (m_migrateCmdParams.flags & CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)) ?
                            MemoryObject::WRITE_ENTIRE : MemoryObject::READ_ONLY; // use READ_ONLY for optimization
    
    SharedPtr<Context> const pQueueContext = m_pContextModule->GetContext(m_pCommandQueue->GetParentHandle());
    for (cl_uint i = 0; i < m_migrateCmdParams.mem_num; i++)
    {
        const SharedPtr<MemoryObject>& pMemObj = m_pContextModule->GetMemoryObject(m_pMemObjects[i]);
        if (0 == pMemObj)
        {
            return CL_INVALID_MEM_OBJECT;
        }
        if (pMemObj->GetContext() != pQueueContext)
        {
            return CL_INVALID_CONTEXT;
        }

        cl_err_code cl_err = pMemObj->CreateDeviceResource(m_pDevice);
        if (CL_FAILED(cl_err))
        {
            return cl_err;
        }
        
        cl_err = GetMemObjectDescriptor( pMemObj, &( m_migrateCmdParams.memObjs[i]));
        assert( CL_SUCCESS == cl_err );

        AddToMemoryObjectArgList( m_MemOclObjects, pMemObj, access );
    }
    
    // Initialize GPA data
    GPA_InitCommand();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateMemObjCommand::Execute()
{
    cl_err_code res = AcquireMemoryObjects(m_MemOclObjects);
    if ( CL_SUCCESS != res )
    {
        return res;
    }

    prepare_command_descriptor(CL_DEV_CMD_MIGRATE, &m_migrateCmdParams, sizeof(cl_dev_cmd_param_migrate));

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(), m_Event->GetId());

    // Sending 1 command to the target device
    cl_dev_cmd_desc* cmdPList[1] = {&m_DevCmd};
    cl_dev_err_code errDev = m_pDevice->GetDeviceAgent()->clDevCommandListExecute(m_clDevCmdListId, cmdPList, 1);
    return CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateMemObjCommand::CommandDone()
{
    RelinquishMemoryObjects(m_MemOclObjects);
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
MigrateUSMMemCommand::MigrateUSMMemCommand(
    const SharedPtr<IOclCommandQueueBase>&  cmdQueue,
    ContextModule*         contextModule,
    cl_mem_migration_flags clFlags,
    const void* ptr,
    size_t size):
    Command(cmdQueue), m_ptr(ptr),
    m_contextModule(contextModule)
{
    assert(nullptr != ptr);
    assert(nullptr != contextModule);

    memset(&m_migrateCmdParams, 0, sizeof(m_migrateCmdParams));
    m_migrateCmdParams.flags = clFlags;
    m_migrateCmdParams.size = size;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateUSMMemCommand::Init()
{
    MemoryObject::MemObjUsage access = (0 !=
        (m_migrateCmdParams.flags & CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)) ?
        MemoryObject::WRITE_ENTIRE : MemoryObject::READ_ONLY;

    SharedPtr<Context> queueContext =
        m_contextModule->GetContext(m_pCommandQueue->GetParentHandle());

    SharedPtr<USMBuffer> memObj = queueContext->GetUSMBufferContainingAddr(
        const_cast<void*>(m_ptr));
    if ((nullptr == memObj.GetPtr()) ||
        !memObj->IsContainedInBuffer(m_ptr, m_migrateCmdParams.size) ||
        (memObj->GetType() != CL_MEM_TYPE_SHARED_INTEL))
        return CL_INVALID_VALUE;

    cl_err_code cl_err = memObj->CreateDeviceResource(m_pDevice);
    if (CL_FAILED(cl_err))
        return cl_err;

    cl_err = GetMemObjectDescriptor(memObj, &(m_migrateCmdParams.memObj));
    if (CL_FAILED(cl_err))
        return cl_err;

    AddToMemoryObjectArgList(m_MemOclObjects, memObj, access);

    // Initialize GPA data
    GPA_InitCommand();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateUSMMemCommand::Execute()
{
    cl_err_code res = AcquireMemoryObjects(m_MemOclObjects);
    if (CL_SUCCESS != res)
        return res;

    prepare_command_descriptor(CL_DEV_CMD_USM_MIGRATE, &m_migrateCmdParams,
                               sizeof(m_migrateCmdParams));

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(),
              m_Event->GetId());

    SharedPtr<Context> queueContext = m_contextModule->GetContext(
        m_pCommandQueue->GetParentHandle());
    SharedPtr<USMBuffer> memObj = queueContext->GetUSMBufferContainingAddr(
        const_cast<void*>(m_ptr));
    if (nullptr == memObj.GetPtr())
        return CL_INVALID_VALUE;
    memObj->SetType(CL_MEM_TYPE_DEVICE_INTEL);
    memObj->SetDevice(m_pDevice->GetHandle());

    // Sending 1 command to the target device
    cl_dev_cmd_desc* cmdPList[1] = {&m_DevCmd};
    cl_dev_err_code errDev = m_pDevice->GetDeviceAgent()
        ->clDevCommandListExecute(m_clDevCmdListId, cmdPList, 1);
    return CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code MigrateUSMMemCommand::CommandDone()
{
    RelinquishMemoryObjects(m_MemOclObjects);
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
AdviseUSMMemCommand::AdviseUSMMemCommand(
    const SharedPtr<IOclCommandQueueBase>& cmdQueue,
    ContextModule* contextModule,
    const void* ptr,
    size_t size,
    cl_mem_advice_intel advice):
    Command(cmdQueue), m_ptr(ptr), m_contextModule(contextModule)
{
    assert(nullptr != ptr);
    assert(nullptr != contextModule);

    memset(&m_adviseCmdParams, 0, sizeof(m_adviseCmdParams));

    // Advice hints for GPU could be:
    // * memadvise hints to pre-setup page tables to refer to remote memory.
    // * override migration policy and keep an allocation on a specific device.
    // * enable replicating an allocation for reading across multiple devices.
    // TODO what are advice hints helpful for CPU?
    m_adviseCmdParams.advice  = advice;
    m_adviseCmdParams.size = size;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code AdviseUSMMemCommand::Init()
{
    m_adviseCmdParams.memObj = nullptr;

    MemoryObject::MemObjUsage access = MemoryObject::WRITE_ENTIRE;

    SharedPtr<Context> queueContext = m_contextModule->GetContext(
        m_pCommandQueue->GetParentHandle());

    SharedPtr<USMBuffer> memObj =
        queueContext->GetUSMBufferContainingAddr(const_cast<void*>(m_ptr));
    if (nullptr == memObj.GetPtr() ||
        !memObj->IsContainedInBuffer(m_ptr, m_adviseCmdParams.size))
        return CL_INVALID_VALUE;

    cl_err_code cl_err = memObj->CreateDeviceResource(m_pDevice);
    if (CL_FAILED(cl_err))
        return cl_err;

    cl_err = GetMemObjectDescriptor(memObj, &(m_adviseCmdParams.memObj));
    if (CL_FAILED(cl_err))
        return cl_err;

    AddToMemoryObjectArgList(m_MemOclObjects, memObj, access);

    // Initialize GPA data
    GPA_InitCommand();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code AdviseUSMMemCommand::Execute()
{
    cl_err_code res = AcquireMemoryObjects(m_MemOclObjects);
    if (CL_SUCCESS != res)
        return res;

    prepare_command_descriptor(CL_DEV_CMD_USM_ADVISE, &m_adviseCmdParams,
                               sizeof(m_adviseCmdParams));

    LogDebugA("Command - EXECUTE: %s (Id: %d)", GetCommandName(),
              m_Event->GetId());

    // Sending 1 command to the target device
    cl_dev_cmd_desc* cmdPList[1] = { &m_DevCmd };
    cl_dev_err_code errDev = m_pDevice->GetDeviceAgent()
        ->clDevCommandListExecute(m_clDevCmdListId, cmdPList, 1);
    return CL_DEV_SUCCEEDED(errDev) ? CL_SUCCESS : CL_OUT_OF_RESOURCES;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code AdviseUSMMemCommand::CommandDone()
{
    RelinquishMemoryObjects(m_MemOclObjects);
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code ErrorQueueEvent::ObservedEventStateChanged(const SharedPtr<OclEvent>& pEvent, cl_int returnCode)
{
    return m_owner->GetEvent()->ObservedEventStateChanged( pEvent, m_owner->GetForcedErrorCode() ); 
}

/******************************************************************
 *
 ******************************************************************/
cl_int     ErrorQueueEvent::GetReturnCode() const
{
    return m_owner->GetForcedErrorCode();
}

cl_err_code    ErrorQueueEvent::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
{
    return m_owner->GetEvent()->GetInfo( iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet );
}

/******************************************************************
 *
 ******************************************************************/
PrePostFixRuntimeCommand::PrePostFixRuntimeCommand( 
    Command* relatedUserCommand, Mode working_mode, const SharedPtr<IOclCommandQueueBase>& cmdQueue ): 
    RuntimeCommand(cmdQueue), 
    m_relatedUserCommand(relatedUserCommand), 
    m_working_mode( working_mode ),
    m_force_error_return(CL_SUCCESS),
    m_error_event(ErrorQueueEvent::Allocate(cmdQueue->GetParentHandle())), m_task(RuntimeCommandTask::Allocate())
{
    assert( 0 != m_relatedUserCommand );

    m_commandType = relatedUserCommand->GetCommandType();
    
    m_error_event->Init( this );

    m_commandType = relatedUserCommand->GetCommandType();
    
    m_error_event->Init( this );
    m_task->Init( this );
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code PrePostFixRuntimeCommand::Init()
{


    // related command should not disapper before I finished


    // Initialize GPA data
    GPA_InitCommand();
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code PrePostFixRuntimeCommand::CommandDone()
{
    SharedPtr<QueueEvent> related_event = m_relatedUserCommand->GetEvent();

    // update times here
    if ( PREFIX_MODE == m_working_mode )
    {
        related_event->IncludeProfilingInfo( m_Event );
    }
    else
    {
        m_Event->IncludeProfilingInfo( related_event );
    }    
    m_task = nullptr;
    return CL_SUCCESS;
}

/******************************************************************
 *
 ******************************************************************/
void PrePostFixRuntimeCommand::ErrorDone()
{
    CommandDone();
}

/******************************************************************
 *
 ******************************************************************/
void PrePostFixRuntimeCommand::ErrorEnqueue(cl_event* intermediate_pEvent, cl_event* user_pEvent, cl_err_code err_to_force_return )
{
    assert( POSTFIX_MODE == m_working_mode && "Calling PrePostFixRuntimeCommand::ErrorEnqueue on prefix." );
    m_force_error_return = err_to_force_return;

    // add manually and leave postfix floating
    EventsManager*    event_manager = GetCommandQueue()->GetEventsManager();

    event_manager->RegisterQueueEvent( m_Event, user_pEvent );

    // 'this' may disapper right now
    cl_err_code err;
    err = event_manager->RegisterEvents( m_error_event, 1, intermediate_pEvent );

    // in our case RegisterEvents() cannot return failure by construction
    assert( CL_SUCCEEDED( err ));
}

/******************************************************************
 *
 ******************************************************************/
void PrePostFixRuntimeCommand::DoAction()
{
    LogDebugA("PrePostFixRuntimeCommand - DoAction Started: PrePostFixRuntimeCommand for %s (Id: %d)", GetCommandName(), m_Event->GetId());

    m_returnCode = CL_SUCCESS;

    cl_dev_cmd_id id = (cl_dev_cmd_id)m_Event->GetId();
    NotifyCmdStatusChanged(id, CL_RUNNING,  m_returnCode, Intel::OpenCL::Utils::HostTime());

    m_returnCode = ( PREFIX_MODE == m_working_mode ) ?
                        m_relatedUserCommand->PrefixExecute() 
                        :
                        m_relatedUserCommand->PostfixExecute();

    LogDebugA("PrePostFixRuntimeCommand - DoAction Finished: PrePostFixRuntimeCommand for %s (Id: %d)", GetCommandName(), m_Event->GetId());
    NotifyCmdStatusChanged(id, CL_COMPLETE, m_returnCode, Intel::OpenCL::Utils::HostTime());
}

/******************************************************************
 *
 ******************************************************************/
void PrePostFixRuntimeCommand::CancelAction()
{
    cl_dev_cmd_id id = (cl_dev_cmd_id)m_Event->GetId();
    LogDebugA("PrePostFixRuntimeCommand - DoAction Canceled: PrePostFixRuntimeCommand for %s (Id: %d)", GetCommandName(), m_Event->GetId());
    NotifyCmdStatusChanged(id, CL_COMPLETE, CL_DEVICE_NOT_AVAILABLE, Intel::OpenCL::Utils::HostTime());
}

/******************************************************************
 *
 ******************************************************************/
cl_err_code PrePostFixRuntimeCommand::Execute()
{
    cl_err_code            ret  = CL_SUCCESS;
    bool ok = FrameworkProxy::Instance()->Execute(SharedPtr<Intel::OpenCL::TaskExecutor::ITaskBase>(m_task));

    if (!ok)
    {
        LogDebugA("PrePostFixRuntimeCommand - Execute: Task submission failed for PrePostFixRuntimeCommand for %s (Id: %d)", GetCommandName(), m_Event->GetId());

        ret = CL_OUT_OF_RESOURCES;
        m_returnCode = ret;    
        NotifyCmdStatusChanged(0, CL_COMPLETE, ret, Intel::OpenCL::Utils::HostTime());

        return m_returnCode;
    }

    return CL_NOT_READY;
}

/******************************************************************
 *
 ******************************************************************/
bool RuntimeCommandTask::SetAsSyncPoint()
{
    assert(0 && "Should not get here");
    return false;
}

bool RuntimeCommandTask::CompleteAndCheckSyncPoint()
{
    return false;
}

bool RuntimeCommandTask::Execute()
{
    m_owner->DoAction();
    m_bIsCompleted = true;
    return true;
}

void RuntimeCommandTask::Cancel()
{
    m_owner->CancelAction();
    m_bIsCompleted = true;
}

/******************************************************************
 *
 ******************************************************************/
long RuntimeCommandTask::Release()
{
    m_owner = nullptr;
    return 0;
}
