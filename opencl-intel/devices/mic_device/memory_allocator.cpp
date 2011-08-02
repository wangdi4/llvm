// Copyright (c) 2006-2008 Intel Corporation
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

///////////////////////////////////////////////////////////
//  MemoryAllocator.cpp
//  Implementation of the Class MemoryAllocator
//  Created on:      16-Dec-2008 4:54:53 PM
//  Original author: efiksman
///////////////////////////////////////////////////////////

#include "pragmas.h"
#include "memory_allocator.h"
#include "mic_logger.h"
#include "mic_dev_limits.h"
#include "mic_device.h"
#include "cl_sys_defines.h"
#include "device_service_communication.h"

#include <source/COIBuffer_source.h>

#include<stdlib.h>
#include <alloca.h>

// The flag below enables a check that allows only a single use of cl_mem objects
// The "lock" on the memory object is obtained during the call to NDRange->Init() and is released when the kernel is done executing
// This is useful only for debugging - it is not conformant to the spec
// So, keep the flag undefined unless you are debugging potential race conditions in kernel executions
//#define _ENABLE_LOCK_OBJECTS_

using namespace Intel::OpenCL::MICDevice;

OclMutex         MemoryAllocator::m_instance_guard;
MemoryAllocator* MemoryAllocator::m_the_instance = NULL;
cl_uint          MemoryAllocator::m_instance_referencies = 0;

MemoryAllocator* MemoryAllocator::getMemoryAllocator(
                                cl_int devId,
                                IOCLDevLogDescriptor *pLogDesc,
                                unsigned long long maxAllocSize )
{
    MemoryAllocator* instance = NULL;

    OclAutoMutex lock( &m_instance_guard );

    if (NULL == m_the_instance)
    {
        m_the_instance = new MemoryAllocator( devId, pLogDesc, maxAllocSize );
        assert( NULL != m_the_instance && "Creating MIC MemoryAllocator singleton" );
    }

    instance = m_the_instance;
    ++m_instance_referencies;

    assert( m_instance_referencies > 0 && "MIC MemoryAllocator singleton - error in refcounter" );

    return instance;
}

void MemoryAllocator::Release( void )
{
    OclAutoMutex lock( &m_instance_guard );

    assert( m_instance_referencies > 0 && "MIC MemoryAllocator singleton - error in refcounter" );

    --m_instance_referencies;

    if (0 == m_instance_referencies)
    {
        assert( NULL != m_the_instance && "Releasing MIC MemoryAllocator singleton" );
        delete m_the_instance; // this is the same as this!!!! Do not access non-static members after!
        m_the_instance = NULL;
    }
}

MemoryAllocator::MemoryAllocator(cl_int devId, IOCLDevLogDescriptor *logDesc, unsigned long long maxAllocSize ):
    m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0), m_lclHeap(NULL)
{
    if ( NULL != logDesc )
    {
        cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, TEXT("MIC Device: Memory Allocator"), &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            m_iLogHandle = 0;
        }
    }

    clCreateHeap(0, (size_t)maxAllocSize, &m_lclHeap);
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("MemoryAllocator Created"));
}

MemoryAllocator::~MemoryAllocator()
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("MemoryAllocator Distructed"));

    clDeleteHeap( m_lclHeap );

    if (0 != m_iLogHandle)
    {
        m_pLogDescriptor->clLogReleaseClient(m_iLogHandle);
    }
}

