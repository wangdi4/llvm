// Copyright (c) 2006-2014 Intel Corporation
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
//
//  Original author: rjiossy
///////////////////////////////////////////////////////////
#include <cl_secure_string.h>
#include "crt_internals.h"
#include "crt_module.h"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

namespace OCLCRT
{
    extern CrtModule crt_ocl_module;
}

CrtObject::CrtObject():
m_refCount(1), m_pendencyCount(1)
{
}


long CrtObject::IncRefCnt()
{
    return atomic_increment(&m_refCount);
}

long CrtObject::DecRefCnt()
{
    long newVal = atomic_decrement(&m_refCount);;
    if (m_refCount < 0)
    {
        atomic_increment(&m_refCount);
        return -1; // we cannot return 0, other call did
    }
    else if (0 == newVal)
    {
        DecPendencyCnt();
    }
    return newVal;
}

long CrtObject::IncPendencyCnt()
{
    return atomic_increment(&m_pendencyCount);
}

long CrtObject::DecPendencyCnt()
{
    long newVal = atomic_decrement(&m_pendencyCount);
    if (0 == newVal)
    {
        delete this;
    }
    return newVal;
}

bool operator==(const cl_image_format &rhs1, const cl_image_format &rhs2)
{
    return ( ( rhs1.image_channel_data_type == rhs2.image_channel_data_type )  &&
             ( rhs1.image_channel_order == rhs2.image_channel_order ) );
}

bool operator<(const cl_image_format &rhs1, const cl_image_format &rhs2)
{
    if ( rhs1.image_channel_data_type == rhs2.image_channel_data_type )
             return ( rhs1.image_channel_order < rhs2.image_channel_order );

    return ( rhs1.image_channel_data_type < rhs2.image_channel_data_type );
}

struct _cl_gpu_context
{
    SOCLEntryPointsTable      dispatch;
    cl_bool                   isSharedContext;
};

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
//  calculates the LCM (Least common multiple)
int CalculateLCM(int a, int b)
{
    int temp_a = a, temp_b = b;
    int temp = 0;
    for (;;)
    {
        if (temp_a == 0)
        {
            temp = temp_b;
            break;
        }
        temp_b %= temp_a;
        if (temp_b == 0)
        {
            temp = temp_a;
            break;
        }
        temp_a %= temp_b;
    }
    return temp ? (a / temp * b) : 0;
}

// Used by CopyMemoryObject, for copying image memory objects
struct MemCpyParams
{
    cl_uint         uiDimCount;
    cl_char*        pSrc;
    size_t          vSrcPitch[2];
    cl_char*        pDst;
    size_t          vDstPitch[2];
    size_t          vRegion[3];
};

