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
                                unsigned long long maxBufferAllocSize )
{
    MemoryAllocator* instance = NULL;

    OclAutoMutex lock( &m_instance_guard );

    if (NULL == m_the_instance)
    {
        m_the_instance = new MemoryAllocator( devId, pLogDesc, maxBufferAllocSize );
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
    m_iDevId(devId), m_pLogDescriptor(logDesc), m_iLogHandle(0), m_maxAllocSize(maxAllocSize)
{
    if ( NULL != logDesc )
    {
        cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, TEXT("MIC Device: Memory Allocator"), &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            m_iLogHandle = 0;
        }
    }
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("MemoryAllocator Created"));
}

MemoryAllocator::~MemoryAllocator()
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("MemoryAllocator Distructed"));

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
    assert( NULL != pAllocProp );

	pAllocProp->bufferSharingGroupId = CL_DEV_MIC_BUFFER_SHARING_GROUP_ID;
	pAllocProp->imageSharingGroupId  = CL_DEV_MIC_IMAGE_SHARING_GROUP_ID;
    pAllocProp->hostUnified          = false;
    pAllocProp->alignment            = MIC_DEV_MAXIMUM_ALIGN;
    pAllocProp->maxBufferSize        = m_maxAllocSize;
    pAllocProp->imagesSupported      = true;
    pAllocProp->DXSharing            = false;
    pAllocProp->GLSharing            = false;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MemoryAllocator::CreateObject( cl_dev_subdevice_id node_id, cl_mem_flags flags, const cl_image_format* format,
					 size_t dim_count, const size_t* dim,
					 IOCLDevRTMemObjectService* pRTMemObjService,
                     IOCLDevMemoryObject*  *memObj )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("CreateObject enter"));

    assert(NULL != memObj);
    assert(NULL != dim);
    assert(MAX_WORK_DIM >= dim_count);

    // Allocate memory for memory object
    MICDevMemoryObject*    pMemObj = new MICDevMemoryObject(*this,
                                                            node_id, flags,
															pRTMemObjService);
    if ( NULL == pMemObj )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("%S"), TEXT("Memory Object allocation failed"));
        return CL_DEV_OBJECT_ALLOC_FAIL;
    }

    cl_dev_err_code rc = pMemObj->Init();
    if ( CL_DEV_FAILED(rc) )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, TEXT("Memory Object descriptor allocation failed, rc=%x"), rc);
        pMemObj->clDevMemObjRelease();
        return rc;
    }

    *memObj = pMemObj;

    return CL_DEV_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Private functions
size_t MemoryAllocator::CalculateOffset(cl_uint dim_count, const size_t* origin, const size_t* pitch, size_t elemSize)
{
    size_t offset = 0;

    if ( NULL != origin )
    {
        offset += origin[0] * elemSize; //Origin is in Pixels
        for(unsigned i=1; i<dim_count; ++i)
        {
            offset += origin[i] * pitch[i-1]; //y * image width pitch
        }
    }

    return offset;
}

//-----------------------------------------------------------------------------------------------------
// MICDevMemoryObject
//-----------------------------------------------------------------------------------------------------

MICDevMemoryObject::MICDevMemoryObject(MemoryAllocator& allocator,
                   cl_dev_subdevice_id nodeId, cl_mem_flags memFlags,
                   IOCLDevRTMemObjectService* pRTMemObjService):
    m_Allocator(allocator), m_nodeId(nodeId), m_memFlags(memFlags),
    m_pRTMemObjService(pRTMemObjService), m_pBackingStore(NULL),
    m_raw_size(0), m_coi_buffer(0)
{
	assert( NULL != m_pRTMemObjService);

	// Get backing store.
	cl_dev_err_code bsErr = m_pRTMemObjService->GetBackingStore(CL_DEV_BS_GET_ALWAYS, &m_pBackingStore);
	assert( CL_DEV_SUCCEEDED(bsErr) && (NULL != m_pBackingStore) && "Runtime did not allocated Backing Store object!");

    if (!CL_DEV_SUCCEEDED(bsErr) || (NULL == m_pBackingStore))
    {
        // error - nothing can be done
        m_pBackingStore = NULL;
        return;
    }

    m_pBackingStore->AddPendency();

    m_raw_size = m_pBackingStore->GetRawDataSize();

    m_objDescr.dim_count = m_pBackingStore->GetDimCount();

    if (1 == m_objDescr.dim_count)
    {
        m_objDescr.dimensions.buffer_size = m_raw_size;
    }
    else
    {
        memcpy( m_objDescr.dimensions.dim, m_pBackingStore->GetDimentions(), sizeof(m_objDescr.dimensions.dim) );
    }

    memcpy( m_objDescr.pitch, m_pBackingStore->GetPitch(), sizeof(m_objDescr.pitch) );

    m_objDescr.format        = m_pBackingStore->GetFormat();
    m_objDescr.uiElementSize = m_pBackingStore->GetElementSize();
    m_objDescr.pData         = m_pBackingStore->GetRawData();

    assert( (NULL != m_objDescr.pData) && (0 != m_raw_size) && "Runtime did not allocated raw data for backing store");
}

