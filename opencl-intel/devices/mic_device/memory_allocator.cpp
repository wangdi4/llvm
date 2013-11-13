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
#include "mic_common_macros.h"
#include "mic_sys_info.h"
#include "command_list.h"

#include <source/COIBuffer_source.h>
#include <source/COIProcess_source.h>

#include<stdlib.h>

#define KILOBYTE 1024
#define MEGABYTE (1024*KILOBYTE)

// The flag below enables a check that allows only a single use of cl_mem objects
// The "lock" on the memory object is obtained during the call to NDRange->Init() and is released when the kernel is done executing
// This is useful only for debugging - it is not conformant to the spec
// So, keep the flag undefined unless you are debugging potential race conditions in kernel executions
//#define _ENABLE_LOCK_OBJECTS_

using namespace Intel::OpenCL::MICDevice;

OclMutex*        MemoryAllocator::m_instance_guard = NULL;
MemoryAllocator* MemoryAllocator::m_the_instance = NULL;
cl_uint          MemoryAllocator::m_instance_referencies = 0;

//
// Helper class to allocate required static members
//
class MemoryAllocator::StaticInitializer
{
public:
    StaticInitializer()
    {
        m_instance_guard = new OclMutex;
    };
};

// init all static classes
MemoryAllocator::StaticInitializer MemoryAllocator::init_statics;

MemoryAllocator* MemoryAllocator::getMemoryAllocator(
                                cl_int devId,
                                IOCLDevLogDescriptor *pLogDesc,
                                unsigned long long maxBufferAllocSize )
{
    MemoryAllocator* instance = NULL;

    OclAutoMutex lock( m_instance_guard );

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
    OclAutoMutex lock( m_instance_guard );

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
	const MICDeviceConfig& tMicConfig = MICSysInfo::getInstance().getMicDeviceConfig();
    m_2M_BufferMinSize         = tMicConfig.Device_2MB_BufferMinSizeInKB() * KILOBYTE;

    if ((0 < m_2M_BufferMinSize) && (m_2M_BufferMinSize <= PAGE_4K_SIZE))
    {
        // no need to use 2MB pages for buffers less or equal to 4KB page
        m_2M_BufferMinSize = PAGE_4K_SIZE + 1;
    }
    
    m_force_immediate_transfer = !(tMicConfig.Device_LazyTransfer());
    
    if ( NULL != logDesc )
    {
        cl_int ret = m_pLogDescriptor->clLogCreateClient(m_iDevId, "MIC Device: Memory Allocator", &m_iLogHandle);
        if(CL_DEV_SUCCESS != ret)
        {
            m_iLogHandle = 0;
        }
    }
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "MemoryAllocator Created");
}