/****************************************************************************************************************
 GetSupportedImageFormats
    Description
        This function returns the list of image formats supported by an OCL implementation when the information about
        an image memory object is specified and device supports image objects.
    Input
        flags                    A bit-field that is used to specify allocation and usage information such as the memory arena
                                that should be used to allocate the image object and how it will be used.
        imageType                Describes the image type as described in (cl_dev_mem_object_type).Only image formats are supported.
        numEntries                Specifies the number of entries that can be returned in the memory location given by formats.
                                If value is 0 and formats is NULL, the num_entries_ret returns actual number of supported formats.
    Output
        formats                    A pointer to array of structures that describes format properties of the image to be allocated.
                                Refer to OCL spec section 5.2.4.1 for a detailed description of the image format descriptor.
        numEntriesRet            The actual number of supported image formats for a specific context and values specified by flags.
                                If the value is NULL, it is ignored.
        Description
                                Return the minimum number of image formats that should be supported according to Spec
     Returns
        CL_DEV_SUCCESS            The function is executed successfully.
        CL_DEV_INVALID_VALUE    If values specified in parameters is not valid or if num_entries is 0 and formats is not NULL.
********************************************************************************************************************/
cl_dev_err_code MemoryAllocator::GetSupportedImageFormats( cl_mem_flags IN flags, cl_mem_object_type IN imageType,
                cl_uint IN numEntries, cl_image_format* OUT formats, cl_uint* OUT numEntriesRet)
{
    //image_type describes the image type and must be either CL_MEM_OBJECT_IMAGE2D or
    //CL_MEM_OBJECT_IMAGE3D
    if(CL_MEM_OBJECT_BUFFER == imageType)
    {
        return CL_DEV_INVALID_VALUE;
    }

    if(0 == numEntries && NULL != formats)
    {
        return CL_DEV_INVALID_VALUE;
    }

    unsigned int uiNumEntries = NUM_OF_SUPPORTED_IMAGE_FORMATS;

    if(NULL != formats)
    {
        uiNumEntries = min(uiNumEntries, numEntries);
        memcpy(formats, suportedImageFormats, uiNumEntries * sizeof(suportedImageFormats[0]));
    }
    if(NULL != numEntriesRet)
    {
        *numEntriesRet = uiNumEntries;
    }

    return CL_DEV_SUCCESS;
}
/****************************************************************************************************************
 GetAllocProperties
    Description
        This function return allocator properties per memory object
    Input
        format                     Image format
********************************************************************************************************************/
cl_dev_err_code MemoryAllocator::GetAllocProperties( cl_mem_object_type IN memObjType,    cl_dev_alloc_prop* OUT pAllocProp )
{
    assert( NULL == pAllocProp );

    pAllocProp->supportedDevices = CL_DEVICE_TYPE_ACCELERATOR;
    pAllocProp->hostUnified = false;
    pAllocProp->alignment = MIC_DEV_MAXIMUM_ALIGN;
    pAllocProp->DXSharing = false;
    pAllocProp->GLSharing = false;

    return CL_DEV_SUCCESS;
}

/****************************************************************************************************************
 ElementSize
    Description
        This function calculated the expected element size
    Input
        format                     Image format
********************************************************************************************************************/
size_t MemoryAllocator::GetElementSize(const cl_image_format* format)
{
    size_t stChannels = 0;
    size_t stChSize = 0;
    switch (format->image_channel_order)
    {
    case CL_R:case CL_A:case CL_LUMINANCE:case CL_INTENSITY:
    case CL_RGB:    // Special case, must be used only with specific data type
        stChannels = 1;
        break;
    case CL_RG:case CL_RA:
        stChannels = 2;
        break;
    case CL_RGBA: case CL_ARGB: case CL_BGRA:
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


cl_dev_err_code MemoryAllocator::CreateObject( cl_dev_subdevice_id node_id, cl_mem_flags flags, const cl_image_format* format,
                     size_t dim_count, const size_t* dim, void* buffer_ptr, const size_t* pitch,
                     cl_dev_host_ptr_flags host_flags,
                     IOCLDevMemoryObject*  *memObj )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateObject enter"));

    if ( NULL == m_lclHeap)
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Device heap was not created"));
        return CL_DEV_OBJECT_ALLOC_FAIL;
    }
    size_t uiElementSize = 1;

    assert(NULL != memObj);
    assert(NULL != dim);
    assert(MAX_WORK_DIM >= dim_count);

    if ( dim_count > 1)
    {
        uiElementSize = GetElementSize(format);
    }

    // Allocate memory for memory object
    MICDevMemoryObject*    pMemObj = new MICDevMemoryObject(*this,
                                                            node_id, flags,
                                                            format, uiElementSize,
                                                            dim_count, dim,
                                                            buffer_ptr, pitch,
                                                            host_flags);
    if ( NULL == pMemObj )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Memory Object allocation failed"));
        return CL_DEV_OBJECT_ALLOC_FAIL;
    }

    cl_dev_err_code rc = pMemObj->Init();
    if ( CL_DEV_FAILED(rc) )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Memory Object descriptor allocation failed, rc=%x"), rc);
        delete pMemObj;
        return rc;
    }

    *memObj = pMemObj;

    return CL_DEV_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Private functions