cl_dev_err_code MICDevMemoryObject::Init()
{
    if (NULL == m_pBackingStore)
    {
        return CL_DEV_OBJECT_ALLOC_FAIL;
    }

    // create COI buffer on top of allocated memory

    // we will need to filter MIC DAs from all devices supported by current memory object
    size_t rt_allocated_devices_count         = m_pRTMemObjService->GetDeviceAgentListSize();
    const IOCLDeviceAgent* const * rt_devices = m_pRTMemObjService->GetDeviceAgentList();

    MICDevice::TMicsSet  active_mics = MICDevice::FilterMicDevices(rt_allocated_devices_count, rt_devices);
    size_t               active_procs_count = active_mics.size();
    assert( active_procs_count > 0 && "Creating MIC buffer without active devices" );

    // allocate array on stack for simplicity - it will be very small
    COIPROCESS*  coi_processes = (COIPROCESS*)alloca( active_procs_count * sizeof(COIPROCESS) );
    assert( NULL != coi_processes && "Cannot allocate small array on a stack" );

    MICDevice::TMicsSet::iterator mic_it  = active_mics.begin();
    MICDevice::TMicsSet::iterator mic_end = active_mics.end();

    for(unsigned int i = 0; mic_it != mic_end; ++mic_it, ++i)
    {
        coi_processes[i] = (*mic_it)->GetDeviceService().getDeviceProcessHandle();
    }

    COIRESULT coi_err = COIBufferCreateFromMemory(
                                m_raw_size, COI_BUFFER_NORMAL, 0, m_objDescr.pData,
                                active_procs_count, coi_processes,
                                &m_coi_buffer);

    if (COI_SUCCESS != coi_err)
    {
        MicErrLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), TEXT("%S"), TEXT("Memory Object COI buffer Allocation failed"));
        return CL_DEV_OBJECT_ALLOC_FAIL;
    }

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjRelease( )
{
    COIRESULT coi_err = COI_SUCCESS;

    MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), TEXT("%S"), TEXT("ReleaseObject enter"));

    if (0 != m_coi_buffer)
    {
        coi_err = COIBufferDestroy( m_coi_buffer );
        assert( COI_SUCCESS == coi_err && "Cannot destroy COI Buffer" );
    }

    if (m_pBackingStore)
    {
	    m_pBackingStore->RemovePendency();
    }

    delete this;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle *handle)
{
    assert(NULL != handle);

	if (CL_DEVICE_TYPE_ACCELERATOR != dev_type)
	{
		*handle = NULL;
		return CL_DEV_INVALID_PROPERTIES;
	}

    *handle = this;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjCreateMappedRegion(cl_dev_cmd_param_map* pMapParams)
{
    MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), TEXT("%S"), TEXT("CreateMappedRegion enter"));

	// Assume that calling this method only once.
    SMemMapParamsList* coi_params = new SMemMapParamsList;
    assert( coi_params && "Cannot allocate coi_params record" );
    if (NULL == coi_params)
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    pMapParams->memObj = this;

    assert(pMapParams->dim_count == m_objDescr.dim_count);
    pMapParams->ptr = (cl_uchar*)m_objDescr.pData + m_pBackingStore->GetRawDataOffset( pMapParams->origin );
    MEMCPY_S(pMapParams->pitch, sizeof(size_t)*(MAX_WORK_DIM-1), m_objDescr.pitch, sizeof(size_t)*(m_objDescr.dim_count-1));

    // set map handle
    pMapParams->map_handle = coi_params;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams )
{
    MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), TEXT("%S"), TEXT("ReleaseMappedRegion enter"));
    assert( NULL != pMapParams->map_handle && "cl_dev_cmd_param_map was not filled by MIC Device" );

    SMemMapParamsList* coi_params = MemoryAllocator::GetCoiMapParams(pMapParams);
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
    MEMCPY_S(&m_objDescr, sizeof(cl_mem_obj_descriptor), &m_Parent.m_objDescr, sizeof(cl_mem_obj_descriptor));

    m_nodeId           = m_Parent.m_nodeId;
    m_pRTMemObjService = m_Parent.m_pRTMemObjService;
    m_pBackingStore    = m_Parent.m_pBackingStore;

    m_pBackingStore->AddPendency();

    m_objDescr.pData = (cl_uchar*)m_objDescr.pData + m_pBackingStore->GetRawDataOffset( origin );

    // Update dimensions
    if ( 1 == m_objDescr.dim_count )
    {
        m_objDescr.dimensions.buffer_size = size[0];
        m_raw_size = size[0];
    }
    else
    {
        for(int i=0; i<MAX_WORK_DIM; ++i)
        {
            m_objDescr.dimensions.dim[i] = (unsigned int)size[i];
        }

        m_raw_size = m_objDescr.pitch[ m_objDescr.dim_count - 1 ] * m_objDescr.dimensions.dim[ m_objDescr.dim_count ];
    }

    m_memFlags = mem_flags;

     // BUGBUG: DK - need to create COI sub-buffer
//    m_coi_buffer =

    return CL_DEV_SUCCESS;
}