MemoryAllocator::~MemoryAllocator()
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "MemoryAllocator Distracted");

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
	pAllocProp->usedByDMA            = true;
    pAllocProp->alignment            = MIC_DEV_MAXIMUM_ALIGN;
    pAllocProp->preferred_alignment  = PAGE_4K_SIZE;
    pAllocProp->maxBufferSize        = m_maxAllocSize;
    pAllocProp->imagesSupported      = (MIC_IMAGES_SUPPORT == CL_TRUE);
    pAllocProp->DXSharing            = false;
    pAllocProp->GLSharing            = false;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MemoryAllocator::CreateObject( cl_dev_subdevice_id node_id, cl_mem_flags flags, const cl_image_format* format,
					 size_t dim_count, const size_t* dim,
					 IOCLDevRTMemObjectService* pRTMemObjService,
                     IOCLDevMemoryObject*  *memObj )
{
    MicInfoLog(m_pLogDescriptor, m_iLogHandle, "%s", "CreateObject enter");

    assert(NULL != memObj);
    assert(NULL != dim);
    assert(MAX_WORK_DIM >= dim_count);

    // Allocate memory for memory object
    MICDevMemoryObject*    pMemObj = new MICDevMemoryObject(*this,
                                                            node_id, flags,
															pRTMemObjService);
    if ( NULL == pMemObj )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, "%s", "Memory Object allocation failed");
        return CL_DEV_OBJECT_ALLOC_FAIL;
    }

    cl_dev_err_code rc = pMemObj->Init();
    if ( CL_DEV_FAILED(rc) )
    {
        MicErrLog(m_pLogDescriptor, m_iLogHandle, "Memory Object descriptor allocation failed, rc=%x", rc);
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

MICDevMemoryObject::MapBuffersMemoryPool MICDevMemoryObject::m_buffersMemoryPool;

MICDevMemoryObject::COI_ProcessesArray MICDevMemoryObject::get_active_processes_int( void )
{
    COI_ProcessesArray coi_processes;
    
    // we will need to filter MIC DAs from all devices supported by current memory object
    size_t rt_allocated_devices_count         = m_pRTMemObjService->GetDeviceAgentListSize();
    const IOCLDeviceAgent* const * rt_devices = m_pRTMemObjService->GetDeviceAgentList();

    MICDevice::TMicsSet  active_mics = MICDevice::FilterMicDevices(rt_allocated_devices_count, rt_devices);
    size_t               active_procs_count = active_mics.size();
    assert( active_procs_count > 0 && "Creating MIC buffer without active devices" );

    coi_processes.resize( active_procs_count );

    MICDevice::TMicsSet::iterator mic_it  = active_mics.begin();
    MICDevice::TMicsSet::iterator mic_end = active_mics.end();

    for(unsigned int i = 0; mic_it != mic_end; ++mic_it, ++i)
    {
        coi_processes[i] = (*mic_it)->GetDeviceService().getDeviceProcessHandle();
    }

    return coi_processes;
}

MICDevice* MICDevMemoryObject::get_owning_device( void )
{
    // we will need to filter MIC DAs from all devices supported by current memory object
    size_t rt_allocated_devices_count         = m_pRTMemObjService->GetDeviceAgentListSize();
    const IOCLDeviceAgent* const * rt_devices = m_pRTMemObjService->GetDeviceAgentList();

    MICDevice::TMicsSet  active_mics = MICDevice::FilterMicDevices(rt_allocated_devices_count, rt_devices);
    size_t               active_procs_count = active_mics.size();
    assert( active_procs_count > 0 && "Creating MIC buffer without active devices" );

    MICDevice::TMicsSet::iterator mic_it  = active_mics.begin();

    return (active_procs_count > 0) ? *mic_it : NULL;
}

MICDevMemoryObject::MICDevMemoryObject(MemoryAllocator& allocator,
                   cl_dev_subdevice_id nodeId, cl_mem_flags memFlags,
                   IOCLDevRTMemObjectService* pRTMemObjService):
    m_Allocator(allocator), m_nodeId(nodeId), m_memFlags(memFlags),
    m_pRTMemObjService(pRTMemObjService), m_pBackingStore(NULL),
    m_raw_size(0), m_coi_buffer(0), m_coi_top_level_buffer(0), m_coi_top_level_buffer_offset(0)
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
    m_objDescr.memObjType    = m_pRTMemObjService->GetMemObjectType();

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
    m_buffActiveProcesses = get_active_processes_int();

    //
    // COI can use 2 different resource pools - 2MB pages and 4K pages. It may be that one of pools is already 
    // exhausted, while another - not.
    // try both in an order of required precedence 
    uint32_t flags[2] = {0, 0}; 
    flags [ (m_Allocator.Use_2M_Pages(m_raw_size)) ? 0 : 1] = (m_Allocator.Use_2M_Pages_Enabled()) ? COI_OPTIMIZE_HUGE_PAGE_SIZE : 0;

    COIRESULT coi_err;
    
    for (unsigned int i = 0; i < ARRAY_ELEMENTS(flags); ++i)
    {
        coi_err = COIBufferCreateFromMemory(
                                m_raw_size, COI_BUFFER_OPENCL, 
                                flags[i],
                                m_objDescr.pData,
                                m_buffActiveProcesses.size(), &(m_buffActiveProcesses[0]),
                                &m_coi_buffer);

        if (COI_RESOURCE_EXHAUSTED != coi_err)
        {
            break;
        }
    }

    if (COI_SUCCESS != coi_err)
    {
        MicErrLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), "Memory Object COI buffer Allocation failed, error=%d", coi_err);
        return CL_DEV_OBJECT_ALLOC_FAIL;
    }

    m_coi_top_level_buffer        = m_coi_buffer;
    m_coi_top_level_buffer_offset = 0;
    // Increment ref counter of this buffer.
    incRefCounter();
    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjRelease( )
{
    decRefCounter();
    return CL_DEV_SUCCESS;
}