void CopyMemoryObject(MemCpyParams* pCopyCmd)
{
    // Copy 1D array only
    if ( 1 == pCopyCmd->uiDimCount )
    {
        memcpy_s( pCopyCmd->pDst, pCopyCmd->vDstPitch[0], pCopyCmd->pSrc, pCopyCmd->vRegion[0] );
        return;
    }

    MemCpyParams sRecParam;

    // Copy current parameters
    memcpy_s( &sRecParam, sizeof(MemCpyParams), pCopyCmd, sizeof(MemCpyParams) );
    sRecParam.uiDimCount = pCopyCmd->uiDimCount-1;
    // Make recursion
    for(unsigned int i=0; i<pCopyCmd->vRegion[sRecParam.uiDimCount]; ++i)
    {
        CopyMemoryObject(&sRecParam);
        sRecParam.pSrc = sRecParam.pSrc + pCopyCmd->vSrcPitch[sRecParam.uiDimCount-1];
        sRecParam.pDst = sRecParam.pDst + pCopyCmd->vDstPitch[sRecParam.uiDimCount-1];
    }
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
// User event Callback, which releases userevent when called by all
// registered callbacks
void CL_CALLBACK CrtEventCallBack(cl_event e, cl_int status, void* user_data)
{
    CrtEventCallBackData* evData = (CrtEventCallBackData*)user_data;

    // Check if this is the last event to call this callback
    if (0 == atomic_decrement(&(evData->numReqCalls)))
    {
        // Release the crt user event (bridge)
        evData->m_eventDEV->dispatch->clSetUserEventStatus(
            evData->m_eventDEV, CL_COMPLETE);

        evData->m_eventDEV->dispatch->clReleaseEvent(evData->m_eventDEV);
        delete evData;
    }
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
void CL_CALLBACK CrtSetEventCallBack(cl_event e, cl_int status, void* crtUserData)
{
    CrtSetEventCallBackData* evData = ( CrtSetEventCallBackData* )crtUserData;
    evData->m_userPfnNotify( evData->m_crtEvent, status, evData->m_userData );
	delete evData;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
void CL_CALLBACK CrtMemDestructorCallBack(cl_mem m, void* userData)
{
    CrtMemDtorCallBackData *memData = (CrtMemDtorCallBackData*)userData;

    CrtMemObject* memObj = (memData->m_clMemHandle);

    // If no memory objects currently coupled in,
    // destroy the memory object then
    if (0 == atomic_decrement(&(memData->m_count)))
    {
        memObj->DecPendencyCnt();
        memData->m_clMemHandle = NULL;
        delete memData;

    }
}

// Mem destructor callback and user data
struct CrtMemDtorForwarderData
{
    // underlying caller context
    mem_dtor_fn     m_user_dtor_func;
    void*           m_user_data;
    long         m_count;

};


void CL_CALLBACK CrtMemDtorForwarder(cl_mem m, void* userData)
{
    CrtMemDtorForwarderData* memData = (CrtMemDtorForwarderData*)userData;

    if (0 == atomic_decrement(&(memData->m_count)))
    {
        memData->m_user_dtor_func(m, memData->m_user_data);
        delete memData;
    }
}

void CL_CALLBACK buildCompleteFn( cl_program program, void *userData )
{
    CrtBuildCallBackData *data  = ( CrtBuildCallBackData* ) userData;
    cl_program pgm              = data->m_clProgramHandle;

    // Check if this is the last build routine to finish
    if (0 == atomic_decrement(&(data->m_numBuild)))
    {
        if( data->m_pfnNotify )
        {
            data->m_pfnNotify( pgm, data->m_userData );
            delete data;
        }
        else
        {
            data->m_lock.Signal();
        }
    }
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtContext::CrtContext(
    cl_context                      context_handle,
    const cl_context_properties *   properties,
    cl_uint                         num_devices,
    const cl_device_id *            devices,
    ctxt_logging_fn                 pfn_notify,
    void *                          user_data,
    cl_int *                        errcode_ret):
m_context_handle(context_handle)
{
    cl_int errCode = CL_SUCCESS;

    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    cl_device_id* outDevices = NULL;

    for (cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++)
    {
        outDevices = new cl_device_id[OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.size()];
        if( NULL == outDevices )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            break;
        }

        cl_uint matchDevices = 0;
        GetDevicesByPlatformId(num_devices,
                            devices,
                            OCLCRT::crt_ocl_module.m_oclPlatforms[i]->m_platformIdDEV,
                            &matchDevices,
                            outDevices);

        if (matchDevices == 0)
        {
            if( outDevices )
            {
                delete[] outDevices;
                outDevices = NULL;
            }
            continue;
        }

        CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(outDevices[0]);

        if( NULL == devInfo )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            break;
        }

        cl_context_properties* props;
        if( CRT_FAIL == OCLCRT::ReplacePlatformId( properties, devInfo->m_crtPlatform->m_platformIdDEV, &props ) )
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            break;
        }
        // It doesn't matter which device we pick from outDevices
        cl_context ctx = devInfo->m_origDispatchTable.clCreateContext(
                                props,
                                matchDevices,
                                outDevices,
                                pfn_notify,
                                user_data,
                                &errCode);

        if( CL_SUCCESS != errCode )
        {
            break;
        }

        if( devInfo->m_devType == CL_DEVICE_TYPE_GPU )
        {
            ( ( _cl_gpu_context* ) ctx )->isSharedContext = CL_TRUE;
        }

        for( cl_uint j=0; j < matchDevices; j++ )
        {
            m_DeviceToContext[outDevices[j]] = ctx;
        }
        m_contexts[ctx] = (OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(outDevices[0])->m_origDispatchTable);

        if( outDevices )
        {
            delete[] outDevices;
            outDevices = NULL;
        }
    }

    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

FINISH:
    if( outDevices )
    {
        delete[] outDevices;
        outDevices = NULL;
    }
    if( errcode_ret )
    {
        *errcode_ret = errCode;
    }
}


cl_device_id CrtContext::GetDeviceByType( cl_device_type device_type )
{
    cl_device_id device = NULL;

    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    for( OCLCRT::DEV_INFO_MAP::const_iterator itr = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().begin();
        itr != OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().end();
        itr++ )
    {
        if( itr->second->m_devType == device_type )
        {
            device = itr->first;
            break;
        }
    }
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();
    return device;
}

void CrtContext::GetDevicesByPlatformId(
    const cl_uint           inNumDevices,
    const cl_device_id*     inDevices,
    const cl_platform_id&   pId,
    cl_uint*                outNumDevices,
    cl_device_id*           outDevices)
{
    cl_int devCount = 0;
    for( cl_uint i=0; i < inNumDevices; i++ )
    {
        CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( inDevices[i] );
        if( ( devInfo != NULL ) && ( devInfo->m_crtPlatform->m_platformIdDEV == pId ) )
        {
            if( !outDevices )
            {
                devCount++;
                continue;
            }
            outDevices[devCount++] = inDevices[i];
        }
    }
    *outNumDevices = devCount;
}

void CrtContext::GetDevsIndicesByPlatformId(
    const cl_uint           inNumDevices,
    const cl_device_id*     inDevices,
    const cl_platform_id&   pId,
    cl_uint*                outNumIndices,
    cl_uint*                indices)
{
    cl_int indexCount = 0;
    for( cl_uint i=0; i < inNumDevices; i++ )
    {
        CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue( inDevices[i] );
        if( ( devInfo != NULL ) && ( devInfo->m_crtPlatform->m_platformIdDEV == pId ) )
        {
            if (!indices)
            {
                indexCount++;
                continue;
            }
            indices[indexCount++] = i;
        }
    }
    *outNumIndices = indexCount;
}


cl_int CrtContext::Release()
{
    cl_int errCode = CL_SUCCESS;

    SHARED_CTX_DISPATCH::iterator itr = m_contexts.begin();
    for( ;itr != m_contexts.end(); itr++ )
    {
        errCode = itr->second.clReleaseContext(itr->first);
        if( CL_SUCCESS != errCode )
        {
            return errCode;
        }
    }
    return errCode;
}

cl_int CrtContext::GetReferenceCount(cl_uint* refCountParam)
{
    cl_int errCode = CL_SUCCESS;
    cl_uint refCount = 0;

    SHARED_CTX_DISPATCH::iterator itr = m_contexts.begin();
    errCode = itr->second.clGetContextInfo(
        itr->first,
        CL_CONTEXT_REFERENCE_COUNT,
        sizeof(cl_uint),
        &refCount,
        NULL);

    *refCountParam = refCount;
    return errCode;
}

CrtContext::~CrtContext()
{
    m_contexts.clear();
    m_HostcommandQueues.clear();
}

cl_int CrtContext::FlushQueues()
{
    OCLCRT::Utils::OclAutoMutex CS(&m_mutex);   // Critical section

    cl_uint errCode = CL_SUCCESS;
    for( std::list<cl_command_queue>::iterator itr = m_HostcommandQueues.begin();
        itr != m_HostcommandQueues.end();
        itr++ )
    {
        const cl_command_queue q = *itr;
        errCode = q->dispatch->clFlush(q);
        if( CL_SUCCESS != errCode )
        {
            return errCode;
        }
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtMemObject::CrtMemObject(cl_mem_flags flags, void* hostPtr, CrtContext* ctx):
m_numValidContextObjs( 0 ),
m_memHandle( NULL )
{
    m_flags = flags;
    m_pUsrPtr = NULL;

    if( flags & ( CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR ) && hostPtr )
        m_pUsrPtr = hostPtr;

    m_pBstPtr = NULL;
    m_pContext = ctx;
    m_mapCount = 0;

    m_pContext->IncPendencyCnt();
}


CrtMemObject::~CrtMemObject()
{
    m_ContextToMemObj.clear();

    // there are 3 cases we need to deallocate m_pBstPtr:
    //     1. no user pointer provided so we allocated backing store
    //     2. user point provided but not aligned, so we kept m_pUsrPtr
    if( m_pBstPtr &&
        ( !( m_flags & CL_MEM_USE_HOST_PTR ) || m_pUsrPtr ) )
    {
        ALIGNED_FREE(m_pBstPtr);
        m_pBstPtr = NULL;
    }
    m_pContext->DecPendencyCnt();
}


inline cl_mem CrtMemObject::getDeviceMemObj(cl_device_id deviceId)
{
    cl_context devCtx = m_pContext->m_DeviceToContext[deviceId];
    return m_ContextToMemObj[devCtx];
}


inline cl_mem CrtMemObject::getAnyValidDeviceMemObj()
{
    CTX_MEM_MAP::iterator itr = m_ContextToMemObj.begin();
    for( ; itr != m_ContextToMemObj.end(); itr++ )
    {
        if( ( IsValidImageFormat( itr->second ) == CL_TRUE ) &&
            ( IsValidMemObjSize( itr->second ) == CL_TRUE ) )
        {
            return itr->second;
        }
    }
    return NULL;
}

cl_int CrtMemObject::Release()
{
    CTX_MEM_MAP::iterator itr_first = m_ContextToMemObj.begin();
    if( itr_first == m_ContextToMemObj.end() )
    {
        // Already released previously
        return CL_SUCCESS;
    }
    while( ( itr_first != m_ContextToMemObj.end() ) && 
           ( ( IsValidImageFormat( itr_first->second ) != CL_TRUE ) ||
             ( IsValidMemObjSize( itr_first->second ) != CL_TRUE ) ) )
    {
        itr_first++;
    }
    if( itr_first == m_ContextToMemObj.end() )
    {
        // Already released previously
        return CL_SUCCESS;
    }
    itr_first->second->dispatch->clRetainMemObject( itr_first->second );

    cl_mem memObj = itr_first->second;
    for( CTX_MEM_MAP::iterator itr = m_ContextToMemObj.begin();
        itr != m_ContextToMemObj.end();
        itr++ )
    {
        if( ( IsValidImageFormat( itr->second ) == CL_TRUE ) &&
            ( IsValidMemObjSize( itr->second ) == CL_TRUE ) )
        {
            itr->second->dispatch->clReleaseMemObject( itr->second );
        }
    }
    m_ContextToMemObj.clear();
    memObj->dispatch->clReleaseMemObject(memObj);
    return CL_SUCCESS;
}


cl_int CrtMemObject::RegisterDestructorCallback(mem_dtor_fn memDtorFunc, void* user_data)
{
    cl_int errCode = CL_SUCCESS;

    // this memory is dealocated in the callback 'CrtMemDtorForwarder'
    CrtMemDtorForwarderData* memData = new CrtMemDtorForwarderData;
    if( memData == NULL )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        return errCode;
    }
    if( getObjectType() == CrtObject::CL_IMAGE )
    {
        // images might have image formats not supported by all devices
        memData->m_count = ((CrtImage*)this)->m_numValidContextObjs;
    }
    else
    {
        memData->m_count = (cl_uint)m_ContextToMemObj.size();
    }
    memData->m_user_dtor_func = memDtorFunc;
    memData->m_user_data = user_data;

    for( CTX_MEM_MAP::iterator itr = m_ContextToMemObj.begin();
        itr != m_ContextToMemObj.end();
        itr++ )
    {
        if( ( IsValidImageFormat( itr->second ) == CL_TRUE ) &&
            ( IsValidMemObjSize( itr->second ) == CL_TRUE ) )
        {
            errCode = itr->second->dispatch->clSetMemObjectDestructorCallback(
                itr->second,
                CrtMemDtorForwarder,
                (void*)memData);

            if( CL_SUCCESS != errCode )
            {
                return errCode;
            }
        }
    }
    return errCode;
}

cl_bool CrtMemObject::HasPrivateCopy()
{
    return ( m_pBstPtr && m_pUsrPtr );
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtBuffer::CrtBuffer(
    const size_t        size,
    cl_mem_flags        flags,
    void*               host_ptr,
    CrtContext*         ctx): CrtMemObject(flags, host_ptr, ctx)
{
    m_parentBuffer = NULL;
    m_size = size;
}

CrtBuffer::CrtBuffer(
    CrtMemObject*   parent_buffer,
    cl_mem_flags    flags,
    CrtContext*     ctx): CrtMemObject(flags, NULL, ctx)
{
    m_parentBuffer = parent_buffer;
    if( m_parentBuffer )
    {
        m_parentBuffer->IncPendencyCnt();
    }
}


CrtBuffer::~CrtBuffer()
{
    if( m_parentBuffer )
    {
        m_parentBuffer->DecPendencyCnt();
        m_parentBuffer = NULL;
        m_pBstPtr = NULL;
        m_pUsrPtr = NULL;
    }
}

cl_int CrtBuffer::Create(CrtMemObject** bufObj)
{
    CTX_MEM_MAP::iterator ctx_mem_itr;

    // HOST pointer provided and memory is aligned
    if( ( m_flags & CL_MEM_USE_HOST_PTR ) &&
        ( m_pUsrPtr != NULL ) &&
        ( ( (size_t)m_pUsrPtr & ( m_pContext->getAlignment( CrtObject::CL_BUFFER )-1 ) ) == 0 ) )
    {
        m_pBstPtr = m_pUsrPtr;
        m_pUsrPtr = NULL;
    }
    else if( m_pBstPtr == NULL && !m_pContext->memObjectAlwaysNotInSync( CrtObject::CL_BUFFER ) )
    {
        // Convert bits to bytes
        m_pBstPtr = ALIGNED_MALLOC( m_size, m_pContext->getAlignment( CrtObject::CL_BUFFER ) );

        if ( !m_pBstPtr )
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        if( ( m_pUsrPtr != NULL ) &&
            ( ( m_flags & CL_MEM_COPY_HOST_PTR ) || ( m_flags & CL_MEM_USE_HOST_PTR ) ) )
        {
            memcpy_s( m_pBstPtr, m_size, m_pUsrPtr, m_size );
        }

        if( !( m_flags & CL_MEM_USE_HOST_PTR ) )
        {
            // We don't need this any more; ex. CL_MEM_COPY_HOST_PTR case
            m_pUsrPtr = NULL;
        }
    }
    else
    {
        assert(0 && "No support for platforms were buffers isn't linear");
    }

    cl_int errCode = CL_SUCCESS;

    *bufObj = this;

    // This is being deallocated in callback 'CrtMemDtorCallBack'
    CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;
    if( NULL == memData )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    memData->m_count = 0;
    memData->m_clMemHandle = this;

    for( SHARED_CTX_DISPATCH::iterator itr = m_pContext->m_contexts.begin();
        itr != m_pContext->m_contexts.end();
        itr++ )
    {
        cl_context ctx = itr->first;
        cl_mem_flags crtCreateFlags = m_flags;

        crtCreateFlags |= CL_MEM_USE_HOST_PTR;
        crtCreateFlags &= ~CL_MEM_ALLOC_HOST_PTR;
        crtCreateFlags &= ~CL_MEM_COPY_HOST_PTR;
        crtCreateFlags &= ~CL_MEM_HOST_WRITE_ONLY;
        crtCreateFlags &= ~CL_MEM_HOST_READ_ONLY;
        crtCreateFlags &= ~CL_MEM_HOST_NO_ACCESS;

        cl_mem memObj = ctx->dispatch->clCreateBuffer(
            ctx,
            crtCreateFlags,
            m_size,
            m_pBstPtr,
            &errCode);

        if( CL_SUCCESS == errCode )
        {
            m_ContextToMemObj[ctx] = memObj;
            m_numValidContextObjs++;
        }
        else if( CL_INVALID_BUFFER_SIZE == errCode )
        {
            // there might be other device in the context supporting
            // this image format
            m_ContextToMemObj[ctx] = (cl_mem)INVALID_MEMOBJ_SIZE;
            continue;
        }
        else
        {
            goto FINISH;
        }
    }

    // if no device supports the image format, then m_ContextToMemObj
    // will be an empty map, and we will continue directly to :FINISH
    // and finally returning CL_IMAGE_FORMAT_NOT_SUPPORTED error code.
    ctx_mem_itr = m_ContextToMemObj.begin();
    for ( ; ctx_mem_itr != m_ContextToMemObj.end(); ctx_mem_itr++ )
    {
        if( IsValidMemObjSize( ctx_mem_itr->second ) == CL_TRUE )
        {
            errCode = ctx_mem_itr->second->dispatch->clSetMemObjectDestructorCallback(
                ctx_mem_itr->second,
                CrtMemDestructorCallBack,
                (void*)memData);

            if( CL_SUCCESS != errCode )
            {
                goto FINISH;
            }
            memData->m_count++;
        }
    }

FINISH:

    if( CL_SUCCESS != errCode )
    {
        if( memData->m_count == 0 )
        {
            delete memData;
        }
        Release();
        *bufObj = NULL;
    }
    return errCode;
}

cl_int CrtBuffer::Create(
    CrtMemObject**          bufObj,
    cl_buffer_create_type   buffer_create_type,
    const void *            buffer_create_info)
{
    cl_int errCode = CL_SUCCESS;
    CTX_MEM_MAP::iterator itr;

    CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;
    if( NULL == memData )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    memData->m_count = 0;
    memData->m_clMemHandle = this;

    // Validate sub-buffer alignment
    const cl_buffer_region* region = reinterpret_cast<const cl_buffer_region*>(buffer_create_info);

    itr = m_parentBuffer->m_ContextToMemObj.begin();
    for( ; itr != m_parentBuffer->m_ContextToMemObj.end(); itr++ )
    {
        if( IsValidMemObjSize( itr->second ) != CL_TRUE )
        {
            m_ContextToMemObj[itr->first] = itr->second;
            continue;
        }

        cl_mem_flags crtCreateFlags = m_flags;

        crtCreateFlags &= ~CL_MEM_HOST_WRITE_ONLY;
        crtCreateFlags &= ~CL_MEM_HOST_READ_ONLY;
        crtCreateFlags &= ~CL_MEM_HOST_NO_ACCESS;

        cl_mem memObj = itr->first->dispatch->clCreateSubBuffer(
            itr->second,
            crtCreateFlags,
            buffer_create_type,
            buffer_create_info,
            &errCode );

        if( CL_SUCCESS == errCode )
        {
            m_ContextToMemObj[itr->first] = memObj;
            m_numValidContextObjs++;
        }
        else if( CL_MISALIGNED_SUB_BUFFER_OFFSET == errCode )
        {
            m_ContextToMemObj[itr->first] = (cl_mem)INVALID_MEMOBJ_SIZE;
            continue;
        }
        else
        {
            goto FINISH;
        }
    }

    if( 0 == m_numValidContextObjs )
    {
        goto FINISH;
    }

    m_pBstPtr = ( cl_char* )m_parentBuffer->m_pBstPtr + region->origin;
    m_pUsrPtr = ( ( m_parentBuffer->m_pUsrPtr == NULL ) ?
                    NULL :
                    ( cl_char* )m_parentBuffer->m_pUsrPtr + region->origin );

    m_size = region->size;

    itr = m_ContextToMemObj.begin();
    for( ; itr != m_ContextToMemObj.end(); itr++ )
    {
        if( IsValidMemObjSize( itr->second ) == CL_TRUE )
        {
            errCode = itr->second->dispatch->clSetMemObjectDestructorCallback(
                itr->second,
                CrtMemDestructorCallBack,
                (void*)memData );

            if( CL_SUCCESS != errCode )
            {
                goto FINISH;
            }
            memData->m_count++;
        }
    }

FINISH:

    if( CL_SUCCESS != errCode )
    {
        if( memData && ( memData->m_count == 0 ) )
        {
            delete memData;
        }
        Release();
        *bufObj = NULL;
    }

    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
void*  CrtBuffer::GetMapPointer(const size_t* origin, const size_t* region)
{
    char* mapPtr = NULL;
    if( this->HasPrivateCopy() )
    {
        mapPtr = (char*)m_pUsrPtr + *origin;
    }
    else
    {
        mapPtr = (char*)m_pBstPtr + *origin;
    }
    return mapPtr;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CrtBuffer::CheckParamsAndBounds(const size_t* origin, const size_t* region)
{
    if( m_size < ( *origin + *region ) )
    {
         return CL_INVALID_VALUE;
    }
    return CL_SUCCESS;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtGLBuffer::CrtGLBuffer(
    cl_bool             isRenderBuffer,
    cl_mem_flags        flags,
    GLuint              bufobj,
    CrtContext*         ctx):
CrtBuffer(0,flags, NULL, ctx)
{
    m_isRenderBuffer = isRenderBuffer;
    m_glBufObj = bufobj;
}

cl_int CrtGLBuffer::Create(CrtMemObject** bufObj)
{
    CTX_MEM_MAP::iterator ctx_mem_itr;
    cl_int errCode = CL_SUCCESS;

    *bufObj = this;

    // This is being deallocated in callback 'CrtMemDtorCallBack'
    CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;
    if( NULL == memData )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    memData->m_count = 0;
    memData->m_clMemHandle = this;

    for( SHARED_CTX_DISPATCH::iterator itr = m_pContext->m_contexts.begin();
        itr != m_pContext->m_contexts.end();
        itr++ )
    {
        cl_context ctx = itr->first;
        cl_mem memObj = NULL;

        if( m_isRenderBuffer )
        {
            memObj = ctx->dispatch->clCreateFromGLRenderbuffer(
                ctx,
                m_flags,
                m_glBufObj,
                &errCode);
        }
        else
        {
            memObj = ctx->dispatch->clCreateFromGLBuffer(
                ctx,
                m_flags,
                m_glBufObj,
                &errCode);
        }

        if( CL_SUCCESS == errCode )
        {
            m_ContextToMemObj[ctx] = memObj;
            m_numValidContextObjs++;
        }
        else if( CL_INVALID_BUFFER_SIZE == errCode )
        {
            // there might be other device in the context supporting
            // this image format
            m_ContextToMemObj[ctx] = (cl_mem)INVALID_MEMOBJ_SIZE;
            continue;
        }
        else
        {
            goto FINISH;
        }
    }

    // if no device supports the image format, then m_ContextToMemObj
    // will be an empty map, and we will continue directly to :FINISH
    // and finally returning CL_IMAGE_FORMAT_NOT_SUPPORTED error code.
    ctx_mem_itr = m_ContextToMemObj.begin();
    for ( ; ctx_mem_itr != m_ContextToMemObj.end(); ctx_mem_itr++ )
    {
        if( IsValidMemObjSize( ctx_mem_itr->second ) == CL_TRUE )
        {
            errCode = ctx_mem_itr->second->dispatch->clSetMemObjectDestructorCallback(
                ctx_mem_itr->second,
                CrtMemDestructorCallBack,
                (void*)memData);

            if( CL_SUCCESS != errCode )
            {
                goto FINISH;
            }
            memData->m_count++;
        }
    }

FINISH:

    if( CL_SUCCESS != errCode )
    {
        if( memData->m_count == 0 )
        {
            delete memData;
        }
        Release();
        *bufObj = NULL;
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
#ifdef _WIN32
CrtDX9MediaSurface::CrtDX9MediaSurface(
    cl_mem_flags            flags,
    IDirect3DSurface9*      resource,
    HANDLE                  sharedhandle,
    UINT                    plane,
    CrtContext*             ctx):
CrtMemObject(flags, NULL, ctx)
{
    m_resource = resource;
    m_sharedHandle = sharedhandle;
    m_plane = plane;
}

cl_int CrtDX9MediaSurface::Create( CrtMemObject** dx9Obj )
{
    CTX_MEM_MAP::iterator ctx_mem_itr;
    cl_int errCode = CL_SUCCESS;

    *dx9Obj = this;

    // This is being deallocated in callback 'CrtMemDtorCallBack'
    CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;
    if( NULL == memData )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    memData->m_count = 0;
    memData->m_clMemHandle = this;

    for( SHARED_CTX_DISPATCH::iterator itr = m_pContext->m_contexts.begin();
        itr != m_pContext->m_contexts.end();
        itr++ )
    {
        cl_context ctx = itr->first;
        cl_mem memObj = NULL;

        memObj = ( (SOCLEntryPointsTable*)ctx )->crtDispatch->clCreateFromDX9MediaSurfaceINTEL(
            ctx,
            m_flags,
            m_resource,
            m_sharedHandle,
            m_plane,
            &errCode);


        if( CL_SUCCESS == errCode )
        {
            m_ContextToMemObj[ctx] = memObj;
            m_numValidContextObjs++;
        }
        else
        {
            goto FINISH;
        }
    }

    // if no device supports the image format, then m_ContextToMemObj
    // will be an empty map, and we will continue directly to :FINISH
    // and finally returning CL_IMAGE_FORMAT_NOT_SUPPORTED error code.
    ctx_mem_itr = m_ContextToMemObj.begin();
    for ( ; ctx_mem_itr != m_ContextToMemObj.end(); ctx_mem_itr++ )
    {
        if( IsValidMemObjSize( ctx_mem_itr->second ) == CL_TRUE )
        {
            errCode = ctx_mem_itr->second->dispatch->clSetMemObjectDestructorCallback(
                ctx_mem_itr->second,
                CrtMemDestructorCallBack,
                (void*)memData);

            if( CL_SUCCESS != errCode )
            {
                goto FINISH;
            }
            memData->m_count++;
        }
    }

FINISH:

    if( CL_SUCCESS != errCode )
    {
        if( memData->m_count == 0 )
        {
            delete memData;
        }
        Release();
        *dx9Obj = NULL;
    }
    return errCode;
}
#endif
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtImage::CrtImage(
    cl_mem_flags            flags,
    CrtContext*             ctx):
CrtMemObject(flags,NULL,ctx)
{
    m_imageDesc.crtBuffer = NULL;
}

CrtImage::CrtImage(
    const cl_image_format * image_format,
    const cl_image_desc *   image_desc,
    cl_mem_flags            flags,
    void*                   host_ptr,
    CrtContext*             ctx):
CrtMemObject(flags, host_ptr, ctx),
m_imageFormat(*image_format),
m_dimCount(0)
{
    m_imageDesc.desc = *image_desc;

    m_hostPtrRowPitch = m_imageDesc.desc.image_row_pitch ?
        m_imageDesc.desc.image_row_pitch:
        ( (cl_uint)m_imageDesc.desc.image_width * GetImageElementSize(&m_imageFormat) );

    m_hostPtrSlicePitch = m_imageDesc.desc.image_slice_pitch ?
        m_imageDesc.desc.image_slice_pitch :
        ( m_hostPtrRowPitch * m_imageDesc.desc.image_height );

    switch( m_imageDesc.desc.image_type )
    {
        case CL_MEM_OBJECT_IMAGE1D:
        case CL_MEM_OBJECT_IMAGE1D_BUFFER:
            m_imageDesc.desc.image_height = 1;
            m_imageDesc.desc.image_depth = 1;
            m_imageDesc.desc.image_array_size = 1;
            break;
        case CL_MEM_OBJECT_IMAGE1D_ARRAY:
            m_imageDesc.desc.image_height = 1;
            m_imageDesc.desc.image_depth = 1;
            break;
        case CL_MEM_OBJECT_IMAGE2D:
            m_imageDesc.desc.image_depth = 1;
            m_imageDesc.desc.image_array_size = 1;
            break;
        case CL_MEM_OBJECT_IMAGE2D_ARRAY:
            m_imageDesc.desc.image_depth = 1;
            break;
        default:
             m_imageDesc.desc.image_array_size = 1;
             break;
    }

    switch( m_imageDesc.desc.image_type )
    {
        case CL_MEM_OBJECT_IMAGE1D:
        case CL_MEM_OBJECT_IMAGE1D_BUFFER:
            m_dimCount = 1;
            break;
        case CL_MEM_OBJECT_IMAGE2D:
        case CL_MEM_OBJECT_IMAGE1D_ARRAY:
            m_dimCount = 2;
            break;
        case CL_MEM_OBJECT_IMAGE3D:
        case CL_MEM_OBJECT_IMAGE2D_ARRAY:
            m_dimCount = 3;
            break;
        default:
            assert(0 && "Not supported image type by CRT");
            break;
    }

    if( m_imageDesc.desc.image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER )
    {
        m_imageDesc.crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)m_imageDesc.desc.mem_object)->object);
        m_imageDesc.crtBuffer->IncPendencyCnt();
        // Don't query GMM for Image From Buffer
        return;
    }
    else
    {
        m_imageDesc.crtBuffer = NULL;
        m_imageDesc.desc.mem_object = NULL;
    }
    cl_device_id gpuDevice = ctx->GetDeviceByType( CL_DEVICE_TYPE_GPU );
    if( gpuDevice != NULL )
    {
        SOCLEntryPointsTable* gpuDispatch = (SOCLEntryPointsTable*)gpuDevice;

        cl_int RetVal = gpuDispatch->crtDispatch->clGetImageParamsINTEL(
                                             ctx->GetContextByDeviceID( gpuDevice ),
                                             &m_imageFormat,
                                             ( cl_image_desc* )( &m_imageDesc ),
                                             &m_imageDesc.desc.image_row_pitch,
                                             &m_imageDesc.desc.image_slice_pitch );

        if( RetVal != CL_SUCCESS )
        {
            if( RetVal == CL_IMAGE_FORMAT_NOT_SUPPORTED )
            {
                m_imageDesc.desc.image_row_pitch = m_imageDesc.desc.image_width * GetImageElementSize( &m_imageFormat );
                m_imageDesc.desc.image_slice_pitch  = m_imageDesc.desc.image_row_pitch * m_imageDesc.desc.image_height;                
            }
            else
            {
                assert( 0 && "clGetImageParamsINTEL has failed with non-expected error" );
            }
        }

        assert(
            ( m_imageDesc.desc.image_type == CL_MEM_OBJECT_IMAGE1D )            ||
            ( m_imageDesc.desc.image_type == CL_MEM_OBJECT_IMAGE2D )            ||
            ( m_imageDesc.desc.image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER )     ||
            !( m_imageDesc.desc.image_slice_pitch % m_imageDesc.desc.image_row_pitch ));
    }
    else
    {
        assert( ( gpuDevice != NULL ) && "Image sharing requires GPU device" );
    }
}

void*  CrtImage::GetMapPointer(const size_t* origin, const size_t* region)
{
    char* mapPtr = NULL;
    size_t elemSize = GetImageElementSize(&m_imageFormat);

    if( !HasPrivateCopy() )
    {
        mapPtr = (char*)m_pBstPtr;
    }
    else
    {
        mapPtr = (char*)m_pUsrPtr;
    }

    mapPtr += origin[0] * elemSize;
    mapPtr += origin[1] * m_hostPtrRowPitch;
    mapPtr += origin[2] * m_hostPtrSlicePitch;
    return mapPtr;
}

cl_int CrtImage::CheckParamsAndBounds(const size_t* origin, const size_t* region)
{

    if( (origin[0] + region[0] ) > m_imageDesc.desc.image_width )
    {
        return CL_INVALID_VALUE;
    }

    if( m_imageDesc.desc.image_type != CL_MEM_OBJECT_IMAGE1D_ARRAY )
    {
        if( (origin[1] + region[1]) > m_imageDesc.desc.image_height )
        {
            return CL_INVALID_VALUE;
        }
    }
    switch( m_imageDesc.desc.image_type )
    {
        case CL_MEM_OBJECT_IMAGE1D:
        case CL_MEM_OBJECT_IMAGE1D_BUFFER:
            if( origin[1] != 0 || region[1] != 1 ||
                origin[2] != 0 || region[2] != 1 )
            {
                return CL_INVALID_VALUE;
            }
            break;
        case CL_MEM_OBJECT_IMAGE2D:
        case CL_MEM_OBJECT_IMAGE1D_ARRAY:
             if( origin[2] != 0 || region[2] != 1 )
             {
                return CL_INVALID_VALUE;
             }
             break;
        case CL_MEM_OBJECT_IMAGE3D:
            if( ( origin[2] + region[2] ) > m_imageDesc.desc.image_depth )
            {
                return CL_INVALID_VALUE;
            }
            break;
        default:
            break;
    }
    return CL_SUCCESS;
}

CrtImage::~CrtImage()
{
    if( m_imageDesc.crtBuffer )
    {
        m_imageDesc.crtBuffer->DecPendencyCnt();
        m_pBstPtr = NULL;
        m_pUsrPtr = NULL;
    }
}


cl_int CrtImage::Create(CrtMemObject**  imageObj)
{
    CTX_MEM_MAP::iterator ctx_mem_itr;
    CrtBuffer* crtBuffer = NULL;

    // Calculate the size of memory provided by the app
    m_size = m_imageDesc.desc.image_slice_pitch * m_imageDesc.desc.image_depth * m_imageDesc.desc.image_array_size;
    size_t hostPtrSize = m_hostPtrSlicePitch * m_imageDesc.desc.image_depth * m_imageDesc.desc.image_array_size;

    // We need to save these params since, we need use them
    // on Map in case CL_MEM_USE_HOST_PTR has been provided.
    if( m_imageDesc.desc.image_type == CL_MEM_OBJECT_IMAGE1D_ARRAY )
    {
        m_imageDesc.desc.image_row_pitch = m_imageDesc.desc.image_slice_pitch;
    }
    else if( m_dimCount < 3 )
    {
        m_hostPtrSlicePitch = 0;
    }

    //  for using the provided host ptr, we need to make sure:
    //     1. size of image provided in host_ptr is big enough to fit the image.
    //     2. host_ptr pointer is aligned to PAGE_ALIGNMENT
    //     3. App provided enough space to accommodate the size requirement of underlying devices
    if( m_imageDesc.desc.image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER )
    {
        crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)m_imageDesc.desc.mem_object)->object);
        m_pBstPtr = crtBuffer->m_pBstPtr;
        m_pUsrPtr = crtBuffer->m_pUsrPtr;
        m_size = crtBuffer->m_size;
    }
    else if( ( m_flags & CL_MEM_USE_HOST_PTR ) &&
        ( m_pUsrPtr ) &&
        ( ( ( size_t ) m_pUsrPtr & ( m_pContext->getAlignment( CrtObject::CL_IMAGE )-1 ) ) == 0 ) &&
        ( hostPtrSize >= m_size ) )
    {
        m_pBstPtr = m_pUsrPtr;
        m_pUsrPtr = NULL;
        m_hostPtrRowPitch = m_imageDesc.desc.image_row_pitch;
        m_hostPtrSlicePitch = m_imageDesc.desc.image_slice_pitch;
    }
    else if( ( m_pBstPtr == NULL ) &&
             ( m_pContext->memObjectAlwaysNotInSync( CrtObject::CL_IMAGE ) == false ) )
    {
        // Convert bits to bytes
        m_pBstPtr = ALIGNED_MALLOC( m_size, m_pContext->getAlignment( CrtObject::CL_IMAGE ) );

        if( !m_pBstPtr )
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        if( ( m_flags & CL_MEM_COPY_HOST_PTR ) ||
            ( m_flags & CL_MEM_USE_HOST_PTR ) )
        {
            MemCpyParams            sCpyParam;

            sCpyParam.pDst = (cl_char*)m_pBstPtr;
            sCpyParam.pSrc = (cl_char*)m_pUsrPtr;
            sCpyParam.uiDimCount = 3;

            sCpyParam.vSrcPitch[0] = m_hostPtrRowPitch;
            sCpyParam.vSrcPitch[1] = m_hostPtrSlicePitch;

            sCpyParam.vDstPitch[0] = m_imageDesc.desc.image_row_pitch;
            sCpyParam.vDstPitch[1] = m_imageDesc.desc.image_slice_pitch;

            sCpyParam.vRegion[0] = m_imageDesc.desc.image_width * GetImageElementSize(&m_imageFormat);
            sCpyParam.vRegion[1] = m_imageDesc.desc.image_height;
            sCpyParam.vRegion[2] = m_imageDesc.desc.image_depth * m_imageDesc.desc.image_array_size;

            CopyMemoryObject(&sCpyParam);

            if( !( m_flags & CL_MEM_USE_HOST_PTR ) )
            {
                // We don't need this any more; ex. CL_MEM_COPY_HOST_PTR case
                m_pUsrPtr = NULL;
            }
        }
    }
    else
    {
        assert( 0 && "No support for platforms where buffers aren't linear" );
    }

    if( !( m_flags & CL_MEM_USE_HOST_PTR ) )
    {
        // In case of not using USE_HOST_PTR flag; we can return
        // our chosen pitches and not necessarily stick to
        // whatever the user proivided
        m_hostPtrRowPitch = m_imageDesc.desc.image_row_pitch;
        m_hostPtrSlicePitch = m_imageDesc.desc.image_slice_pitch;
    }

    cl_int errCode = CL_SUCCESS;

    *imageObj = this;

    // This is being deallocated in callback 'CrtMemDtorCallBack'
    CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;
    if ( NULL == memData )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    memData->m_count = 0;
    memData->m_clMemHandle = this;

    SHARED_CTX_DISPATCH::iterator itr = m_pContext->m_contexts.begin();
    for( ;itr != m_pContext->m_contexts.end(); itr++ )
    {
        cl_context ctx = itr->first;
        cl_mem_flags crtCreateFlags = m_flags;

        if( crtBuffer == NULL )
        {
            // - Force using Linear images
            // - some flags are mutual execlusive, so we need to turn off CL_MEM_COPY_HOST_PTR
            //     and CL_MEM_ALLOC_HOST_PTR since we are forcing CL_MEM_USE_HOST_PTR
            crtCreateFlags |= CL_MEM_USE_HOST_PTR;
            crtCreateFlags &= ~CL_MEM_ALLOC_HOST_PTR;
            crtCreateFlags &= ~CL_MEM_COPY_HOST_PTR;
            crtCreateFlags &= ~CL_MEM_HOST_WRITE_ONLY;
            crtCreateFlags &= ~CL_MEM_HOST_READ_ONLY;
            crtCreateFlags &= ~CL_MEM_HOST_NO_ACCESS;
        }

        cl_mem memObj  = NULL;        
        cl_image_desc* imageDesc = &m_imageDesc.desc;
        cl_image_desc imageDescCopy;
        if( crtBuffer )
        {
            // In case of Image from Buffer; we need to replace the buffer
            // handle with the underlying corresponding one.
            imageDescCopy = m_imageDesc.desc;
            imageDescCopy.mem_object = m_imageDesc.crtBuffer->m_ContextToMemObj[ ctx ];
            imageDesc = ( cl_image_desc* )( &imageDescCopy );
        }

        memObj = ctx->dispatch->clCreateImage(
            ctx,
            crtCreateFlags,
            &m_imageFormat,
            imageDesc,
            ( crtBuffer ? NULL : m_pBstPtr ),
            &errCode);        

        if( ( CL_SUCCESS == errCode ) &&
            ( memObj != NULL ) )
        {
            m_ContextToMemObj[ctx] = memObj;
            m_numValidContextObjs++;
        }
        else if( CL_IMAGE_FORMAT_NOT_SUPPORTED == errCode )
        {
            // there might be other device in the context supporting
            // this image format
            m_ContextToMemObj[ctx] = (cl_mem)INVALID_IMG_FORMAT;
            continue;
        }
        else if( CL_INVALID_IMAGE_SIZE == errCode )
        {
            // there might be other device in the context supporting
            // this image format
            m_ContextToMemObj[ctx] = (cl_mem)INVALID_MEMOBJ_SIZE;
            continue;
        }
        else
        {
            goto FINISH;
        }
    }
    // if no device supports the image format, then m_ContextToMemObj
    // will be an empty map, and we will continue directly to :FINISH
    // and finally returning CL_IMAGE_FORMAT_NOT_SUPPORTED error code.
    ctx_mem_itr = m_ContextToMemObj.begin();
    for( ; ctx_mem_itr != m_ContextToMemObj.end(); ctx_mem_itr++ )
    {
        if( ( IsValidImageFormat( ctx_mem_itr->second ) == CL_TRUE ) &&
            ( IsValidMemObjSize( ctx_mem_itr->second ) == CL_TRUE ) )
        {
            errCode = ctx_mem_itr->second->dispatch->clSetMemObjectDestructorCallback(
                ctx_mem_itr->second,
                CrtMemDestructorCallBack,
                (void*)memData );

            if( CL_SUCCESS != errCode )
            {
                goto FINISH;
            }
            memData->m_count++;
        }
    }
FINISH:
    if( memData->m_count == 0 )
    {
         delete memData;
    }
    if( CL_SUCCESS != errCode )
    {
        Release();
        *imageObj = NULL;
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtGLImage::CrtGLImage(    
    cl_uint                 dim_count,
    cl_mem_flags            flags,
    GLenum                  texture_target,
    GLint                   miplevel,
    GLuint                  texture,
    CrtContext*           ctx):
CrtImage(flags, ctx)
{    
    m_dimCount      = dim_count;
    m_textureTarget = texture_target;
    m_mipLevel      = miplevel;
    m_texture       = texture;    
}

CrtGLImage::~CrtGLImage()
{    
}

cl_int CrtGLImage::Create(CrtMemObject**  imageObj)
{
    CTX_MEM_MAP::iterator ctx_mem_itr;
    cl_int errCode = CL_SUCCESS;

    *imageObj = this;

    // This is being deallocated in callback 'CrtMemDtorCallBack'
    CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;
    if ( NULL == memData )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    memData->m_count = 0;
    memData->m_clMemHandle = this;
    
    SHARED_CTX_DISPATCH::iterator itr = m_pContext->m_contexts.begin(); 
    for( ;itr != m_pContext->m_contexts.end(); itr++ )
    {
        cl_context ctx = itr->first;        
        cl_mem memObj  = NULL;
       
        switch( m_dimCount )
        {
        case 2:                         
            memObj = ctx->dispatch->clCreateFromGLTexture2D(
                ctx,
                m_flags,
                m_textureTarget,
                m_mipLevel,
                m_texture,
                &errCode);          
            break;
        case 3:
            memObj = ctx->dispatch->clCreateFromGLTexture3D(
                ctx,
                m_flags,
                m_textureTarget,
                m_mipLevel,
                m_texture,
                &errCode);
            break;
        default:
            memObj = ctx->dispatch->clCreateFromGLTexture(
                ctx,
                m_flags,
                m_textureTarget,
                m_mipLevel,
                m_texture,
                &errCode);
            break;
        };
        
        if( ( CL_SUCCESS == errCode ) &&
            ( memObj != NULL ) )
        {
            m_ContextToMemObj[ctx] = memObj;
            m_numValidContextObjs++;
        }
        else if( CL_IMAGE_FORMAT_NOT_SUPPORTED == errCode )
        {
            // there might be other device in the context supporting
            // this image format
            m_ContextToMemObj[ctx] = (cl_mem)INVALID_IMG_FORMAT;
            continue;
        }
        else if( CL_INVALID_IMAGE_SIZE == errCode )
        {
            // there might be other device in the context supporting
            // this image format
            m_ContextToMemObj[ctx] = (cl_mem)INVALID_MEMOBJ_SIZE;
            continue;
        }
        else
        {
            goto FINISH;
        }
    }
    // if no device supports the image format, then m_ContextToMemObj
    // will be an empty map, and we will continue directly to :FINISH
    // and finally returning CL_IMAGE_FORMAT_NOT_SUPPORTED error code.
    ctx_mem_itr = m_ContextToMemObj.begin();
    for( ; ctx_mem_itr != m_ContextToMemObj.end(); ctx_mem_itr++ )
    {
        if( ( IsValidImageFormat( ctx_mem_itr->second ) == CL_TRUE ) &&
            ( IsValidMemObjSize( ctx_mem_itr->second ) == CL_TRUE ) )
        {           
            errCode = ctx_mem_itr->second->dispatch->clSetMemObjectDestructorCallback(
                ctx_mem_itr->second,
                CrtMemDestructorCallBack,
                (void*)memData );

            if( CL_SUCCESS != errCode )
            {
                goto FINISH;
            }
            memData->m_count++;
        }
    }
FINISH:
    if( memData->m_count == 0 )
    {
         delete memData;
    }
    if( CL_SUCCESS != errCode )
    {
        Release();
        *imageObj = NULL;
    }
    return errCode;
}

size_t GetImageElementSize(const cl_image_format *  format)
{
    size_t stChannels = 0;
    size_t stChSize = 0;
    switch (format->image_channel_order)
    {
        case CL_R:
        case CL_A:
        case CL_LUMINANCE:
        case CL_INTENSITY:
        case CL_RGB:    // Special case, must be used only with specific data type
        case CL_DEPTH:
            stChannels = 1;
            break;
        case CL_RG:
        case CL_RA:
        case CL_DEPTH_STENCIL:
            stChannels = 2;
            break;
        case CL_RGBA:
        case CL_ARGB:
        case CL_BGRA:
            stChannels = 4;
            break;
        default:
            assert(0);
    }
    switch (format->image_channel_data_type)
    {
        case (CL_SNORM_INT8):
        case (CL_UNORM_INT8):
        case (CL_SIGNED_INT8):
        case (CL_UNSIGNED_INT8):
                stChSize = 1;
                break;
        case (CL_SNORM_INT16):
        case (CL_UNORM_INT16):
        case (CL_UNSIGNED_INT16):
        case (CL_SIGNED_INT16):
        case (CL_HALF_FLOAT):
        case CL_UNORM_SHORT_555:
        case CL_UNORM_SHORT_565:
        case CL_UNORM_INT24:
                stChSize = 2;
                break;
        case (CL_SIGNED_INT32):
        case (CL_UNSIGNED_INT32):
        case (CL_FLOAT):
        case CL_UNORM_INT_101010:
                stChSize = 4;
                break;
        default:
                assert(0);
    }
    return stChannels*stChSize;
}

CrtPipe::CrtPipe(
    const cl_uint               packetSize,
    const cl_uint               maxPackets,
    const cl_pipe_properties*   properties,
    cl_mem_flags                flags,
    CrtContext*                 ctx ):
CrtMemObject(flags, NULL, ctx),
m_packetSize(packetSize),
m_maxPackets(maxPackets),
m_properties(NULL)
{
    m_size = 0; // this will be initilized in CrtPipe::Create
}

cl_int CrtPipe::Create( CrtMemObject** memObj )
{
    cl_int                  errCode   = CL_SUCCESS;
    size_t                  allocSize = 0;
    cl_context              ctx       = NULL;
    CrtMemDtorCallBackData* memData   = NULL;

    CTX_MEM_MAP::iterator           ctx_mem_itr;
    SHARED_CTX_DISPATCH::iterator   ctx_itr;

    assert( NULL == m_pBstPtr && "Pipe already has backing store!" );

    // Ask underlying context for the required size for the pipe
    ctx = m_pContext->m_contexts.begin()->first;
    // TODO: should we check the return value of this?
    ( (SOCLEntryPointsTable*)ctx )->crtDispatch->clCreatePipeINTEL(
                                                    ctx,
                                                    m_flags,
                                                    m_packetSize,
                                                    m_maxPackets,
                                                    m_properties,
                                                    NULL,
                                                    &allocSize,
                                                    &errCode );

    if( CL_SUCCESS != errCode )
    {
        goto FINISH;
    }

    assert( 0 != allocSize && "The required size for the pipe shouldn't be zero!" );
    m_size = allocSize;

    // Allocate needed memory
    m_pBstPtr = ALIGNED_MALLOC( m_size, m_pContext->getAlignment( CrtObject::CL_PIPE ) );

    if( NULL == m_pBstPtr )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    *memObj = this;

    // This is being deallocated in callback 'CrtMemDtorCallBack'
    memData = new CrtMemDtorCallBackData;
    if( NULL == memData )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    memData->m_count = 0;
    memData->m_clMemHandle = this;

    // After we allocated the pipe's buffer in CRT we call both underlying contexts clCreatePipeINTEL
    // with the pointer to our allocated buffer
    for( ctx_itr = m_pContext->m_contexts.begin();
        ctx_itr != m_pContext->m_contexts.end();
        ctx_itr++ )
    {
        ctx = ctx_itr->first;

        cl_mem memObject = ( (SOCLEntryPointsTable*)ctx )->crtDispatch->clCreatePipeINTEL(
                                                                        ctx,
                                                                        m_flags,
                                                                        m_packetSize,
                                                                        m_maxPackets,
                                                                        m_properties,
                                                                        m_pBstPtr,
                                                                        &allocSize,
                                                                        &errCode );

        if( CL_SUCCESS == errCode )
        {
            m_ContextToMemObj[ctx] = memObject;
            m_numValidContextObjs++;
        }
        else if( CL_INVALID_PIPE_SIZE == errCode )
        {
            // there might be other device in the context that
            // succeed to create the pipe
            m_ContextToMemObj[ctx] = ( cl_mem )INVALID_MEMOBJ_SIZE;
            continue;
        }
        else
        {
            goto FINISH;
        }
    }

    ctx_mem_itr = m_ContextToMemObj.begin();
    for ( ; ctx_mem_itr != m_ContextToMemObj.end(); ctx_mem_itr++ )
    {
        if( CL_TRUE == IsValidMemObjSize( ctx_mem_itr->second ) )
        {
            errCode = ctx_mem_itr->second->dispatch->clSetMemObjectDestructorCallback(
                ctx_mem_itr->second,
                CrtMemDestructorCallBack,
                ( void* )memData );

            if( CL_SUCCESS != errCode )
            {
                goto FINISH;
            }
            memData->m_count++;
        }
    }

FINISH:
    if( CL_SUCCESS != errCode )
    {
        if( ( NULL != memData ) &&
            ( memData->m_count == 0 ) )
        {
            delete memData;
        }
        Release();
        *memObj = NULL;
    }
    return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
inline cl_uint CrtContext::getAlignment( CrtObjectType objType ) const
{
    cl_uint mem_obj_alignment = 0;
    switch( objType )
    {
        case CrtObject::CL_BUFFER:
        case CrtObject::CL_IMAGE:
        case CrtObject::CL_PIPE:
            mem_obj_alignment = CRT_PAGE_ALIGNMENT;
            break;

        default:
            break;
    }

    return mem_obj_alignment;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CrtContext::CreateImage(
    cl_mem_flags                    flags,
    const cl_image_format *         image_format,
    const cl_image_desc *           image_desc,
    void *                          host_ptr,
    CrtMemObject**                  imageObj)
{
    cl_int errCode = CL_SUCCESS;

    cl_uint validMemObjTypes =
        CL_MEM_OBJECT_IMAGE1D |
        CL_MEM_OBJECT_IMAGE1D_BUFFER |
        CL_MEM_OBJECT_IMAGE2D |
        CL_MEM_OBJECT_IMAGE3D |
        CL_MEM_OBJECT_IMAGE1D_ARRAY |
        CL_MEM_OBJECT_IMAGE2D_ARRAY;

    if( image_desc->image_type & ( ~validMemObjTypes ) )
    {
        return CL_INVALID_VALUE;
    }

    if( !( ( image_desc->image_type == CL_MEM_OBJECT_IMAGE1D_BUFFER ) ^ ( image_desc->mem_object == NULL ) ) )
    {
        return CL_INVALID_IMAGE_DESCRIPTOR;
    }

    if( image_desc->mem_object != NULL )
    {
        CrtBuffer* crtBuffer = reinterpret_cast<CrtBuffer*>(((_cl_mem_crt*)image_desc->mem_object)->object);
        host_ptr = crtBuffer->m_pBstPtr;
    }

    if( ( host_ptr == NULL ) &&
        ( flags & CL_MEM_USE_HOST_PTR ) )
    {
        return CL_INVALID_HOST_PTR;
    }

    CrtImage* image = new CrtImage(
        image_format,
        image_desc,
        flags,
        host_ptr,
        this);

    if( !image )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = image->Create( imageObj );
    if( CL_SUCCESS != errCode )
    {
        image->Release();
        image->DecPendencyCnt();
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CrtContext::CreateBuffer(
    cl_mem_flags            flags,
    size_t                  size,
    void *                  host_ptr,
    CrtMemObject**          bufObj)
{
    cl_int errCode = CL_SUCCESS;

    if( size == 0 )
    {
        return CL_INVALID_BUFFER_SIZE;
    }

    if( ( flags & CL_MEM_USE_HOST_PTR || flags & CL_MEM_COPY_HOST_PTR ) && ( NULL == host_ptr ) )
    {
        return CL_INVALID_HOST_PTR;
    }

    if( !( flags & CL_MEM_USE_HOST_PTR || flags & CL_MEM_COPY_HOST_PTR ) && ( NULL != host_ptr ) )
    {
        return CL_INVALID_HOST_PTR;
    }

    CrtBuffer* buffer = new CrtBuffer(size, flags, host_ptr, this);
    if( !buffer )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    errCode = buffer->Create(bufObj);
    if( CL_SUCCESS != errCode )
    {
        buffer->Release();
        buffer->DecPendencyCnt();
    }
    return errCode;
}

cl_int CrtContext::CreateSubBuffer(
    _cl_mem_crt*            parent_buffer,
    cl_mem_flags            flags,
    cl_buffer_create_type   buffer_create_type,
    const void *            buffer_create_info,
    CrtMemObject**          bufObj)
{
    cl_int errCode = CL_SUCCESS;

    if( flags & CL_MEM_USE_HOST_PTR || flags & CL_MEM_COPY_HOST_PTR || flags & CL_MEM_ALLOC_HOST_PTR )
    {
        return CL_INVALID_VALUE;
    }

    CrtMemObject* parentBuf = ((CrtMemObject*)(parent_buffer->object));
    CrtBuffer* buffer = new CrtBuffer(parentBuf, flags, this);           
            
    errCode = buffer->Create(bufObj, buffer_create_type, buffer_create_info);
    if( CL_SUCCESS != errCode )
    {
        buffer->Release();
        buffer->DecPendencyCnt();
    }
    else
    {           
        if( (flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY)) == 0 )
        {
            buffer->m_flags |= buffer->m_parentBuffer->m_flags & (CL_MEM_READ_WRITE | CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY);            
        }
        buffer->m_flags |= buffer->m_parentBuffer->m_flags & (CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR);            
        if( (flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS)) == 0)
        {
            buffer->m_flags |= buffer->m_parentBuffer->m_flags & (CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS);
        }
        *bufObj = buffer;
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int  CrtContext::CreateCommandQueue(
	cl_command_queue			queue_crt_handle,
    cl_device_id                device,
    cl_command_queue_properties properties,
    CrtQueue**                  crtQueue)
{
    cl_int errCode = CL_SUCCESS;
    CrtQueue* pCrtQueue = NULL;
    *crtQueue = NULL;

    DEV_CTX_MAP::iterator itr = m_DeviceToContext.find(device);
    if( itr == m_DeviceToContext.end() )
    {
        return CL_INVALID_DEVICE;
    }

    cl_command_queue queueDEV = itr->second->dispatch->clCreateCommandQueue(itr->second, device, properties, &errCode);
    if( CL_SUCCESS == errCode )
    {
        pCrtQueue = new CrtQueue(this);
        if( NULL == pCrtQueue )
        {
            errCode = itr->second->dispatch->clReleaseCommandQueue(queueDEV);
            return CL_OUT_OF_HOST_MEMORY;
        }
        pCrtQueue->m_cmdQueueDEV = queueDEV;
        pCrtQueue->m_device = device;
		pCrtQueue->m_queue_handle = queue_crt_handle;
        *crtQueue = pCrtQueue;
        {
            OCLCRT::Utils::OclAutoMutex CS(&m_mutex);   // Critical section
            m_HostcommandQueues.push_back(queueDEV);
        }
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int  CrtContext::CreateCommandQueueWithProperties(
    cl_command_queue            queue_crt_handle,
    cl_device_id                device,
    const cl_queue_properties*  properties,
    CrtQueue**                  crtQueue )
{
    cl_int errCode = CL_SUCCESS;
    CrtQueue* pCrtQueue = NULL;
    *crtQueue = NULL;

    DEV_CTX_MAP::iterator itr = m_DeviceToContext.find( device );
    if( itr == m_DeviceToContext.end( ) )
    {
        return CL_INVALID_DEVICE;
    }

    cl_command_queue queueDEV = itr->second->dispatch->clCreateCommandQueueWithProperties( itr->second,
                                                                                           device,
                                                                                           properties,
                                                                                           &errCode );
    if( CL_SUCCESS == errCode )
    {
        pCrtQueue = new CrtQueue( this );
        if( NULL == pCrtQueue )
        {
            errCode = itr->second->dispatch->clReleaseCommandQueue( queueDEV );
            return CL_OUT_OF_HOST_MEMORY;
        }
        pCrtQueue->m_cmdQueueDEV = queueDEV;
        pCrtQueue->m_device = device;
        pCrtQueue->m_queue_handle = queue_crt_handle;
        *crtQueue = pCrtQueue;

        bool QueueOnDevice = false;

        //extract data from cl_queue_properties, to see if queue on device is being created.
        cl_queue_properties PropertyType ;
        cl_queue_properties PropertyValue ;
        if( properties != NULL )
        {
            while( *properties != 0 )
            {
                PropertyType  = properties[0];
                PropertyValue = properties[1];
                properties += 2;
                if( PropertyType == CL_QUEUE_PROPERTIES )
                {
                    if( ( ( ( cl_command_queue_properties ) PropertyValue ) & CL_QUEUE_ON_DEVICE ) != 0 )
                    {
                        QueueOnDevice = true;
                        break;
                    }
                }
            }
        }
        //add only if we have craete non device queue, such queues can't flush on host
        if( QueueOnDevice == false )
        {
            OCLCRT::Utils::OclAutoMutex CS( &m_mutex );   // Critical section
            m_HostcommandQueues.push_back( queueDEV );
        }
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtPlatform::CrtPlatform()
:m_icdSuffix(NULL),
m_supportedExtensionsStr(NULL),
m_platformIdDEV(NULL)
{
}

CrtKernel::CrtKernel(CrtProgram* program)
:m_programCRT(program)
{    
    m_programCRT->IncPendencyCnt();
}

CrtKernel::~CrtKernel()
{
    m_programCRT->DecPendencyCnt();
}

CrtProgram::CrtProgram( CrtContext* ctx )
:m_contextCRT(ctx),
m_options("")
{
    m_contextCRT->IncPendencyCnt();
}

CrtProgram::~CrtProgram()
{
    m_buildContexts.clear();
    m_contextCRT->DecPendencyCnt();
}

CrtQueue::CrtQueue(CrtContext* ctx)
:m_contextCRT(ctx)
{
    m_contextCRT->IncPendencyCnt();
}

CrtQueue::~CrtQueue()
{
    m_contextCRT->DecPendencyCnt();
}

CrtEvent::CrtEvent(CrtQueue* queue, bool isUserEvent):
m_queueCRT(queue),m_eventDEV(NULL), m_isUserEvent(isUserEvent)
{
    if (m_queueCRT)
    {
        m_queueCRT->IncPendencyCnt();
    }
}

CrtEvent::~CrtEvent()
{
    if (m_queueCRT)
    {
        m_queueCRT->DecPendencyCnt();
    }
}

CrtUserEvent::CrtUserEvent(CrtContext* ctx):
m_pContext(ctx),CrtEvent(NULL,true)
{
    m_pContext->IncPendencyCnt();
}

CrtUserEvent::~CrtUserEvent()
{
    m_pContext->DecPendencyCnt();
}

cl_int CrtQueue::Release()
{
    OCLCRT::Utils::OclAutoMutex CS(&m_contextCRT->m_mutex);   // Critical section
    cl_int errCode  = m_cmdQueueDEV->dispatch->clReleaseCommandQueue(m_cmdQueueDEV);    
    m_contextCRT->m_HostcommandQueues.remove(m_cmdQueueDEV);
    return errCode;
}

cl_int CrtSampler::Release()
{
    cl_int errCode = CL_SUCCESS;

    CTX_SMP_MAP::iterator itr = m_ContextToSampler.begin();
    for( ;itr != m_ContextToSampler.end(); itr++ )
    {
        errCode = itr->second->dispatch->clReleaseSampler(itr->second);
        if( errCode != CL_SUCCESS )
            break;
    }

    m_ContextToSampler.clear();
    return errCode;
}

cl_int CrtContext::CreateSampler(
    cl_bool                 normalized_coords,
    cl_addressing_mode      addressing_mode,
    cl_filter_mode          filter_mode,
    CrtSampler**            sampler)
{
    cl_int errCode = CL_SUCCESS;

    CrtSampler* crtSampler = new CrtSampler;
    if( !crtSampler )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    for( SHARED_CTX_DISPATCH::iterator itr = m_contexts.begin();
        itr != m_contexts.end();
        itr++ )
    {
        cl_context ctx = itr->first;
        cl_sampler sampObj = ctx->dispatch->clCreateSampler(
            ctx,
            normalized_coords,
            addressing_mode,
            filter_mode,
            &errCode );

        if( CL_SUCCESS == errCode )
        {
            crtSampler->m_ContextToSampler[ctx] = sampObj;
            crtSampler->m_contextCRT = this;
        }
        else
        {
            break;
        }
    }

FINISH:
    if( ( CL_SUCCESS != errCode ) && crtSampler )
    {
        crtSampler->Release();
        crtSampler->DecPendencyCnt();
    }
    *sampler = crtSampler;
    return errCode;

}

cl_int CrtContext::clCreateSamplerWithProperties(
    const cl_sampler_properties *sampler_properties,
    CrtSampler                  **sampler)
{
    cl_int     errCode     = CL_SUCCESS;
    CrtSampler *crtSampler = new CrtSampler;

    if( !crtSampler )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    for( SHARED_CTX_DISPATCH::iterator itr = m_contexts.begin();
        itr != m_contexts.end();
        itr++ )
    {
        cl_context ctx = itr->first;
        cl_sampler sampObj = ctx->dispatch->clCreateSamplerWithProperties(
            ctx,
            sampler_properties,
            &errCode );

        if( CL_SUCCESS == errCode )
        {
            crtSampler->m_ContextToSampler[ctx] = sampObj;
            crtSampler->m_contextCRT = this;
        }
        else
        {
            break;
        }
    }

FINISH:
    if( ( CL_SUCCESS != errCode ) && crtSampler )
    {
        crtSampler->Release();
        crtSampler->DecPendencyCnt();
    }
    *sampler = crtSampler;
    return errCode;

}

cl_int CrtProgram::Release()
{
    cl_int errCode = CL_SUCCESS;
    CTX_PGM_MAP::iterator itr = m_ContextToProgram.begin();
    for( ;itr != m_ContextToProgram.end(); itr++ )
    {
        cl_int ctxErrCode = itr->first->dispatch->clReleaseProgram(itr->second);
        if( CL_SUCCESS != ctxErrCode )
        {
            errCode = ctxErrCode;
        }
    }    
    return errCode;
}

cl_int CrtKernel::Release()
{
    cl_int errCode = CL_SUCCESS;
    CTX_KRN_MAP::iterator itr = m_ContextToKernel.begin();
    for( ;itr != m_ContextToKernel.end(); itr++ )
    {
        cl_int ctxErrCode = itr->first->dispatch->clReleaseKernel(itr->second);
        if( CL_SUCCESS != ctxErrCode )
        {
            errCode = ctxErrCode;
        }
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CrtContext::CreatePipe(
    cl_mem_flags                flags,
    cl_uint                     pipe_packet_size,
    cl_uint                     pipe_max_packets,
    const cl_pipe_properties *  properties,
    CrtMemObject**              memObj)
{
    cl_int      errCode = CL_SUCCESS;
    CrtPipe *   pipe    = NULL;

    if( 0 == pipe_packet_size || 0 == pipe_max_packets )
    {
        return CL_INVALID_VALUE;
    }

    pipe = new CrtPipe( pipe_packet_size, pipe_max_packets, properties, flags, this );
    if( NULL == pipe )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = pipe->Create( memObj );
    if( CL_SUCCESS != errCode )
    {
        pipe->Release();
        pipe->DecPendencyCnt();
    }
    return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CrtContext::CreateProgramWithSource(
    cl_uint            count ,
    const char **      strings ,
    const size_t *     lengths ,
    CrtProgram **      crtProgram )
{

    cl_int errCode = CL_SUCCESS;
    CrtProgram *pCrtProgram = new CrtProgram(this);
    if( !pCrtProgram )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    DEV_CTX_MAP::iterator dev_itr = m_DeviceToContext.begin();
    while( dev_itr != m_DeviceToContext.end() )
    {
        pCrtProgram->m_assocDevices.push_back( dev_itr->first );
        dev_itr++;
    }

    SHARED_CTX_DISPATCH::iterator itr = m_contexts.begin();
    for( ;itr != m_contexts.end(); itr++ )
    {
        cl_context ctx = itr->first;
        cl_program pgmObj = ctx->dispatch->clCreateProgramWithSource( ctx, count, strings, lengths, &errCode );
        if (CL_SUCCESS == errCode)
        {
            pCrtProgram->m_ContextToProgram[ctx] = pgmObj;
        }
        else
        {
            break;
        }
    }

    if( CL_SUCCESS == errCode )
    {
        pCrtProgram->m_contextCRT = this;
        *crtProgram = pCrtProgram;
    }
    else
    {
        // Release all previously allocated underlying program objects
        pCrtProgram->Release();
        pCrtProgram->DecPendencyCnt();
    }
    return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CrtContext::CreateProgramWithBinary(
    cl_uint                 num_devices,
    const cl_device_id *    device_list,
    const size_t *          lengths,
    const unsigned char **  binaries,
    cl_int *                binary_status,
    CrtProgram **           crtProgram )
{
    cl_int errCode = CL_SUCCESS;
    cl_uint* indices = NULL;
    cl_device_id* outDevices = NULL;
    size_t* fwdLengths = NULL;
    cl_int* fwdBinaryStatus = NULL;
    const unsigned char ** fwdBinaries = NULL;

    CrtProgram *pCrtProgram = new CrtProgram(this);
    if( !pCrtProgram )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    fwdBinaries = new const unsigned char *[num_devices];
    if( NULL == fwdBinaries )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    fwdBinaryStatus = new cl_int[num_devices];
    if( NULL == fwdBinaryStatus )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    fwdLengths = new size_t[num_devices];
    if( NULL == fwdLengths )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    outDevices = new cl_device_id[num_devices];
    if( NULL == outDevices )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    indices = new cl_uint[num_devices];
    if( NULL == indices )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    for( cl_uint i = 0; i <num_devices; i++ )
    {
        pCrtProgram->m_assocDevices.push_back( device_list[i] );
    }

    for( cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++ )
    {
        cl_uint matchDevices = 0;
        GetDevsIndicesByPlatformId(
            num_devices,
            device_list,
            OCLCRT::crt_ocl_module.m_oclPlatforms[i]->m_platformIdDEV,
            &matchDevices,
            indices);

        if( matchDevices == 0 )
        {
            continue;
        }

        for( cl_uint j=0; j < matchDevices; j++ )
        {
            cl_uint index = indices[j];
            outDevices[j]   = device_list[index];
            fwdBinaries[j]  = binaries[index];
            fwdLengths[j]   = lengths[index];
        }

        cl_context ctx = m_DeviceToContext[outDevices[0]];
        cl_program prog = ctx->dispatch->clCreateProgramWithBinary(
            ctx,
            matchDevices,
            outDevices,
            fwdLengths,
            fwdBinaries,
            fwdBinaryStatus,
            &errCode);

        if( CL_SUCCESS != errCode )
        {
            goto FINISH;
        }

        if( binary_status )
        {
            for (cl_uint j=0; j < matchDevices; j++)
            {
                cl_uint index = indices[j];
                binary_status[index] = fwdBinaryStatus[j];
            }
        }
        pCrtProgram->m_ContextToProgram[ctx] = prog;
    }

FINISH:
    if( indices )
    {
        delete[] indices;
    }
    if( outDevices )
    {
        delete[] outDevices;
    }
    if( fwdLengths )
    {
        delete[] fwdLengths;
    }
    if( fwdBinaryStatus )
    {
        delete[] fwdBinaryStatus;
    }
    if( fwdBinaries )
    {
        delete[] fwdBinaries;
    }
    if( ( CL_SUCCESS != errCode ) && pCrtProgram )
    {
        pCrtProgram->Release();
        pCrtProgram->DecPendencyCnt();
    }
    *crtProgram = pCrtProgram;
    return errCode;
}

/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CrtContext::CreateProgramWithBuiltInKernels(
    cl_uint                 num_devices,
    const cl_device_id *    device_list,
    const char *            kernel_names,
    CrtProgram **           crtProgram )
{
    cl_int errCode = CL_SUCCESS;
    cl_uint* indices = NULL;
    cl_device_id* outDevices = NULL;

    CrtProgram *pCrtProgram = new CrtProgram(this);
    if( !pCrtProgram )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    outDevices = new cl_device_id[num_devices];
    if( NULL == outDevices )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    indices = new cl_uint[num_devices];
    if( NULL == indices )
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    for( cl_uint i = 0; i <num_devices; i++ )
    {
        pCrtProgram->m_assocDevices.push_back( device_list[i] );
    }

    for( cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++ )
    {
        cl_uint matchDevices = 0;
        GetDevsIndicesByPlatformId(
            num_devices,
            device_list,
            OCLCRT::crt_ocl_module.m_oclPlatforms[i]->m_platformIdDEV,
            &matchDevices,
            indices);

        if( matchDevices == 0 )
        {
            continue;
        }

        for( cl_uint j=0; j < matchDevices; j++ )
        {
            cl_uint index = indices[j];
            outDevices[j]   = device_list[index];
        }

        cl_context ctx = m_DeviceToContext[outDevices[0]];
        cl_program prog = ctx->dispatch->clCreateProgramWithBuiltInKernels(
            ctx,
            matchDevices,
            outDevices,
            kernel_names,
            &errCode);

        if( CL_SUCCESS != errCode )
        {
            goto FINISH;
        }
        pCrtProgram->m_ContextToProgram[ctx] = prog;
    }

FINISH:
    if( indices )
    {
        delete[] indices;
    }
    if( outDevices )
    {
        delete[] outDevices;
    }
    if( ( CL_SUCCESS != errCode ) && pCrtProgram )
    {
        pCrtProgram->Release();
        pCrtProgram->DecPendencyCnt();
        pCrtProgram = NULL;
    }
    *crtProgram = pCrtProgram;
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------

SyncManager::SyncManager()
{
    m_callBackData = NULL;
    m_outEventArray = NULL;
}

cl_int  SyncManager::EnqueueNopCommand(
    CrtMemObject*   memObj,
    CrtQueue*       queue,
    cl_uint         NumEventsInWaitList,
    const cl_event* inEventWaitList,
    cl_event*       outEvent)
{
    cl_int errCode = CL_SUCCESS;
    static size_t origin[3] = { 0, 0, 0 };
    static size_t region[3] = { 1, 1, 1 };

    if( memObj->getObjectType() == CrtObject::CL_BUFFER )
    {
        errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueReadBuffer(
            queue->m_cmdQueueDEV,
            memObj->getDeviceMemObj(queue->m_device),
            CL_FALSE,
            origin[0],
            region[0],
            memObj->m_pBstPtr,
            NumEventsInWaitList,
            inEventWaitList,
            outEvent);
    }
    else
    {
        errCode = queue->m_cmdQueueDEV->dispatch->clEnqueueReadImage(
            queue->m_cmdQueueDEV,
            memObj->getDeviceMemObj(queue->m_device),
            CL_FALSE,
            origin,
            region,
            0,
            0,
            memObj->m_pBstPtr,
            NumEventsInWaitList,
            inEventWaitList,
            outEvent);
    }
    return errCode;
}

cl_int SyncManager::PrepareToExecute(
    CrtQueue*       queue,
    cl_uint         NumEventsInWaitList,
    const cl_event* inEventWaitList,
    cl_uint*        numOutEvents,
    cl_event**      OutEvents)
{
    CrtEvent *nullEvent     = NULL;
    cl_int errCode          = CL_SUCCESS;
    cl_uint numReqCalls     = 0;
    cl_context dstContext   = NULL;
    cl_uint j                = 0;
    
    if( ( ( inEventWaitList == NULL ) && ( NumEventsInWaitList > 0 ) ) || 
        ( ( inEventWaitList != NULL ) && ( NumEventsInWaitList == 0 ) ) )
    {
        return CL_INVALID_EVENT_WAIT_LIST;
    }

    for( cl_uint i=0; i < NumEventsInWaitList; i++ )
    {
        CrtEvent* pEvent = (CrtEvent*)((reinterpret_cast<const _cl_event_crt*>(inEventWaitList[i]))->object);
        if( pEvent->getContext() != queue->m_contextCRT )
        {
            errCode = CL_INVALID_CONTEXT;
            goto FINISH;
        }
    }

    dstContext = queue->m_contextCRT->m_DeviceToContext[queue->m_device];
    if( NumEventsInWaitList != 0 )
    {
        m_outEventArray = new cl_event[NumEventsInWaitList];
        if( NULL == m_outEventArray)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }
    }    
    for( cl_uint i=0; i < NumEventsInWaitList; i++ )
    {
        const CrtEvent* waitEvent  = NULL;
        cl_context evContext = NULL;
        // there are events belong to different undelying platforms
        if( nullEvent )
        {
            waitEvent = nullEvent;
        }
        else
        {
            waitEvent = (CrtEvent*)( (reinterpret_cast<const _cl_event_crt*>(inEventWaitList[i]))->object );
        }
        if( waitEvent->m_isUserEvent )
        {
            evContext = dstContext;
        }
        else
        {
            evContext = waitEvent->m_queueCRT->m_contextCRT->m_DeviceToContext[waitEvent->m_queueCRT->m_device];
        }
        if( !waitEvent->m_isUserEvent && ( dstContext != evContext ) )
        {
            numReqCalls++;
        }
    }

    m_eventRetained = false;
    for( cl_uint i=0; i < NumEventsInWaitList; i++ )
    {
        const CrtEvent* waitEvent  = NULL;
        // there are events belong to different undelying platforms
        if( nullEvent )
        {
            waitEvent = nullEvent;
        }
        else
        {
            waitEvent = (CrtEvent*)(
                (reinterpret_cast<const _cl_event_crt*>(inEventWaitList[i]))->object
                );
        }

        cl_context evContext = NULL;
        cl_event eventDEV = NULL;

        if( waitEvent->m_isUserEvent )
        {
            eventDEV = ((CrtUserEvent*)waitEvent)->m_ContextToEvent[dstContext];
            evContext = dstContext;
        }
        else
        {
            eventDEV = waitEvent->m_eventDEV;
            evContext = waitEvent->m_queueCRT->m_contextCRT->m_DeviceToContext[waitEvent->m_queueCRT->m_device];
        }

        // the target event (aiming for queue) is dependant on an event beloging to different
        // underlying context.
        if( !waitEvent->m_isUserEvent && ( dstContext != evContext ) )
        {
            // Initialization of callback and data is done only once
            if( !m_callBackData )
            {
                // Initialize crt callback data
                m_callBackData = new CrtEventCallBackData;
                if( !m_callBackData )
                {
                    errCode = CL_OUT_OF_HOST_MEMORY;
                    goto FINISH;
                }
                m_callBackData->numReqCalls = numReqCalls;
                m_callBackData->m_eventDEV = queue->m_cmdQueueDEV->dispatch->clCreateUserEvent(dstContext, &errCode);
                if( CL_SUCCESS != errCode )
                {
                    errCode = CL_OUT_OF_RESOURCES;
                    goto FINISH;
                }

                m_userEvent = m_callBackData->m_eventDEV;

                // The corresponding release happens in
                //     1) CrtEventCallBack - guarantees we manage to call setUserEventStatus
                //                           before it fires
                //     2) ~SyncManager - guarantees we manage to register
                //                      on it before it fires
                errCode = m_callBackData->m_eventDEV->dispatch->clRetainEvent(
                                m_callBackData->m_eventDEV);

                if( CL_SUCCESS != errCode )
                {
                    errCode = CL_OUT_OF_RESOURCES;
                    goto FINISH;
                }
                m_eventRetained = true;
                if( CL_SUCCESS != errCode )
                {
                    errCode = CL_OUT_OF_RESOURCES;
                    goto FINISH;
                }
                // annex the user event to the destination queue events
                m_outEventArray[j++] = m_callBackData->m_eventDEV;
            }

            errCode = eventDEV->dispatch->clSetEventCallback(
                eventDEV, CL_COMPLETE, CrtEventCallBack, m_callBackData );

            if( CL_SUCCESS != errCode )
            {
                goto FINISH;
            }
        }
        else
        {
            m_outEventArray[j++] = eventDEV;
        }
    }
    *numOutEvents = j;
    if( 0 == *numOutEvents )
    {
        *OutEvents = NULL;
    }
    else
    {
        *OutEvents = m_outEventArray;
    }
FINISH:

    if( nullEvent )
    {
        nullEvent->Release();
        nullEvent->DecPendencyCnt();
    }
    if( CL_SUCCESS != errCode )
    {
        if( m_eventRetained )
        {
            m_callBackData->m_eventDEV->dispatch->clReleaseEvent(
                                m_callBackData->m_eventDEV);
            m_eventRetained = false;
        }
        if( m_callBackData )
        {
            delete m_callBackData;
        }
        m_userEvent = NULL;
        return errCode;
    }
    return errCode;
}

void SyncManager::Release(cl_int errCode)
{
    if( m_eventRetained && m_userEvent )
    {
        m_userEvent->dispatch->clReleaseEvent(m_userEvent);
        m_eventRetained = false;
    }
    if( m_outEventArray )
    {
        delete[] m_outEventArray;
        m_outEventArray = NULL;
    }
}

SyncManager::~SyncManager()
{
    Release( CL_SUCCESS );
}


cl_int CrtEvent::Release()
{
    cl_int errCode = CL_SUCCESS;
    if( m_eventDEV )
    {
        // Its enough to call Release once since we didn't
        // forward Retains from CRT to the underlying platform
        errCode = m_eventDEV->dispatch->clReleaseEvent(m_eventDEV);
    }
    return errCode;
}

cl_int CrtUserEvent::Release()
{
    cl_int errCode = CL_SUCCESS;
        std::map<cl_context,cl_event>::iterator itr = m_ContextToEvent.begin();
        for( ;itr != m_ContextToEvent.end(); itr++ )
        {
        // Its enought to call Release once since we didn't
        // forward Retains from CRT to the underlying platform
        cl_int ctxErrCode = itr->second->dispatch->clReleaseEvent(itr->second);
        if( ctxErrCode != CL_SUCCESS )
        {
            errCode = ctxErrCode;
        }
    }
    return errCode;
}

cl_int GetCrtExtension(const char* str_extensions)
{
    cl_int ext_keys = 0;
    std::istringstream iss(str_extensions);
    do
    {
        std::string sub;
        iss >> sub;

        if( !sub.compare( "cl_khr_gl_sharing" ) )
        {
            ext_keys |= CRT_CL_GL_EXT;
        }
        else if( !sub.compare( "cl_intel_dx9_media_sharing" ) )
        {
            ext_keys |= CRT_CL_INTEL_D3D9_EXT;
        }
        else if( !sub.compare( "cl_khr_dx9_media_sharing" ) )
        {
            ext_keys |= CRT_CL_D3D9_EXT;
        }
        else if( !sub.compare( "cl_khr_d3d10_sharing" ) )
        {
            ext_keys |= CRT_CL_D3D10_EXT;
        }
        else if( !sub.compare( "cl_khr_d3d11_sharing" ) )
        {
            ext_keys |= CRT_CL_D3D11_EXT;
        }
    } while( iss );
    return ext_keys;
}

cl_uint GetPlatformVersion( const char* platform_version )
{
    cl_uint version = 0;

    std::istringstream iss( platform_version );
    std::string sub;
    iss >> sub;
    iss >> sub;

    if( !sub.compare( "1.1" ) )
    {
        version = OPENCL_1_1;
    }
    else if( !sub.compare( "1.2" ) )
    {
        version = OPENCL_1_2;
    }
    else if( !sub.compare( "2.0" ) )
    {
        version = OPENCL_2_0;
    }
    return version;
}

#ifdef _WIN32
cl_int CrtContext::CreateFromDX9MediaSurface(        
        cl_mem_flags            flags,        
        IDirect3DSurface9 *     resource,
        HANDLE                  sharedHandle,
        UINT                    plane,
        CrtMemObject**          memObj)
{
    cl_int errCode = CL_SUCCESS;

    CrtDX9MediaSurface* dx9MediaSurfaceObj = new CrtDX9MediaSurface(
                                                flags, 
                                                resource,
                                                sharedHandle,
                                                plane,
                                                this);

    if( !dx9MediaSurfaceObj )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    errCode = dx9MediaSurfaceObj->Create( memObj );
    if( CL_SUCCESS != errCode )
    {
        dx9MediaSurfaceObj->Release();
        dx9MediaSurfaceObj->DecPendencyCnt();
    }
    return errCode;
}
#endif

cl_int CrtContext::CreateGLBuffer(
    bool                    isRender,
    cl_mem_flags            flags,        
    GLuint                  bufobj,
    CrtMemObject**          memObj)
{
    cl_int errCode = CL_SUCCESS;

    CrtGLBuffer* buffer = new CrtGLBuffer(isRender, flags, bufobj, this);
    if( !buffer )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    errCode = buffer->Create( memObj );
    if( CL_SUCCESS != errCode )
    {
        buffer->Release();
        buffer->DecPendencyCnt();
    }
    return errCode;
}

cl_int CrtContext::CreateGLImage(
    cl_uint                 dim_count,
    cl_mem_flags            flags,
    cl_GLenum               target,
    cl_GLint                miplevel,
    cl_GLuint               texture,
    CrtMemObject**          memObj)
{
    cl_int errCode = CL_SUCCESS;
   
    CrtGLImage* image = new CrtGLImage(
        dim_count,
        flags,
        target,
        miplevel,
        texture,
        this);

    if( !image )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    errCode = image->Create( memObj );
    if( CL_SUCCESS != errCode )
    {
        image->Release();
        image->DecPendencyCnt();
    }
    return errCode;
}

SVMFreeCallbackData::~SVMFreeCallbackData()
{
    if( m_SVMPointers )
    {
        delete[] m_SVMPointers;
    }
}

bool SVMFreeCallbackData::CopySVMPointers(void** SVMPointers, cl_uint numSVMPointers)
{
    if( NULL != m_SVMPointers )
    {
        return false;
    }

    m_SVMPointers = new void*[ numSVMPointers ];
    if( NULL == m_SVMPointers )
    {
        return false;
    }
    m_numSVMPointers = numSVMPointers;

    memcpy_s( m_SVMPointers,
        m_numSVMPointers * sizeof(void*),
        SVMPointers,
        numSVMPointers * sizeof(void*) );

    return true;
}

void CL_CALLBACK SVMFreeCallbackFunction(cl_event event, cl_int status, void *myData)
{
    bool        haveSystemPointers  = false;

    assert( CL_COMPLETE == status && "Callback called before event complete!!");
    assert( myData != NULL && "Invalid data was passed to the callback");

    SVMFreeCallbackData* clbkData = static_cast<SVMFreeCallbackData*>( myData );

    CrtQueue* crtQueue = reinterpret_cast<CrtQueue*>( ( ( _cl_command_queue_crt* )clbkData->m_queue )->object );
    assert( crtQueue != NULL && "Invalid queue was passed to the callback");

    if( clbkData->m_isGpuQueue )
    {
        // Callback called from GPU; only delete SVM pointers from cache
        for( cl_uint i = 0; i < clbkData->m_numSVMPointers; i++ )
        {
            crtQueue->m_contextCRT->m_svmPointers.remove( clbkData->m_SVMPointers[i] );
        }
    }
    else
    {
        // Callback called from CPU

        std::list<void *> * svmPointers = &( crtQueue->m_contextCRT->m_svmPointers );

        // Free pointers that were returned by clSVMAlloc
        for( cl_uint i = 0; i < clbkData->m_numSVMPointers; i++ )
        {
            if( svmPointers->end() != std::find( svmPointers->begin(), svmPointers->end(), clbkData->m_SVMPointers[i] ) )
            {
                clSVMFree( crtQueue->m_contextCRT->m_context_handle, clbkData->m_SVMPointers[i] );
            }
            else
            {
                // We have system pointers to free using user's original callback
                haveSystemPointers = true;
            }
        }

        if( NULL != clbkData->m_originalCallback )
        {
            // User provided free function to the original clEnqueueSVMFree call
            clbkData->m_originalCallback(
                clbkData->m_queue,
                clbkData->m_numSVMPointers,
                clbkData->m_SVMPointers,
                clbkData->m_originalUserData );
        }
        else
        {
            // User didn't provide free function; Assume no system pointers here
            assert( !haveSystemPointers && "System pointers were passed to callback without free function! This should have been checked before." );
        }

        // Mark the event which is returned to user as complete
        clSetUserEventStatus( clbkData->m_svmFreeUserEvent->m_eventDEV, CL_COMPLETE );
    }

    // Release the marker that was created in clEnqueueSVMFree
    clReleaseEvent( event );

    // The event was not returned to user so we must release it here
    if( clbkData->m_shouldReleaseEvent )
    {
        clbkData->m_svmFreeUserEvent->Release();
        clbkData->m_svmFreeUserEvent->DecPendencyCnt();
    }

    delete clbkData;
}