void* MemoryAllocator::CalculateOffsetPointer(void* pBasePtr, cl_uint dim_count, const size_t* origin, const size_t* pitch, size_t elemSize)
{
    char* lockedPtr = (char*)pBasePtr;
    if ( NULL != origin )
    {
        lockedPtr += origin[0] * elemSize; //Origin is in Pixels
        for(unsigned i=1; i<dim_count; ++i)
        {
            lockedPtr += origin[i] * pitch[i-1]; //y * image width pitch
        }
    }

    return lockedPtr;
}
void MemoryAllocator::CopyMemoryBuffer(SMemCpyParams* pCopyCmd)
{
    // Copy 1D array only
    if ( 1 == pCopyCmd->uiDimCount )
    {
        memcpy(pCopyCmd->pDst, pCopyCmd->pSrc, pCopyCmd->vRegion[0]);
        return;
    }

    SMemCpyParams sRecParam;

    // Copy current parameters
    memcpy(&sRecParam, pCopyCmd, sizeof(SMemCpyParams));
    sRecParam.uiDimCount = pCopyCmd->uiDimCount-1;
    // Make recursion
    for(unsigned int i=0; i<pCopyCmd->vRegion[sRecParam.uiDimCount]; ++i)
    {
        CopyMemoryBuffer(&sRecParam);
        sRecParam.pSrc = sRecParam.pSrc + pCopyCmd->vSrcPitch[sRecParam.uiDimCount-1];
        sRecParam.pDst = sRecParam.pDst + pCopyCmd->vDstPitch[sRecParam.uiDimCount-1];
    }
}

//-----------------------------------------------------------------------------------------------------
// MICDevMemoryObject
//-----------------------------------------------------------------------------------------------------

MICDevMemoryObject::MICDevMemoryObject(MemoryAllocator& allocator,
                   cl_dev_subdevice_id nodeId, cl_mem_flags memFlags,
                   const cl_image_format* pImgFormat, size_t elemSize,
                   size_t dimCount, const size_t* dim,
                   void* pBuffer, const size_t* pPitches,
                   cl_dev_host_ptr_flags hostFlags): m_Allocator(allocator),
m_nodeId(nodeId), m_memFlags(memFlags), m_hostPtrFlags(hostFlags), m_pHostPtr(pBuffer),
m_coi_buffer(0), m_bAlocated(false)
{
    m_objDecr.dim_count = (cl_uint)dimCount;
    if ( NULL != pImgFormat )
    {
        // Convert from User to Kernel format
        m_objDecr.format.image_channel_data_type = pImgFormat->image_channel_data_type - CL_SNORM_INT8;
        m_objDecr.format.image_channel_order = pImgFormat->image_channel_order - CL_R;
    }
    if ( 1 == dimCount )
    {
        m_objDecr.dimensions.buffer_size = dim[0];
    } else
    {
        for (size_t i=0; i<MAX_WORK_DIM; ++i)
        {
            m_objDecr.dimensions.dim[i] = (unsigned int)dim[i];
        }
    }
    if ( NULL != pPitches )
    {
        for(unsigned int i=0; i<m_objDecr.dim_count-1; i++)
        {
            m_hostPitch[i] = pPitches[i];
        }
    }
    else
    {
        for(unsigned int i=0; i<m_objDecr.dim_count-1; i++)
        {
            m_hostPitch[i] = 0;
        }
    }
    m_objDecr.uiElementSize = (unsigned int)elemSize;
    m_objDecr.pData = NULL;
}