bool MICDevMemoryObject::getMemObjFromMapBuffersPool(void* ptr, size_t size, MICDevMemoryObject** ppOutMemObj)
{
	return m_buffersMemoryPool.getBuffer(ptr, size, ppOutMemObj);
}

void MICDevMemoryObject::incRefCounter()
{
	m_bufferRefCounter ++;
}

void MICDevMemoryObject::decRefCounter()
{
	m_bufferRefCounter --;
	if (0 < m_bufferRefCounter)
	{
		return;
	}
	COIRESULT coi_err = COI_SUCCESS;

    MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), "%s", "ReleaseObject enter");

    if ((0 != m_coi_buffer) && (!MICDevice::isDeviceLibraryUnloaded()))
    {
        coi_err = COIBufferDestroy( m_coi_buffer );
        assert( COI_SUCCESS == coi_err && "Cannot destroy COI Buffer" );
    }

    if (m_pBackingStore)
    {
	    m_pBackingStore->RemovePendency();
    }

    delete this;
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
    if (MICDevice::isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }
    
    MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), "%s", "CreateMappedRegion enter");

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
    MEMCPY_S(pMapParams->pitch, sizeof(size_t)*(MAX_WORK_DIM-1), m_objDescr.pitch, sizeof(size_t)*(MAX_WORK_DIM-1));

    // set map handle
    pMapParams->map_handle = coi_params;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjReleaseMappedRegion( cl_dev_cmd_param_map* IN pMapParams )
{
    if (MICDevice::isDeviceLibraryUnloaded())
    {
        return CL_DEV_SUCCESS;
    }

    MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), "%s", "ReleaseMappedRegion enter");
    assert( NULL != pMapParams->map_handle && "cl_dev_cmd_param_map was not filled by MIC Device" );

    SMemMapParamsList* coi_params = MemoryAllocator::GetCoiMapParams(pMapParams);
    delete coi_params;
    pMapParams->map_handle = NULL;

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjCreateSubObject( cl_mem_flags mem_flags, const size_t *origin,
                                           const size_t *size, 
                                           IOCLDevRTMemObjectService IN *pBSService,
                                           IOCLDevMemoryObject** ppSubBuffer )
{
    if (MICDevice::isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }

    MICDevMemorySubObject* pSubObject = new MICDevMemorySubObject(m_Allocator, *this);
    if ( NULL == pSubObject )
    {
        return CL_DEV_OUT_OF_MEMORY;
    }

    cl_dev_err_code devErr = pSubObject->Init(mem_flags, origin, size, pBSService);
    if ( CL_DEV_FAILED(devErr) )
    {
        delete pSubObject;
        return devErr;
    }

    assert (NULL != ppSubBuffer);
    *ppSubBuffer = pSubObject;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjUpdateBackingStore( 
                            void* operation_handle, cl_dev_bs_update_state* pUpdateState )
{
    if (MICDevice::isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }

	MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), "%s", "clDevMemObjUpdateBackingStore enter");

    assert( NULL != pUpdateState );

    COIEVENT  completion_event;
    COIRESULT coi_err = COIBufferSetState(  m_coi_buffer, 
                                            COI_PROCESS_SOURCE, COI_BUFFER_VALID, COI_BUFFER_MOVE, 
                                            0, NULL, 
                                            &completion_event );

    assert( (COI_SUCCESS == coi_err) && "COIBufferSetState( SOURCE, VALID, MOVE ) failed" );

    if (COI_SUCCESS != coi_err)
    {
        *pUpdateState = CL_DEV_BS_UPDATE_COMPLETED;
        return CL_DEV_ERROR_FAIL;
    }

    // Here races in reporting to framework from this thread and notification port thread may occur
    // We do not need lock here because we know that state update inside framework is done inside lock and we are 
    // called from inside lock.
    // More than this, taking lock here may result in deadlock if this function is called from inside lock.

    MICDevice* owner = get_owning_device();
    assert( NULL != owner );

    if (NULL == owner)
    {
        *pUpdateState = CL_DEV_BS_UPDATE_COMPLETED;
        return CL_DEV_ERROR_FAIL;
    }

    owner->GetDeviceNotificationPort().addBarrier(completion_event, this, operation_handle);
    *pUpdateState = CL_DEV_BS_UPDATE_LAUNCHED;
    return CL_DEV_SUCCESS;
}

