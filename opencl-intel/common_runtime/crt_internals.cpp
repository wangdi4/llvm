// Copyright (c) 2006-2007 Intel Corporation
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
#include "crt_internals.h"
#include "crt_module.h"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

namespace OCLCRT
{
    extern CrtModule crt_ocl_module;
};

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

// USed by CopyMemoryBuffer, for copying image memory objects
struct MemCpyParams
{
    cl_uint         uiDimCount;
    cl_char*        pSrc;
    size_t          vSrcPitch[2];
    cl_char*        pDst;
    size_t          vDstPitch[2];
    size_t          vRegion[3];
};

void CopyMemoryBuffer(MemCpyParams* pCopyCmd)
{
    // Copy 1D array only
    if ( 1 == pCopyCmd->uiDimCount )
    {
        memcpy(pCopyCmd->pDst, pCopyCmd->pSrc, pCopyCmd->vRegion[0]);
        return;
    }

    MemCpyParams sRecParam;

    // Copy current parameters
    memcpy(&sRecParam, pCopyCmd, sizeof(MemCpyParams));
    sRecParam.uiDimCount = pCopyCmd->uiDimCount-1;
    // Make recursion
    for(unsigned int i=0; i<pCopyCmd->vRegion[sRecParam.uiDimCount]; ++i)
    {
        CopyMemoryBuffer(&sRecParam);
        sRecParam.pSrc = sRecParam.pSrc + pCopyCmd->vSrcPitch[sRecParam.uiDimCount-1];
        sRecParam.pDst = sRecParam.pDst + pCopyCmd->vDstPitch[sRecParam.uiDimCount-1];
    }
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
// User event Callback, which releases userevent when called by all
// registered callbacks
void __stdcall CrtEventCallBack(cl_event e, cl_int status, void* user_data)
{
    CrtEventCallBackData* evData = (CrtEventCallBackData*)user_data;

    // Check if this is the last event to call this callback
    if (0 == atomic_decrement(&(evData->numReqCalls)))
    {
            /// Release the crt user event (bridge)
        cl_int error = evData->m_eventDEV->dispatch->clSetUserEventStatus(
            evData->m_eventDEV, CL_COMPLETE);

        evData->m_eventDEV->dispatch->clReleaseEvent(evData->m_eventDEV);
        delete evData;
    }
}

void CL_CALLBACK CrtMapUnmapCallBack(cl_event e, cl_int status, void* user_data)
{
    CrtMapUnmapCallBackData *mapData = (CrtMapUnmapCallBackData*)user_data;

    // Currently we copy all the memory object data;
    // more ideally we want to copy only the [region] which
    // has been mapped and not all data.
    if (mapData->m_syncWay == SyncManager::SYNC_FROM_BACKING_STORE)
    {
        if (mapData->m_memObj->getObjectType() == CrtObject::CL_IMAGE)
        {
            CrtImage * image = (CrtImage*)(mapData->m_memObj);
            MemCpyParams            sCpyParam;
            sCpyParam.pSrc = (cl_char*)image->m_pBstPtr;
            sCpyParam.pDst = (cl_char*)image->m_pUsrPtr;
            sCpyParam.uiDimCount = image->m_dimCount;
            sCpyParam.vSrcPitch[0] = image->m_imageRowPitch;
            sCpyParam.vSrcPitch[1] = image->m_imageSlicePitch;
            sCpyParam.vDstPitch[0] = image->m_hostPtrRowPitch;
            sCpyParam.vDstPitch[1] = image->m_hostPtrSlicePitch;
            sCpyParam.vRegion[0] = image->m_imageWidth * GetImageElementSize(image->m_imageFormat);
            sCpyParam.vRegion[1] = image->m_imageHeight;
            sCpyParam.vRegion[2] = image->m_imageDepth;
            CopyMemoryBuffer(&sCpyParam);
        }
        else
        {
            // On Map (->), we copy from private copy to user copy
            memcpy_s(
                mapData->m_memObj->m_pUsrPtr,
                mapData->m_memObj->m_size,
                mapData->m_memObj->m_pBstPtr,
                mapData->m_memObj->m_size);
        }
    }
    else if (mapData->m_syncWay == SyncManager::SYNC_TO_BACKING_STORE)
    {
        // On Unmap (->), we copy from the user copy to the
        //                 crt private copy
        if (mapData->m_memObj->getObjectType() == CrtObject::CL_IMAGE)
        {
            CrtImage * image = (CrtImage*)(mapData->m_memObj);
            MemCpyParams            sCpyParam;
            sCpyParam.pSrc = (cl_char*)image->m_pUsrPtr;
            sCpyParam.pDst = (cl_char*)image->m_pBstPtr;
            sCpyParam.uiDimCount = image->m_dimCount;
            sCpyParam.vSrcPitch[0] = image->m_hostPtrRowPitch;
            sCpyParam.vSrcPitch[1] = image->m_hostPtrSlicePitch;
            sCpyParam.vDstPitch[0] = image->m_imageRowPitch;
            sCpyParam.vDstPitch[1] = image->m_imageSlicePitch;
            sCpyParam.vRegion[0] = image->m_imageWidth * GetImageElementSize(image->m_imageFormat);
            sCpyParam.vRegion[1] = image->m_imageHeight;
            sCpyParam.vRegion[2] = image->m_imageDepth;
            CopyMemoryBuffer(&sCpyParam);
        }
        else
        {
            memcpy_s(
                mapData->m_memObj->m_pBstPtr,
                mapData->m_memObj->m_size,
                mapData->m_memObj->m_pUsrPtr,
                mapData->m_memObj->m_size);
        }
    }
    else
    {
        assert(0 && "unsupported mode in CrtMapUnmapCallBack");
    }
    // We don't need to release (e), it has been released immediately
    // after registering this callback

    // Release the dependent event on this command
    mapData->m_userEvent->m_eventDEV->dispatch->clSetUserEventStatus(
            mapData->m_userEvent->m_eventDEV, CL_COMPLETE);

    mapData->m_userEvent->Release();
    mapData->m_userEvent->DecPendencyCnt();
    delete mapData;
}

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
    cl_int errCode              = CL_SUCCESS;
    CrtBuildCallBackData *data  = ( CrtBuildCallBackData* ) userData;
    cl_program pgm              = data->m_clProgramHandle;

    // Check if this is the last build routine to finish
    if (0 == atomic_decrement(&(data->m_numBuild)))
    {
        if( data->m_pfnNotify )
        {
            data->m_pfnNotify( pgm, data->m_userData );
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
    logging_fn                      pfn_notify,
    void *                          user_data,
    cl_int *                        errcode_ret):
m_context_handle(context_handle),
m_kernelReflectionDevice(NULL)
{
    cl_int errCode = CL_SUCCESS;

    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

    for (cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++)
    {
        cl_device_id* outDevices = new cl_device_id[OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.size()];
        if (NULL == devices)
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
            continue;

        CrtDeviceInfo* devInfo = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(outDevices[0]);

        cl_context_properties* props;
        if (CRT_FAIL == OCLCRT::ReplacePlatformId(properties, devInfo->m_platformIdDEV, &props))
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

        if (CL_SUCCESS != errCode)
        {
            break;
        }

        for (cl_uint j=0; j < matchDevices; j++)
        {
            m_DeviceToContext[outDevices[j]] = ctx;
        }

        m_contexts[ctx] = &(OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(outDevices[0])->m_origDispatchTable);
    }
    OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();

    if (CL_SUCCESS == errCode)
    {
        errCode = GetDevicesPreferredAlignment(num_devices, devices, &m_alignment);
    }

    if (errcode_ret)
    {
        *errcode_ret = errCode;
    }
}


cl_device_id CrtContext::GetKernelReflectionDevice()
{
    if (NULL == m_kernelReflectionDevice)
    {
        OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Lock();

        for (OCLCRT::DEV_INFO_MAP::const_iterator itr = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().begin();
            itr != OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.get().end();
            itr++)
        {
            if (((CrtKHRicdVendorDispatch*)itr->first->dispatch)->clGetKernelArgInfo != NULL)
            {
                m_kernelReflectionDevice = itr->first;
                break;
            }
        }
        if (m_kernelReflectionDevice == NULL)
        {
            assert(0 && "Cannot operate without clGetKernelArgInfo query");
        }
        OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.Release();
    }

    return m_kernelReflectionDevice;
}


void CrtContext::GetDevicesByPlatformId(
    const cl_uint           inNumDevices,
    const cl_device_id*     inDevices,
    const cl_platform_id&   pId,
    cl_uint*                outNumDevices,
    cl_device_id*           outDevices)
{
    cl_int devCount = 0;
    for (cl_uint i=0; i < inNumDevices; i++)
    {
        if (OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(inDevices[i])->m_platformIdDEV == pId)
        {
            if (!outDevices)
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
    for (cl_uint i=0; i < inNumDevices; i++)
    {
        if (OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(inDevices[i])->m_platformIdDEV == pId)
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


cl_int CrtContext::GetDevicesPreferredAlignment(
    const cl_uint           numDevices,
    const cl_device_id*     devices,
    cl_uint*                alignment)
{
    for (cl_uint i=0; i < numDevices; i++)
    {
        cl_int errCode = CL_SUCCESS;
        cl_uint devAlignment = 0;
        errCode = OCLCRT::crt_ocl_module.m_deviceInfoMapGuard.GetValue(devices[i])->m_origDispatchTable.clGetDeviceInfo(
                        devices[i],
                        CL_DEVICE_MEM_BASE_ADDR_ALIGN,
                        sizeof(cl_uint),
                        (void*)(&devAlignment),
                        NULL);

        if (errCode != CL_SUCCESS)
        {
            return CL_OUT_OF_RESOURCES;
        }

        if (i==0)
        {
            *alignment  = devAlignment;
        }
        else
        {
            *alignment  = CalculateLCM(*alignment , devAlignment);
        }
    }

    return CL_SUCCESS;
}



cl_int CrtContext::Release()
{
    cl_int errCode = CL_SUCCESS;

    SHARED_CTX_DISPATCH::iterator itr = m_contexts.begin();
    for(;itr != m_contexts.end(); itr++)
    {
        errCode = itr->second->clReleaseContext(itr->first);
        if (CL_SUCCESS != errCode)
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
    errCode = itr->second->clGetContextInfo(
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
    m_commandQueues.clear();
}

cl_int CrtContext::FlushQueues()
{
    cl_uint errCode = CL_SUCCESS;
    for(std::list<cl_command_queue>::iterator itr = m_commandQueues.begin();
        itr != m_commandQueues.end();
        itr++)
    {
        const cl_command_queue q = *itr;
        errCode = q->dispatch->clFlush(q);
        if (CL_SUCCESS != errCode)
            return errCode;
    }
    return errCode;
}


/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtMemObject::CrtMemObject(cl_mem_flags flags, void* hostPtr, CrtContext* ctx)
{
    m_flags = flags;
    m_pUsrPtr = NULL;

    if (flags & (CL_MEM_USE_HOST_PTR|CL_MEM_COPY_HOST_PTR) && hostPtr)
        m_pUsrPtr = hostPtr;

    m_pBstPtr = NULL;
    m_pContext = ctx;

    m_pContext->IncPendencyCnt();
}


CrtMemObject::~CrtMemObject()
{
    m_ContextToMemObj.clear();

    /// there are 3 cases we need to deallocate m_pBstPtr:
    ///     1. no user pointer provided so we allocated backing store
    ///     2. user point provided but not aligned, so we kept m_pUsrPtr
    if (m_pBstPtr && (!(m_flags & CL_MEM_USE_HOST_PTR) || m_pUsrPtr))
    {
        _aligned_free(m_pBstPtr);
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
    for ( ; itr != m_ContextToMemObj.end(); itr++)
    {
        if ( itr->second != NULL)
        {
            return itr->second;
        }
    }
    return NULL;
}

cl_int CrtMemObject::Release()
{
    CTX_MEM_MAP::iterator itr_first = m_ContextToMemObj.begin();
    if (itr_first == m_ContextToMemObj.end())
    {
            /// Already released previously
        return CL_SUCCESS;
    }
    itr_first->second->dispatch->clRetainMemObject(itr_first->second);

    cl_mem memObj = itr_first->second;

    for (CTX_MEM_MAP::iterator itr = m_ContextToMemObj.begin();
        itr != m_ContextToMemObj.end();
        itr++)
    {
        itr->second->dispatch->clReleaseMemObject(itr->second);
    }

    m_ContextToMemObj.clear();
    memObj->dispatch->clReleaseMemObject(memObj);

    return CL_SUCCESS;
}


cl_int CrtMemObject::RegisterDestructorCallback(mem_dtor_fn memDtorFunc, void* user_data)
{
    cl_int errCode = CL_SUCCESS;

        /// this memory is dealocated in the callback 'CrtMemDtorForwarder'
    CrtMemDtorForwarderData* memData = new CrtMemDtorForwarderData;
    if (memData == NULL)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        return errCode;
    }
    memData->m_count = (cl_uint)m_ContextToMemObj.size();
    memData->m_user_dtor_func = memDtorFunc;
    memData->m_user_data = user_data;

    for (CTX_MEM_MAP::iterator itr = m_ContextToMemObj.begin();
        itr != m_ContextToMemObj.end();
        itr++)
    {
        errCode = itr->second->dispatch->clSetMemObjectDestructorCallback(
            itr->second,
            CrtMemDtorForwarder,
            (void*)memData);

        if (CL_SUCCESS != errCode)
        {
            return errCode;
        }
    }
    return errCode;
}

bool CrtMemObject::HasPrivateCopy()
{
    return (m_pBstPtr && m_pUsrPtr);
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
    _cl_mem_crt*    parent_buffer,
    cl_mem_flags    flags,
    CrtContext*     ctx): CrtMemObject(flags, NULL, ctx)
{
    m_parentBuffer = parent_buffer;
    if (m_parentBuffer)
    {
        ((CrtBuffer*)(m_parentBuffer->object))->IncPendencyCnt();
}
}


CrtBuffer::~CrtBuffer()
{
    if (m_parentBuffer)
    {
        ((CrtBuffer*)(m_parentBuffer->object))->DecPendencyCnt();
    }
}

cl_int CrtBuffer::Create(CrtMemObject** bufObj)
{
            /// HOST pointer provided and memory is aligned
    if ((m_flags & CL_MEM_USE_HOST_PTR) &&
        m_pUsrPtr && (( (size_t)m_pUsrPtr & (m_pContext->getAlignment()-1)) == 0))
    {
        m_pBstPtr = m_pUsrPtr;
        m_pUsrPtr = NULL;
    }
    else if (m_pBstPtr == NULL && !m_pContext->memObjectAlwaysNotInSync(CrtObject::CL_BUFFER))
    {
            /// Convert bits to bytes
        m_pBstPtr = _aligned_malloc(m_size, m_pContext->getAlignment());

        if (!m_pBstPtr)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        if (m_flags & CL_MEM_COPY_HOST_PTR || m_flags & CL_MEM_USE_HOST_PTR)
        {
            memcpy(m_pBstPtr, m_pUsrPtr, m_size);
        }
    }
    else
    {
        assert(0 && "No support for platforms were buffers isn't linear");
    }

    cl_int errCode = CL_SUCCESS;

    *bufObj = this;

        /// This is being deallocated in callback 'CrtMemDtorCallBack'
    CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;
    if (NULL == memData)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    memData->m_count = (cl_uint)m_pContext->m_contexts.size();
    memData->m_clMemHandle = this;

    cl_uint numActualBufs = 0;

    for(SHARED_CTX_DISPATCH::iterator itr = m_pContext->m_contexts.begin();
        itr != m_pContext->m_contexts.end();
        itr++)
    {
        cl_context ctx = itr->first;
        cl_mem_flags crtCreateFlags = ( ( m_flags | CL_MEM_USE_HOST_PTR ) & ~CL_MEM_ALLOC_HOST_PTR ) &
                                       ~CL_MEM_COPY_HOST_PTR;

        cl_mem memObj = ctx->dispatch->clCreateBuffer(
            ctx,
            crtCreateFlags,
            m_size,
            m_pBstPtr,
            &errCode);

        if (CL_SUCCESS == errCode)
        {
            m_ContextToMemObj[ctx] = memObj;
            errCode = memObj->dispatch->clSetMemObjectDestructorCallback(
                memObj,
                CrtMemDestructorCallBack,
                (void*)memData);

            if (CL_SUCCESS != errCode)
            {
                goto FINISH;
        }
            numActualBufs++;
        }
        else
        {
            goto FINISH;
        }
    }

FINISH:

    if (CL_SUCCESS != errCode)
    {
        if (numActualBufs > 0)
        {
            memData->m_count = numActualBufs;
        }
        else
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

    CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;
    if (NULL == memData)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }
    memData->m_count = (cl_uint)m_pContext->m_contexts.size();
    memData->m_clMemHandle = this;

    cl_uint numActualBufs = 0;
    CrtBuffer* parentBuf = (CrtBuffer*)(m_parentBuffer->object);

    for(SHARED_CTX_DISPATCH::iterator itr = m_pContext->m_contexts.begin();
        itr != m_pContext->m_contexts.end();
        itr++)
    {
        cl_context ctx = itr->first;
        cl_mem parent_Buffer = parentBuf->m_ContextToMemObj[ctx];
        cl_mem memObj = ctx->dispatch->clCreateSubBuffer(
            parent_Buffer,
            m_flags,
            buffer_create_type,
            buffer_create_info,
            &errCode);

        if (CL_SUCCESS == errCode)
        {
            m_ContextToMemObj[ctx] = memObj;
            errCode = memObj->dispatch->clSetMemObjectDestructorCallback(
                memObj,
                CrtMemDestructorCallBack,
                (void*)memData);

            if (CL_SUCCESS != errCode)
            {
                goto FINISH;
            }
            numActualBufs++;
        }
        else
        {
            goto FINISH;
        }
    }

FINISH:

    if (CL_SUCCESS != errCode)
    {
        if (numActualBufs > 0)
        {
            memData->m_count = numActualBufs;
        }
        else
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
CrtImage::CrtImage(
    cl_mem_object_type      image_type,
    const cl_image_format * image_format,
    size_t                  image_width,
    size_t                  image_height,
    size_t                  image_depth,
    cl_mem_flags            flags,
    void*                   host_ptr,
    CrtContext*             ctx): CrtMemObject(flags, host_ptr, ctx),
    m_imageType(image_type),
    m_imageFormat(image_format),
    m_imageWidth(image_width),
    m_imageHeight(image_height),
    m_imageDepth(image_depth)
{
    switch(m_imageType)
    {
        case CL_MEM_OBJECT_IMAGE2D:
            m_dimCount = 2;
            break;
        case CL_MEM_OBJECT_IMAGE3D:
            m_dimCount = 3;
            break;
        default:
            assert(0 && "Not supported image type by CRT");
            break;
    }
        /// Holds for both image2D and image3D creation
    m_imageRowPitch = ( cl_uint )m_imageWidth * GetImageElementSize(m_imageFormat);

        /// Align the row pitch to match the user requirements
    m_imageRowPitch = ( ( m_imageRowPitch + CRT_IMAGE_PITCH_ALIGN -1 ) & ~( CRT_IMAGE_PITCH_ALIGN - 1 ) );

    m_imageSlicePitch = m_imageRowPitch * ( cl_uint )m_imageHeight;

    m_size = m_imageSlicePitch * m_imageDepth;
}


CrtImage::~CrtImage()
{
    // Do nothing
}


cl_int CrtImage::Create(size_t rowPitch, size_t slicePitch, CrtMemObject**  imageObj)
{
    // We need to save these params since, we need use them
    // on Map in case CL_MEM_USE_HOST_PTR has been provided.
    m_hostPtrRowPitch = rowPitch;
    m_hostPtrSlicePitch = slicePitch;



    // Size of memory provided by the user
    size_t hostPtrSize = m_hostPtrSlicePitch * m_imageWidth;

    //  for using the provided host ptr, we need to make sure:
    //     1. size of image provided in host_ptr is big enough to fit
    //        the image.
    //     2. host_ptr pointer is aligned to PAGE_ALIGNMENT
    //     3. host_ptr row pitch has enough space to accommodate
    //        width * elementSize and make that aligned to 64-byte (GEN)
    if ((m_flags & CL_MEM_USE_HOST_PTR) &&
        ( m_pUsrPtr ) &&
        ( ( ( size_t ) m_pUsrPtr & ( CRT_PAGE_ALIGNMENT-1 ) ) == 0 ) &&
        ( hostPtrSize >= m_size ) &&
        ( m_hostPtrRowPitch >= m_imageRowPitch ) )
    {
        m_pBstPtr = m_pUsrPtr;
        m_pUsrPtr = NULL;
    }
    else if( ( m_pBstPtr == NULL ) &&
             ( m_pContext->memObjectAlwaysNotInSync( CrtObject::CL_IMAGE ) == false ) )
    {
        // Convert bits to bytes
        m_pBstPtr = _aligned_malloc( m_size, CRT_PAGE_ALIGNMENT );

        if (!m_pBstPtr)
        {
            return CL_OUT_OF_HOST_MEMORY;
        }

        if( ( m_flags & CL_MEM_COPY_HOST_PTR ) ||
            ( m_flags & CL_MEM_USE_HOST_PTR ) )
        {
            MemCpyParams            sCpyParam;

            sCpyParam.pDst = (cl_char*)m_pBstPtr;
            sCpyParam.pSrc = (cl_char*)m_pUsrPtr;
            sCpyParam.uiDimCount = m_dimCount;

            sCpyParam.vSrcPitch[0] = m_hostPtrRowPitch;
            sCpyParam.vSrcPitch[1] = m_hostPtrSlicePitch;

            sCpyParam.vDstPitch[0] = m_imageRowPitch;
            sCpyParam.vDstPitch[1] = m_imageSlicePitch;

            sCpyParam.vRegion[0] = m_imageWidth * GetImageElementSize(m_imageFormat);
            sCpyParam.vRegion[1] = m_imageHeight;
            sCpyParam.vRegion[2] = m_imageDepth;

            CopyMemoryBuffer(&sCpyParam);

            if (!(m_flags & CL_MEM_USE_HOST_PTR))
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

    cl_int errCode = CL_SUCCESS;

    *imageObj = this;

    // This is being deallocated in callback 'CrtMemDtorCallBack'
    CrtMemDtorCallBackData* memData = new CrtMemDtorCallBackData;
    if (NULL == memData)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    memData->m_count = (cl_uint)m_pContext->m_contexts.size();
    memData->m_clMemHandle = this;

    cl_uint numActualImages = 0;
    SHARED_CTX_DISPATCH::iterator itr = m_pContext->m_contexts.begin();
    for(;itr != m_pContext->m_contexts.end(); itr++)
    {
        cl_context ctx = itr->first;
        // - Force using Linear images
        // - some flags are mutual execlusive, so we need to turn off CL_MEM_COPY_HOST_PTR
        //     and CL_MEM_ALLOC_HOST_PTR since we are forcing CL_MEM_USE_HOST_PTR
        cl_mem_flags crtCreateFlags = ((m_flags | CL_MEM_USE_HOST_PTR | CL_MEM_LINEAR_IMAGE_INTEL ) & ~CL_MEM_ALLOC_HOST_PTR) & ~CL_MEM_COPY_HOST_PTR;
        cl_mem memObj  = NULL;
        if (m_imageType == CL_MEM_OBJECT_IMAGE2D)
        {
            memObj = ctx->dispatch->clCreateImage2D(
                ctx,
                crtCreateFlags,
                m_imageFormat,
                m_imageWidth,
                m_imageHeight,
                m_imageRowPitch,
                m_pBstPtr,
                &errCode);
        }
        else if (m_imageType == CL_MEM_OBJECT_IMAGE3D)
        {
            memObj = ctx->dispatch->clCreateImage3D(
                ctx,
                crtCreateFlags,
                m_imageFormat,
                m_imageWidth,
                m_imageHeight,
                m_imageDepth,
                m_imageRowPitch,
                m_imageSlicePitch,
                m_pBstPtr,
                &errCode);
        }

        if (CL_SUCCESS == errCode)
        {
            m_ContextToMemObj[ctx] = memObj;
            errCode = memObj->dispatch->clSetMemObjectDestructorCallback(
                memObj,
                CrtMemDestructorCallBack,
                (void*)memData);

            if (CL_SUCCESS != errCode)
            {
                goto FINISH;
        }
            numActualImages++;
        }
        else
        {
            goto FINISH;
        }
    }
FINISH:
    if (CL_SUCCESS != errCode)
    {
        if (numActualImages > 0)
        {
            memData->m_count = numActualImages;
        }
        else
        {
            delete memData;
        }
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
            stChannels = 1;
            break;
        case CL_RG:
        case CL_RA:
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
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CrtContext::CreateImage(
    cl_mem_object_type      image_type,
    cl_mem_flags            flags,
    const cl_image_format * image_format,
    size_t                  image_width,
    size_t                  image_height,
    size_t                  image_depth,
    size_t                  image_row_pitch,
    size_t                  image_slice_pitch,
    void *                  host_ptr,
    CrtMemObject**          imageObj)
{
    cl_int errCode = CL_SUCCESS;

        /// Not sure i'll need all these checks ToDo

    if ( ((flags & CL_MEM_READ_ONLY) && (flags & CL_MEM_WRITE_ONLY)) ||
        ((flags & CL_MEM_READ_ONLY) && (flags & CL_MEM_READ_WRITE)) ||
        ((flags & CL_MEM_WRITE_ONLY) && (flags & CL_MEM_READ_WRITE))||
        ((flags & CL_MEM_USE_HOST_PTR) && (flags & CL_MEM_ALLOC_HOST_PTR))
        )
    {
        return CL_INVALID_VALUE;
    }

    if ( (NULL == host_ptr) && ((0 != image_row_pitch) ||(0 != image_slice_pitch)) )
    {
        return CL_INVALID_IMAGE_SIZE;
    }

    if ( (NULL == host_ptr) && ((CL_MEM_COPY_HOST_PTR|CL_MEM_USE_HOST_PTR)&flags) )
    {
        return CL_INVALID_HOST_PTR;
    }

    if ( (NULL != host_ptr) && !((CL_MEM_COPY_HOST_PTR|CL_MEM_USE_HOST_PTR)&flags) )
    {
        return CL_INVALID_HOST_PTR;
    }

    size_t pixelBytesCnt = GetImageElementSize(image_format);
    if (0 == pixelBytesCnt)
    {
        return CL_INVALID_IMAGE_FORMAT_DESCRIPTOR;
    }

    // Check minumum row pitch size
    size_t szMinRowPitchSize = image_width * pixelBytesCnt;
    if ( (NULL != host_ptr) && (0 != image_row_pitch) && ((image_row_pitch<szMinRowPitchSize)||(image_row_pitch % pixelBytesCnt)) )
    {
        return CL_INVALID_IMAGE_SIZE;
    }

    size_t szMinSlicePitchSize = image_row_pitch * image_height;
    if ( (NULL != host_ptr) && (0 != image_slice_pitch) && ((image_slice_pitch<szMinSlicePitchSize)||(image_slice_pitch % pixelBytesCnt)) )
    {
        return CL_INVALID_IMAGE_SIZE;
    }

    CrtImage* image = new CrtImage(
        image_type,
        image_format,
        image_width,
        image_height,
        image_depth,
        flags,
        host_ptr,
        this);

    if (!image)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    errCode = image->Create(image_row_pitch, image_slice_pitch, imageObj);
    if (CL_SUCCESS != errCode)
    {
        image->Release();
        image->DecPendencyCnt();
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
    /// CreateBuffer
cl_int CrtContext::CreateBuffer(
    cl_mem_flags            flags,
    size_t                  size,
    void *                  host_ptr,
    CrtMemObject**          bufObj)
{
    cl_int errCode = CL_SUCCESS;

    if (size == 0)
    {
        return CL_INVALID_BUFFER_SIZE;
    }

    if ((flags & CL_MEM_USE_HOST_PTR || flags & CL_MEM_COPY_HOST_PTR) && NULL == host_ptr)
    {
        return CL_INVALID_HOST_PTR;
    }

    if (!(flags & CL_MEM_USE_HOST_PTR || flags & CL_MEM_COPY_HOST_PTR) && NULL != host_ptr)
    {
        return CL_INVALID_HOST_PTR;
    }

    CrtBuffer* buffer = new CrtBuffer(size, flags, host_ptr, this);
    if (!buffer)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    errCode = buffer->Create(bufObj);
    if (CL_SUCCESS != errCode)
    {
        buffer->Release();
        buffer->DecPendencyCnt();
    }
    return errCode;
}

    /// CreateBuffer
cl_int CrtContext::CreateSubBuffer(
    _cl_mem_crt*            parent_buffer,
    cl_mem_flags            flags,
    cl_buffer_create_type   buffer_create_type,
    const void *            buffer_create_info,
    CrtMemObject**          bufObj)
{
    cl_int errCode = CL_SUCCESS;

    if (flags & CL_MEM_USE_HOST_PTR || flags & CL_MEM_COPY_HOST_PTR || flags & CL_MEM_ALLOC_HOST_PTR)
    {
        return CL_INVALID_VALUE;
    }

    CrtBuffer* buffer = new CrtBuffer(parent_buffer, flags, this);
    errCode = buffer->Create(bufObj, buffer_create_type, buffer_create_info);
    if (CL_SUCCESS != errCode)
    {
        buffer->Release();
        buffer->DecPendencyCnt();
    }
    else
    {
        *bufObj = buffer;
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int  CrtContext::CreateCommandQueue(
    cl_device_id                device,
    cl_command_queue_properties properties,
    CrtQueue**                  crtQueue)
{
    cl_int errCode = CL_SUCCESS;
    CrtQueue* pCrtQueue = NULL;
    *crtQueue = NULL;

    DEV_CTX_MAP::iterator itr = m_DeviceToContext.find(device);
    if (itr == m_DeviceToContext.end())
    {
        return CL_INVALID_DEVICE;
    }

    cl_command_queue queueDEV = itr->second->dispatch->clCreateCommandQueue(itr->second, device, properties, &errCode);
    if (CL_SUCCESS == errCode)
    {
        pCrtQueue = new CrtQueue(this);
        if (NULL == pCrtQueue)
        {
            errCode = itr->second->dispatch->clReleaseCommandQueue(queueDEV);
            return CL_OUT_OF_HOST_MEMORY;
        }
        pCrtQueue->m_cmdQueueDEV = queueDEV;
        pCrtQueue->m_device = device;
        *crtQueue = pCrtQueue;
        m_commandQueues.push_back(queueDEV);
    }

    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
CrtKernel::CrtKernel(CrtProgram* program)
:m_programCRT(program)
{
    m_programCRT->IncPendencyCnt();
}

CrtKernel::~CrtKernel()
{
    m_programCRT->DecPendencyCnt();
}

CrtProgram::CrtProgram(CrtContext* ctx)
:m_contextCRT(ctx)
{
    m_contextCRT->IncPendencyCnt();
}

CrtProgram::~CrtProgram()
{
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
    cl_int errCode  = m_cmdQueueDEV->dispatch->clReleaseCommandQueue(m_cmdQueueDEV);
    m_contextCRT->m_commandQueues.remove(m_cmdQueueDEV);
    return errCode;
}

cl_int CrtSampler::Release()
{
    cl_int errCode = CL_SUCCESS;

    CTX_SMP_MAP::iterator itr = m_ContextToSampler.begin();
    for (;itr != m_ContextToSampler.end(); itr++)
    {
        errCode = itr->second->dispatch->clReleaseSampler(itr->second);
        if (errCode != CL_SUCCESS)
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
    if (!crtSampler)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    for(SHARED_CTX_DISPATCH::iterator itr = m_contexts.begin();
        itr != m_contexts.end();
        itr++)
    {
        cl_context ctx = itr->first;
        cl_sampler sampObj = ctx->dispatch->clCreateSampler(
            ctx,
            normalized_coords,
            addressing_mode,
            filter_mode,
            &errCode );

        if (CL_SUCCESS == errCode)
        {
            crtSampler->m_ContextToSampler[ctx] = sampObj;
        }
        else
        {
            break;
        }
    }

FINISH:
    if (CL_SUCCESS != errCode && crtSampler)
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
    for(;itr != m_ContextToProgram.end(); itr++)
    {
        cl_int ctxErrCode = itr->first->dispatch->clReleaseProgram(itr->second);
        if (CL_SUCCESS != ctxErrCode)
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
    for(;itr != m_ContextToKernel.end(); itr++)
    {
        cl_int ctxErrCode = itr->first->dispatch->clReleaseKernel(itr->second);
        if (CL_SUCCESS != ctxErrCode)
        {
            errCode = ctxErrCode;
        }
    }
    return errCode;
}
/// ------------------------------------------------------------------------------
///
/// ------------------------------------------------------------------------------
cl_int CrtContext::CreateProgramWithSource( cl_uint            count ,
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

    SHARED_CTX_DISPATCH::iterator itr = m_contexts.begin();
    for(;itr != m_contexts.end(); itr++)
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

    if (CL_SUCCESS == errCode)
    {
        pCrtProgram->m_contextCRT = this;
        *crtProgram = pCrtProgram;
    }
    else
    {
            /// Release all previously allocated underlying program objects
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

    CrtProgram *pCrtProgram = new CrtProgram(this);
    if( !pCrtProgram )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    const unsigned char ** fwdBinaries = new const unsigned char *[num_devices];
    if (NULL == fwdBinaries)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    cl_int* fwdBinaryStatus = new cl_int[num_devices];
    if (NULL == fwdBinaries)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    size_t* fwdLengths = new size_t[num_devices];
    if (NULL == fwdLengths)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

        cl_device_id* outDevices = new cl_device_id[num_devices];
        if (NULL == outDevices)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }

        cl_uint* indices = new cl_uint[num_devices];
        if (NULL == indices)
        {
            errCode = CL_OUT_OF_HOST_MEMORY;
            goto FINISH;
        }

    for (cl_uint i = 0; i < OCLCRT::crt_ocl_module.m_oclPlatforms.size(); i++)
    {

        cl_uint matchDevices = 0;
        GetDevsIndicesByPlatformId(
            num_devices,
            device_list,
            OCLCRT::crt_ocl_module.m_oclPlatforms[i]->m_platformIdDEV,
            &matchDevices,
            indices);

        if (matchDevices == 0)
        {
            continue;
        }
        for (cl_uint j=0; j < matchDevices; j++)
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

        if (CL_SUCCESS != errCode)
        {
            goto FINISH;
        }
        pCrtProgram->m_ContextToProgram[ctx] = prog;
    }

FINISH:
    if (indices)
    {
        delete indices;
    }
    if (outDevices)
    {
        delete outDevices;
    }
    if (fwdLengths)
    {
        delete[] fwdLengths;
    }
    if (fwdBinaryStatus)
    {
        delete[] fwdBinaryStatus;
    }
    if (fwdBinaries)
    {
        delete[] fwdBinaries;
    }

    if (CL_SUCCESS != errCode && pCrtProgram)
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

SyncManager::SyncManager()
{
    m_callBackData = NULL;
    m_outEventArray = NULL;
    m_postCallBackData = NULL;
}

cl_int SyncManager::PreExecuteMemSync(
    SYNC_WAY        syncWay,
    CrtQueue*       queue,
    CrtMemObject*   memObj,
    cl_uint         numDepEvents,
    cl_event*       depEvents,
    CrtEvent**      outEvent)
{
    cl_int errCode = CL_SUCCESS;

    cl_context ctx = queue->m_contextCRT->m_DeviceToContext[queue->m_device];
    (*outEvent)->m_eventDEV = ctx->dispatch->clCreateUserEvent(ctx, &errCode);
    if (CL_SUCCESS != errCode)
    {
        return errCode;
    }

    errCode = (*outEvent)->m_eventDEV->dispatch->clRetainEvent((*outEvent)->m_eventDEV);
    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    m_postCallBackData = new CrtMapUnmapCallBackData;
    if (!m_postCallBackData)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    m_postCallBackData->m_memObj = memObj;
    m_postCallBackData->m_syncWay = syncWay;
    m_postCallBackData->m_userEvent = *outEvent;
    m_postCallBackData->m_numDepEvents = numDepEvents;

    cl_uint i = 0;
    for (; i < numDepEvents; i++)
    {
        errCode = depEvents[i]->dispatch->clSetEventCallback(
                depEvents[i],
                CL_COMPLETE,
                CrtMapUnmapCallBack,
                m_postCallBackData);

        if (CL_SUCCESS != errCode)
        {
            break;
        }
    }
    if (i < numDepEvents)
    {
        long numExecuted = numDepEvents - i;
        long newVal = atomic_add_ret_prev(&(m_postCallBackData->m_numDepEvents), -numExecuted);
        if (newVal == 0)
        {
            delete m_postCallBackData;
        }
    }

FINISH:
    if (CL_SUCCESS != errCode)
    {
        (*outEvent)->m_eventDEV->dispatch->clReleaseEvent((*outEvent)->m_eventDEV);

        if (m_postCallBackData)
        {
            delete m_postCallBackData;
        }
    }
    return errCode;
}

cl_int SyncManager::PostExecuteMemSync(
    SYNC_WAY        syncWay,
    CrtQueue*       queue,
    CrtMemObject*   memObj,
    CrtEvent*       depEvent,
    CrtEvent**      outEvent)
{
    cl_int errCode = CL_SUCCESS;

    cl_context ctx = queue->m_contextCRT->m_DeviceToContext[queue->m_device];
    (*outEvent)->m_eventDEV = ctx->dispatch->clCreateUserEvent(ctx, &errCode);

    errCode = (*outEvent)->m_eventDEV->dispatch->clRetainEvent((*outEvent)->m_eventDEV);
    if (CL_SUCCESS != errCode)
    {
        goto FINISH;
    }

    m_postCallBackData = new CrtMapUnmapCallBackData;
    if (!m_postCallBackData)
    {
        errCode = CL_OUT_OF_HOST_MEMORY;
        goto FINISH;
    }

    m_postCallBackData->m_memObj = memObj;
    m_postCallBackData->m_syncWay = syncWay;
    m_postCallBackData->m_userEvent = *outEvent;
    m_postCallBackData->m_numDepEvents = 1;

    errCode = depEvent->m_eventDEV->dispatch->clSetEventCallback(
            depEvent->m_eventDEV,
            CL_COMPLETE,
            CrtMapUnmapCallBack,
            m_postCallBackData);

FINISH:
    if (CL_SUCCESS != errCode)
    {
        if (m_postCallBackData)
        {
            delete m_postCallBackData;
        }
    }
    return errCode;
}

cl_int SyncManager::PrepareToExecute(
    CrtQueue*       queue,
    cl_uint         NumEventsInWaitList,
    const cl_event* inEventWaitList,
    cl_uint*        numOutEvents,
    cl_event**      OutEvents,
    const cl_uint   numMemObjects,
    CrtMemObject**  memObjects)
{
    assert(numMemObjects <= 1);    

    cl_context dstContext = queue->m_contextCRT->m_DeviceToContext[queue->m_device];

    cl_int errCode = CL_SUCCESS;
    if (NumEventsInWaitList != 0)
    {
        m_outEventArray = new cl_event[NumEventsInWaitList];
    }

    m_eventRetained = false;
    cl_uint j = 0;
    for (cl_uint i=0; i < NumEventsInWaitList; i++)
    {
            /// Ohh... there are events belong to different undelying platforms
        const CrtEvent* waitEvent = (CrtEvent*)(
            (reinterpret_cast<const _cl_event_crt*>(inEventWaitList[i]))->object
            );

        cl_context evContext = NULL;
        cl_event eventDEV = NULL;

        if (waitEvent->m_isUserEvent)
        {
            eventDEV = ((CrtUserEvent*)waitEvent)->m_ContextToEvent[dstContext];
            evContext = dstContext;
        }
        else
        {
            eventDEV = waitEvent->m_eventDEV;
            evContext = waitEvent->m_queueCRT->m_contextCRT->m_DeviceToContext[waitEvent->m_queueCRT->m_device];
        }

            /// the target event (aiming for queue) is dependant on an event beloging to different
            /// underlying context.
        if (!waitEvent->m_isUserEvent && dstContext != evContext)
        {
                /// Initialization of callback and data is done only once
            if (!m_callBackData)
            {
                    /// Initialize crt callback data
                m_callBackData = new CrtEventCallBackData;
                if (!m_callBackData)
                {
                    errCode = CL_OUT_OF_HOST_MEMORY;
                    goto FINISH;
                }
                m_callBackData->numReqCalls = 0;
                m_callBackData->m_eventDEV = queue->m_cmdQueueDEV->dispatch->clCreateUserEvent(dstContext, &errCode);
                if (CL_SUCCESS != errCode)
                {
                    errCode = CL_OUT_OF_RESOURCES;
                    goto FINISH;
                }

                m_userEvent = m_callBackData->m_eventDEV;

                    /// The corresponding release happens in
                    ///     1) CrtEventCallBack - guarantees we manage to call setUserEventStatus
                    ///                           before it fires
                    ///     2) ~SyncManager - guarantees we manage to register
                    ////                      on it before it fires
                errCode = m_callBackData->m_eventDEV->dispatch->clRetainEvent(
                                m_callBackData->m_eventDEV);

                if (CL_SUCCESS != errCode)
                {
                    errCode = CL_OUT_OF_RESOURCES;
                    goto FINISH;
                }
                m_eventRetained = true;
                if (CL_SUCCESS != errCode)
                {
                    errCode = CL_OUT_OF_RESOURCES;
                    goto FINISH;
                }
                    /// annex the user event to the destination queue events
                m_outEventArray[j++] = m_callBackData->m_eventDEV;
            }

            m_callBackData->numReqCalls++;

            errCode = eventDEV->dispatch->clSetEventCallback(
                eventDEV, CL_COMPLETE, CrtEventCallBack, m_callBackData );

            if (CL_SUCCESS != errCode)
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
    if (0 == *numOutEvents)
    {
        *OutEvents = NULL;
    }
    else
        *OutEvents = m_outEventArray;

FINISH:
    if (CL_SUCCESS != errCode)
    {
        if (m_eventRetained)
        {
            m_callBackData->m_eventDEV->dispatch->clReleaseEvent(
                                m_callBackData->m_eventDEV);
            m_eventRetained = false;
        }
        if (m_callBackData)
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
    if (errCode != CL_SUCCESS)
    {
        if (m_postCallBackData)
        {
            delete m_postCallBackData;
            m_postCallBackData = NULL;
        }
    }

    if (m_eventRetained && m_userEvent)
    {
        m_userEvent->dispatch->clReleaseEvent(m_userEvent);
        m_eventRetained = false;
    }
    if (m_outEventArray)
    {
        delete[] m_outEventArray;
        m_outEventArray = NULL;
    }
}

SyncManager::~SyncManager()
{
    Release(CL_SUCCESS);
}


cl_int CrtEvent::Release()
{
    cl_int errCode = CL_SUCCESS;
    if (m_eventDEV)
    {
        /// Its enough to call Release once since we didn't
            /// forward Retains from CRT to the underlying platform
        errCode = m_eventDEV->dispatch->clReleaseEvent(m_eventDEV);
    }
    return errCode;
}

cl_int CrtUserEvent::Release()
{
    cl_int errCode = CL_SUCCESS;
        std::map<cl_context,cl_event>::iterator itr = m_ContextToEvent.begin();
        for(;itr != m_ContextToEvent.end(); itr++)
        {
                /// Its enought to call Release once since we didn't
                /// forward Retains from CRT to the underlying platform
        cl_int ctxErrCode = itr->second->dispatch->clReleaseEvent(itr->second);
        if (ctxErrCode != CL_SUCCESS)
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

        if (!sub.compare("cl_khr_gl_sharing"))
        {
            ext_keys |= CRT_CL_GL_EXT;
        }
        else if (!sub.compare("cl_khr_d3d10_sharing"))
        {
            ext_keys |= CRT_CL_D3D10_EXT;
        }
        else if (!sub.compare("cl_intel_d3d9_media_sharing"))
        {
            ext_keys |= CRT_CL_D3D9_EXT;
        }
    } while (iss);
    return ext_keys;
}