cl_dev_err_code MICDevMemoryObject::Init()
{
    bool bPtrNotAligned = ((CL_DEV_HOST_PTR_USER_MAPPED_REGION & m_hostPtrFlags)!=0) &&
        ( ((((size_t)m_pHostPtr & (MIC_DEV_MAXIMUM_ALIGN-1)) != 0) && (1 == m_objDecr.dim_count)) ||
          ((((size_t)m_pHostPtr & (MIC_DEV_IMAGE_ALIGN-1)) != 0) && (1 != m_objDecr.dim_count))
        );

    // Calculate total memory required for allocation
    size_t allocSize;
    if ( 1 == m_objDecr.dim_count )
    {
        allocSize = m_objDecr.dimensions.buffer_size;
    }
    else
    {
        allocSize = m_objDecr.uiElementSize;
        for(unsigned int i=0; i<m_objDecr.dim_count; ++i)
        {
            allocSize*=m_objDecr.dimensions.dim[i];
        }
    }

    // Allocate memory internally
    // 1. No host pointer provided
    // 2. Pointer provided by RT is not alligned and it's working area
    // 3. Host pointer contains user data (not working area)
    if ( NULL == m_pHostPtr || bPtrNotAligned || (CL_DEV_HOST_PTR_DATA_AVAIL == m_hostPtrFlags) )
    {
        m_objDecr.pData = clAllocateFromHeap(m_Allocator.GetHeap(), allocSize, MIC_DEV_MAXIMUM_ALIGN);
        if ( NULL == m_objDecr.pData )
        {
            MicErrLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), TEXT("%S"), TEXT("Memory Object memory buffer Allocation failed"));
            return CL_DEV_OBJECT_ALLOC_FAIL;
        }
        m_bAlocated = true;

        // For images only
        size_t prev_pitch = m_objDecr.uiElementSize;
        for(unsigned int i=0; i<m_objDecr.dim_count-1; i++)
        {
            m_objDecr.pitch[i] = prev_pitch*m_objDecr.dimensions.dim[i];
            prev_pitch = m_objDecr.pitch[i];
        }
    } else
    {
        // The object pointers to external buffer
        m_objDecr.pData = m_pHostPtr;
        // Also need to update object pitches
        for(unsigned int i=0; i<m_objDecr.dim_count-1; i++)
        {
            m_objDecr.pitch[i] = m_hostPitch[i];
        }
    }

    // create COI buffer on top of allocated memory

    // BUGBUG: DK - Runtime should privide service that returns list of Device Agents for buffer
    // we will need to filter MIC DAs from there
    // temporaryget the full list of MIC DAs
    MICDevice::TMicsList active_mics = MICDevice::GetActiveMicDevices();
    size_t               active_procs_count = active_mics.size();
    assert( active_procs_count > 0 && "Creating MIC buffer without active devices" );

    // allocate array on stack for simplicity - it will be very small
    COIPROCESS*  coi_processes = (COIPROCESS*)alloca( active_procs_count * sizeof(COIPROCESS) );
    assert( NULL != coi_processes && "Cannot allocate small array on a stack" );

    MICDevice::TMicsList::iterator mic_it  = active_mics.begin();
    MICDevice::TMicsList::iterator mic_end = active_mics.end();

    for(unsigned int i = 0; mic_it != mic_end; ++mic_it, ++i)
    {
        coi_processes[i] = (*mic_it)->GetDeviceService().getDeviceProcessHandle();
    }

    COIRESULT coi_err = COIBufferCreateFromMemory(
                                allocSize, COI_BUFFER_NORMAL, 0, m_objDecr.pData,
                                active_procs_count, coi_processes,
                                &m_coi_buffer);

    if (COI_SUCCESS != coi_err)
    {
        MicErrLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), TEXT("%S"), TEXT("Memory Object COI buffer Allocation failed"));

        if (m_bAlocated)
        {
            clFreeHeapPointer(m_Allocator.GetHeap(), m_objDecr.pData);
        }
        return CL_DEV_OBJECT_ALLOC_FAIL;
    }

    // Copy initial data if required:
    //        Data available and our PTR is not HOST ptr
    if ( (m_hostPtrFlags & CL_DEV_HOST_PTR_DATA_AVAIL) && (NULL != m_pHostPtr) && (m_objDecr.pData != m_pHostPtr) )
    {
        SMemCpyParams sCpyPrm;

        sCpyPrm.uiDimCount = m_objDecr.dim_count;
        sCpyPrm.pSrc = (cl_char*)m_pHostPtr;

        sCpyPrm.pDst = (cl_char*)m_objDecr.pData;
        for(unsigned int i=0; i<m_objDecr.dim_count-1; i++)
        {
            sCpyPrm.vDstPitch[i] = m_objDecr.pitch[i];
            sCpyPrm.vSrcPitch[i] = m_hostPitch[i];
        }
        if ( 1 == m_objDecr.dim_count )
        {
            sCpyPrm.vRegion[0] = m_objDecr.dimensions.buffer_size;
        }
        else
        {
            for(unsigned int i=0; i< m_objDecr.dim_count; i++)
            {
                sCpyPrm.vRegion[i] = m_objDecr.dimensions.dim[i];
            }
            sCpyPrm.vRegion[0] = sCpyPrm.vRegion[0] * m_objDecr.uiElementSize;
        }
        // Copy original buffer to internal area
        MemoryAllocator::CopyMemoryBuffer(&sCpyPrm);
    }

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjRelease( )
{
    MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), TEXT("%S"), TEXT("ReleaseObject enter"));

    if (0 != m_coi_buffer)
    {
        COIRESULT coi_err = COIBufferDestroy( m_coi_buffer );
        assert( COI_SUCCESS == coi_err && "Cannot destroy COI Buffer" );
    }

    // Did we allocate the buffer and not sub-object
    if ( m_bAlocated )
    {
        clFreeHeapPointer(m_Allocator.GetHeap(), m_objDecr.pData);
    }

    delete this;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle *handle)
{
    assert(NULL != handle);

    *handle = (void*)(&m_objDecr);
    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjCreateMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
    MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), TEXT("%S"), TEXT("CreateMappedRegion enter"));

    SMemMapParams* coi_params = new SMemMapParams;
    assert( coi_params && "Cannot allocate coi_params record" );
    if (NULL == coi_params)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    pMapParams->memObj = this;

    void*    pMapPtr = NULL;
    // Determine which pointer to use
    pMapPtr = m_hostPtrFlags & CL_DEV_HOST_PTR_USER_MAPPED_REGION ? m_pHostPtr : m_objDecr.pData;
    size_t*    pitch = m_hostPtrFlags & CL_DEV_HOST_PTR_USER_MAPPED_REGION ? m_hostPitch : m_objDecr.pitch;

    assert(pMapParams->dim_count == m_objDecr.dim_count);
    pMapParams->ptr = MemoryAllocator::CalculateOffsetPointer(pMapPtr, m_objDecr.dim_count, pMapParams->origin, pitch, m_objDecr.uiElementSize);
    MEMCPY_S(pMapParams->pitch, sizeof(size_t)*(MAX_WORK_DIM-1), pitch, sizeof(size_t)*(m_objDecr.dim_count-1));

    // init coi_params
    pMapParams->map_handle = coi_params;

    // calculate size of the mapped region
    if ( 1 == pMapParams->dim_count )
    {
        coi_params->size = pMapParams->region[0];
    }
    else
    {
        coi_params->size = m_objDecr.uiElementSize;
        for(unsigned int i=0; i<pMapParams->dim_count ; ++i)
        {
            coi_params->size *= pMapParams->region[i];
        }
    }

    coi_params->coi_buffer = m_coi_buffer;
    coi_params->offset     = (char*)pMapParams->ptr - (char*)pMapPtr;
    coi_params->map_handle = NULL;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams )
{
    MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), TEXT("%S"), TEXT("ReleaseMappedRegion enter"));
    assert( NULL != pMapParams->map_handle && "cl_dev_cmd_param_map was not filled by MIC Device" );

    SMemMapParams* coi_params = MemoryAllocator::GetCoiMapParams(pMapParams);
    delete coi_params;
    pMapParams->map_handle = NULL;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjCreateSubObject( cl_mem_flags mem_flags, const size_t *origin,
                                           const size_t *size, IOCLDevMemoryObject** ppSubBuffer )
{
    MICDevMemorySubObject* pSubObject = new MICDevMemorySubObject(m_Allocator, *this);
    if ( NULL == pSubObject )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    cl_dev_err_code devErr = pSubObject->Init(mem_flags, origin, size);
    if ( CL_DEV_FAILED(devErr) )
    {
        delete pSubObject;
        return devErr;
    }

    assert (NULL != ppSubBuffer);
    *ppSubBuffer = pSubObject;
    return CL_DEV_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
MICDevMemorySubObject::MICDevMemorySubObject(MemoryAllocator& allocator, MICDevMemoryObject& pParent) :
    MICDevMemoryObject(allocator), m_Parent(pParent)

{
}

cl_dev_err_code MICDevMemorySubObject::Init(cl_mem_flags mem_flags, const size_t *origin, const size_t *size)
{
    MEMCPY_S(&m_objDecr, sizeof(cl_mem_obj_descriptor), &m_Parent.m_objDecr, sizeof(cl_mem_obj_descriptor));

    // Update dimensions
    m_objDecr.pData = MemoryAllocator::CalculateOffsetPointer(m_objDecr.pData, m_objDecr.dim_count, origin, m_objDecr.pitch, m_objDecr.uiElementSize);

    if ( 1 == m_objDecr.dim_count )
    {
        m_objDecr.dimensions.buffer_size = size[0];
    }
    else
    {
        for(int i=0; i<MAX_WORK_DIM; ++i)
        {
            m_objDecr.dimensions.dim[i] = (unsigned int)size[i];
        }
    }

    m_memFlags = mem_flags;

    if ( NULL != m_Parent.m_pHostPtr )
    {
        // Set appropriate host pointer values
        m_pHostPtr = MemoryAllocator::CalculateOffsetPointer(m_Parent.m_pHostPtr, m_objDecr.dim_count, origin, m_objDecr.pitch, m_objDecr.uiElementSize);
        MEMCPY_S(m_hostPitch, sizeof(size_t)*(MAX_WORK_DIM-1), m_Parent.m_hostPitch, sizeof(size_t)*(m_objDecr.dim_count-1));
        m_hostPtrFlags = m_Parent.m_hostPtrFlags;
    }

    // BUGBUG: DK - need to create COI sub-buffer
//    m_coi_buffer =

    return CL_DEV_SUCCESS;
}