void MICDevMemoryObject::fireCallBack(void* arg)
{
     if (MICDevice::isDeviceLibraryUnloaded())
     {
        return;
     }

     MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), "%s", "BackingStoreUpdateFinished enter");

     m_pRTMemObjService->BackingStoreUpdateFinished(arg, CL_DEV_SUCCESS );
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjUpdateFromBackingStore( 
                            void* operation_handle, cl_dev_bs_update_state* pUpdateState )
{
    bool all_is_ok = true;
    
    if (MICDevice::isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }

    MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), "%s", "clDevMemObjUpdateFromBackingStore enter");

    assert( NULL != pUpdateState );

    COIRESULT coi_err = COIBufferSetState(  m_coi_buffer, 
                                            COI_PROCESS_SOURCE, COI_BUFFER_VALID, COI_BUFFER_NO_MOVE, 
                                            0, NULL, 
                                            NULL);

    assert( (COI_SUCCESS == coi_err) && "COIBufferSetState( SOURCE, VALID, NO_MOVE ) failed" );
    all_is_ok = (COI_SUCCESS == coi_err);

	// Invalidate all SINK instances.
    coi_err = COIBufferSetState(  m_coi_buffer, 
                                    COI_SINK_OWNERS, COI_BUFFER_INVALID, COI_BUFFER_NO_MOVE, 
                                    0, NULL, 
                                    NULL);

    assert( (COI_SUCCESS == coi_err) && "COIBufferSetState( SINK, INVALID, NO_MOVE ) failed" );
    all_is_ok = all_is_ok && (COI_SUCCESS == coi_err);

    *pUpdateState = CL_DEV_BS_UPDATE_COMPLETED;
    return (all_is_ok) ? CL_DEV_SUCCESS : CL_DEV_ERROR_FAIL;
}

cl_dev_err_code MICDevMemoryObject::clDevMemObjInvalidateData( )
{
    bool all_is_ok = true;
    COIRESULT coi_err;

    if (MICDevice::isDeviceLibraryUnloaded())
    {
        return CL_DEV_ERROR_FAIL;
    }

    MicInfoLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), "%s", "clDevMemObjInvalidateData enter");

    // Invalidate all SINK instances.
    coi_err = COIBufferSetState(  m_coi_buffer, 
                                    COI_SINK_OWNERS, COI_BUFFER_INVALID, COI_BUFFER_NO_MOVE, 
                                    0, NULL, 
                                    NULL);

    assert( (COI_SUCCESS == coi_err) && "COIBufferSetState( SINK, VALID_MAYDROP, NO_MOVE ) failed" );
    all_is_ok = all_is_ok && (COI_SUCCESS == coi_err);

    return (all_is_ok) ? CL_DEV_SUCCESS : CL_DEV_ERROR_FAIL;
}

bool MICDevMemoryObject::MapBuffersMemoryPool::getBuffer(void* ptr, size_t size, MICDevMemoryObject** ppOutMemObj)
{
	map<void*, mem_obj_directive>::iterator iter;
	OclAutoReader mutex(&m_multiReadSingleWriteMutex);

    if (0 == m_addressToMemObj.size())
	{
		return false;
	}

	iter = m_addressToMemObj.lower_bound(ptr);
	if (((m_addressToMemObj.end() == iter) && (m_addressToMemObj.size() > 0)) || (((size_t)(iter->first) > (size_t)ptr) && (m_addressToMemObj.begin() != iter)))
	{
		iter --;
	}
	if ((m_addressToMemObj.end() == iter) || (false == iter->second.isReady) || ((size_t)ptr < (size_t)(iter->first)) || ((size_t)ptr + size > (size_t)(iter->first) + iter->second.pMemObj->GetRawDataSize()))
	{
		return false;
	}
	*ppOutMemObj = iter->second.pMemObj;
	(*ppOutMemObj)->incRefCounter();
	return true;
}

void MICDevMemoryObject::MapBuffersMemoryPool::addBufferToPool(MICDevMemoryObject* pMicMemObj)
{
	OclAutoWriter mutex(&m_multiReadSingleWriteMutex);
	map<void*, mem_obj_directive>::iterator iter = m_addressToMemObj.find(pMicMemObj->clDevMemObjGetDescriptorRaw().pData);
	if (m_addressToMemObj.end() == iter)
	{
		m_addressToMemObj.insert( pair<void*, mem_obj_directive>( pMicMemObj->clDevMemObjGetDescriptorRaw().pData, mem_obj_directive(pMicMemObj) ) );
	}
	else
	{
		iter->second.refCounter ++;
	}
	pMicMemObj->incRefCounter();
}

void MICDevMemoryObject::MapBuffersMemoryPool::removeBufferFromPool(MICDevMemoryObject* pMicMemObj)
{
	OclAutoWriter mutex(&m_multiReadSingleWriteMutex);
	assert(m_addressToMemObj.end() != m_addressToMemObj.find(pMicMemObj->clDevMemObjGetDescriptorRaw().pData));
	map<void*, mem_obj_directive>::iterator iter = m_addressToMemObj.find(pMicMemObj->clDevMemObjGetDescriptorRaw().pData);
	if (iter->second.refCounter == 1)
	{
		m_addressToMemObj.erase(iter);
	}
	else
	{
		iter->second.refCounter --;
	}
	pMicMemObj->decRefCounter();
}

void MICDevMemoryObject::MapBuffersMemoryPool::setBufferReady(MICDevMemoryObject* pMicMemObj, CommandList* cur_queue)
{
	OclAutoWriter mutex(&m_multiReadSingleWriteMutex);
	map<void*, mem_obj_directive>::iterator iter = m_addressToMemObj.find(pMicMemObj->clDevMemObjGetDescriptorRaw().pData);
    if (iter != m_addressToMemObj.end())
    {
	    iter->second.isReady = true;
    }
    else
    {        
        assert( cur_queue->isCanceled() );
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
MICDevMemorySubObject::MICDevMemorySubObject(MemoryAllocator& allocator, MICDevMemoryObject& pParent) :
    MICDevMemoryObject(allocator), m_Parent(pParent)

{
}

cl_dev_err_code MICDevMemorySubObject::Init(cl_mem_flags mem_flags, const size_t *origin, const size_t *size,
                                            IOCLDevRTMemObjectService IN *pBSService )
{
    MEMCPY_S(&m_objDescr, sizeof(cl_mem_obj_descriptor), &m_Parent.m_objDescr, sizeof(cl_mem_obj_descriptor));

    m_nodeId           = m_Parent.m_nodeId;

    m_pRTMemObjService = pBSService;
    assert( NULL != m_pRTMemObjService);

    // Get backing store.
    cl_dev_err_code bsErr = m_pRTMemObjService->GetBackingStore(CL_DEV_BS_GET_ALWAYS, &m_pBackingStore);
    assert( CL_DEV_SUCCEEDED(bsErr) && (NULL != m_pBackingStore) && "Runtime did not allocated Backing Store object!");

    if (!CL_DEV_SUCCEEDED(bsErr) || (NULL == m_pBackingStore))
    {
        // error - nothing can be done
        m_pBackingStore = NULL;
        return CL_DEV_INVALID_MEM_OBJECT;
    }

    m_pBackingStore->AddPendency();

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

    m_memFlags       = mem_flags;
    m_objDescr.pData = m_pBackingStore->GetRawData();

    m_coi_top_level_buffer        = m_Parent.m_coi_top_level_buffer;
    m_coi_top_level_buffer_offset = m_Parent.m_coi_top_level_buffer_offset + m_Parent.m_pBackingStore->GetRawDataOffset(origin);

    assert( (0 != m_raw_size) && "Zero subbuffer size" );

    COIRESULT coi_err = COIBufferCreateSubBuffer( m_coi_top_level_buffer, 
                                                  m_raw_size, 
                                                  m_coi_top_level_buffer_offset, 
                                                  &m_coi_buffer );

    assert( ((COI_SUCCESS == coi_err) || (COI_OUT_OF_MEMORY == coi_err)) && "COIBufferCreateSubBuffer() failed"); 

    if (COI_SUCCESS != coi_err)
    {
        MicErrLog(m_Allocator.GetLogDescriptor(), m_Allocator.GetLogHandle(), "Memory Object COI sub-buffer Allocation failed, error=%d", coi_err);
        return CL_DEV_OBJECT_ALLOC_FAIL;
    }

    m_buffActiveProcesses = m_Parent.m_buffActiveProcesses;
    // Increment ref counter of this buffer.
    incRefCounter();

    return CL_DEV_SUCCESS;
}

