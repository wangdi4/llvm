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
//  Implementation of the MemoryObject Class
//  Created on:      22-Aug-2011
//  Original author: kdmitry
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "GenericMemObj.h"

#include <Device.h>
#include <assert.h>
#include "cl_sys_defines.h"

#include "MemoryObjectFactory.h"

// assumes alignment is a power of 2
#define IS_ALIGNED_ON( what, alignment ) (0 == (((size_t)(what) & ((size_t)(alignment) - 1))))

using namespace std;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
GenericMemObject::GenericMemObject(Context * pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType) :
	MemoryObject(pContext, pOclEntryPoints),
    m_BS(NULL), m_active_groups_count(0)
{
	INIT_LOGGER_CLIENT(TEXT("GenericMemObject"), LL_DEBUG);

    m_clMemObjectType = clObjType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
GenericMemObject::~GenericMemObject()
{
	LOG_DEBUG(TEXT("%S"), TEXT("Enter GenericMemObject D'tor"));

	NotifyDestruction();
    remove_device_objects();

	if ( NULL != m_pBackingStore )
	{
		m_pBackingStore->RemovePendency();
	}

	if ( NULL != m_BS )
	{
		m_BS->RemovePendency();
	}

	RELEASE_LOGGER_CLIENT;
}

void GenericMemObject::remove_device_objects(void)
{
    if (m_active_groups_count > 0)
    {
        for (unsigned int i = 0; i < MAX_DEVICE_SHARING_GROUP_ID; ++i)
        {
            SharingGroup& group = m_sharing_groups[i];

            if (NULL != group.m_dev_mem_obj)
            {
                group.m_dev_mem_obj->clDevMemObjRelease();
                group.m_dev_mem_obj = NULL;
            }
        }

        m_active_groups_count = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::Initialize
///////////////////////////////////////////////////////////////////////////////////////////////////

static inline bool is_buffer_ok_for_device(  size_t buffer_size,
                                             const cl_dev_alloc_prop& properies )
{
    bool ok = true;

    ok &= (buffer_size <= properies.maxBufferSize);
    return ok;
}

static inline bool is_image_ok_for_device( unsigned int		dim_count,
            							   const size_t*	dimension,
            							   const size_t*    pitches,
                                           const cl_dev_alloc_prop& properies )
{
    bool ok = true;

    ok &= properies.imagesSupported;

    // TODO: DK: Need to check for image format device limitations.
    return ok;
}

// initialize the memory object
cl_err_code GenericMemObject::Initialize(
							   cl_mem_flags		clMemFlags,
							   const cl_image_format*	pclImageFormat,
							   unsigned int		dim_count,
							   const size_t*	dimension,
							   const size_t*    pitches,
							   void*			pHostPtr
							   )
{
	m_clFlags = clMemFlags;
	m_pHostPtr = pHostPtr;

	// assign default value
	if ( !(m_clFlags & (CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY | CL_MEM_READ_WRITE)) )
	{
		m_clFlags |= CL_MEM_READ_WRITE;
	}

    m_uiNumDim      = dim_count;
    if ((dim_count < 1) || (dim_count > MAX_WORK_DIM))
    {
        // error
        return CL_INVALID_VALUE;
    }

    if (NULL == dimension)
    {
        return CL_INVALID_VALUE;
    }

    // caclulate alignment requirements and filter out non-conformant devices
    cl_uint             dev_count = 0;
    FissionableDevice* *pDevices  = m_pContext->GetDevices(&dev_count);

    assert( (0 != dev_count) && (NULL != pDevices) );

    size_t alignment = 1; // no alignment
    size_t object_size = GenericMemObjectBackingStore::calculate_size(
                            GenericMemObjectBackingStore::get_element_size( pclImageFormat ),
                            dim_count, dimension, pitches );

    cl_err_code no_devices_error_code = CL_SUCCESS;

    for ( cl_uint dev_idx = 0; dev_idx < dev_count; ++dev_idx)
    {
        FissionableDevice* dev = pDevices[ dev_idx ];
        assert( dev );

        cl_dev_alloc_prop device_properties;
        cl_dev_err_code   err = dev->GetDeviceAgent()->clDevGetMemoryAllocProperties(
                                        m_clMemObjectType,
                                        &device_properties );
        assert( CL_DEV_SUCCEEDED( err ) );
        assert( (unsigned int)device_properties.imageSharingGroupId  < MAX_DEVICE_SHARING_GROUP_ID );
        assert( (unsigned int)device_properties.bufferSharingGroupId < MAX_DEVICE_SHARING_GROUP_ID );
        assert( 0 == (device_properties.alignment & (device_properties.alignment - 1))
                        && "Device Mem alignment requirement is not a power of 2");

        unsigned int sharingGroupId;

        // filter out devices that do not support this memory object
        if (CL_MEM_OBJECT_BUFFER == m_clMemObjectType)
        {
            sharingGroupId = (unsigned int)device_properties.bufferSharingGroupId;

            if (! is_buffer_ok_for_device(object_size, device_properties))
            {
                no_devices_error_code = CL_DEVICE_MAX_MEM_ALLOC_SIZE;
                continue;
            }
        }
        else
        {
            // assume image
            sharingGroupId = (unsigned int)device_properties.imageSharingGroupId;

            if (! is_image_ok_for_device(dim_count, dimension, pitches, device_properties))
            {
                no_devices_error_code = CL_INVALID_OPERATION;
                continue;
            }
        }

        alignment = MAX( alignment, device_properties.alignment );

        // add device to the list
        m_device_descriptors.push_back( DeviceDescriptor(dev, sharingGroupId,
                                              device_properties.alignment) );
        DeviceDescriptor& last_added = m_device_descriptors.back();

        m_sharing_groups[ sharingGroupId ].m_device_list.push_back( &last_added );

        // add device agent to the list
        m_device_agents.push_back( dev->GetDeviceAgent() );

    }

    if (m_device_descriptors.empty())
    {
        // all devices were filtered out
        return no_devices_error_code;
    }

    // Create Backing Store
    m_BS = new GenericMemObjectBackingStore(   clMemFlags,
                							   pclImageFormat,
                							   dim_count,
                							   dimension,
                							   pitches,
                							   pHostPtr,
                							   alignment );

    if (NULL == m_BS)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    // just start with both pointers equal
    m_BS->AddPendency();
    m_pBackingStore = m_BS;

    // buffer sizes are ok - allocate on host
    if (! m_BS->AllocateData())
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    // now allocate on devices. Only one single allocation per sharing group is required.
    for (unsigned int i = 0; i < MAX_DEVICE_SHARING_GROUP_ID; ++i)
    {
        SharingGroup& group = m_sharing_groups[i];

        if (group.m_device_list.empty())
        {
            continue;
        }

        cl_err_code dev_err = allocate_object_for_sharing_group( i );

        if ( CL_FAILED(dev_err) )
        {
            remove_device_objects();
        	return CL_OUT_OF_RESOURCES;
        }
    }

    // TODO: DK: this is WRONG!!! need to support multiple owners
    // override GetLocation to get any acceptable and add check_copy_required to
    // check if copy to the given device is required
	m_pLocation = pDevices[0];

	// Now we should set backing store
	// Get access to internal pointer
	m_pMemObjData = m_BS->GetRawData();

	m_stMemObjSize  = m_BS->GetRawDataSize();

	return CL_SUCCESS;
}

cl_err_code GenericMemObject::InitializeSubObject(
                                            cl_mem_flags      clMemFlags,
                                            GenericMemObject& parent,
                                            const size_t*     origin,
                                            const size_t*     region )
{
    // sub-buffer related - used by internal functions call later in this function
	m_pParentObject = &parent;
   	m_pParentObject->AddPendency(this);

    memcpy( m_stOrigin, origin, sizeof(m_stOrigin) );

    // copy everything from GenericMemObject class only excluding parent classes

	m_clFlags   = clMemFlags;

    // Create Backing Store
    assert( (NULL != parent.m_pBackingStore->GetRawData()) && parent.m_pBackingStore->IsDataValid() &&
            "Parent Memory Object Backing Store must be finalized during SubObject creation" );

    m_BS = new GenericMemObjectBackingStore( clMemFlags,
                							 parent.m_pBackingStore,
                							 origin,
                							 region,
                							 *(parent.m_BS));

    if (NULL == m_BS)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    // just start with both pointers equal
    m_BS->AddPendency();
    m_pBackingStore = m_BS;

	m_pHostPtr = m_BS->GetUserProvidedHostMapPtr();
    m_uiNumDim = m_BS->GetDimCount();

    size_t start_offset = parent.m_BS->GetRawDataOffset(origin);

    for (unsigned int group_idx = 0; group_idx < MAX_DEVICE_SHARING_GROUP_ID; ++group_idx)
    {
        const SharingGroup& p_grp = parent.m_sharing_groups[group_idx];

        if (p_grp.m_device_list.empty())
        {
            continue;
        }

        TDeviceDescPtrList::const_iterator it     = p_grp.m_device_list.begin();
        TDeviceDescPtrList::const_iterator it_end = p_grp.m_device_list.end();

        for(; it != it_end; ++it)
        {
            const DeviceDescriptor* p_dev_desc = *it;

            // filter out devices that do not support this memory object
            if (CL_MEM_OBJECT_BUFFER == m_clMemObjectType)
            {
                if (! IS_ALIGNED_ON( start_offset, p_dev_desc->m_alignment))
                {
                    continue;
                }
            }

            m_device_descriptors.push_back( DeviceDescriptor( p_dev_desc->m_pDevice,
                                                   group_idx,
                                                   p_dev_desc->m_alignment) );
            DeviceDescriptor& last_added = m_device_descriptors.back();

            m_sharing_groups[group_idx].m_device_list.push_back( &last_added );

            m_device_agents.push_back( p_dev_desc->m_pDevice->GetDeviceAgent() );
        }

        cl_err_code dev_err = allocate_object_for_sharing_group( group_idx );

        if ( CL_FAILED(dev_err) )
        {
            remove_device_objects();
        	return CL_OUT_OF_RESOURCES;
        }
    }

    if (m_device_descriptors.empty())
    {
        // all devices were filtered out
        return CL_MISALIGNED_SUB_BUFFER_OFFSET;
    }

    // TODO: DK: this is WRONG!!! need to support multiple owners
    // override GetLocation to get any acceptable and add check_copy_required to
    // check if copy to the given device is required
	m_pLocation = parent.m_pLocation;

	// Now we should set backing store
	// Get access to internal pointer
	m_pMemObjData = m_BS->GetRawData();

    // sub-buffer related
    m_stMemObjSize  = m_BS->GetRawDataSize();

    return CL_SUCCESS;
}



const GenericMemObject::DeviceDescriptor* GenericMemObject::get_device( FissionableDevice* dev ) const
{
    TDeviceDescList::const_iterator it     = m_device_descriptors.begin();
    TDeviceDescList::const_iterator it_end = m_device_descriptors.end();

    for(;it != it_end; ++it)
    {
        const DeviceDescriptor& desc = *it;
        if (desc.m_pDevice == dev )
        {
            return &desc;
        }
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject::SetDataLocation
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code GenericMemObject::UpdateLocation(FissionableDevice* pDevice)
{
    // TODO: DK: Add parametetr to specify W/R access
	LOG_DEBUG(TEXT("Enter SetDataLocation (clDevice=%x)"), pDevice);

	m_pLocation = pDevice;
	return CL_SUCCESS;
}

bool GenericMemObject::IsSharedWith(FissionableDevice* pDevice)
{
    // TODO: DK: Need 2 devices or another meaning
	return (pDevice == m_pLocation);
}

cl_err_code GenericMemObject::allocate_object_for_sharing_group( unsigned int group_id )
{
    assert( group_id < MAX_DEVICE_SHARING_GROUP_ID );
    SharingGroup& group      = m_sharing_groups[group_id];
    IOCLDevMemoryObject* obj = device_object(group);

    if (NULL != obj)
    {
        return CL_SUCCESS;
    }

    if (group.m_device_list.empty())
    {
        return CL_INVALID_VALUE;
    }

    FissionableDevice* dev = group.m_device_list.front()->m_pDevice;

    // Pass only R/W values
	cl_mem_flags clMemFlags = m_clFlags & (CL_MEM_WRITE_ONLY | CL_MEM_READ_ONLY);

    cl_err_code err = create_device_object( clMemFlags, dev, m_BS, &obj );

    if ( CL_FAILED(err) )
    {
    	return err;
    }

    ++m_active_groups_count;
    group.m_dev_mem_obj = obj;

    return CL_SUCCESS;
}

cl_err_code GenericMemObject::create_device_object( cl_mem_flags clMemFlags,
                                                    FissionableDevice* dev,
                                                    GenericMemObjectBackingStore* bs,
                                                    IOCLDevMemoryObject** dev_object )
{
    cl_dev_err_code devErr = dev->GetDeviceAgent()->clDevCreateMemoryObject(
                                    dev->GetSubdeviceId(),
    	                            clMemFlags,
    	                            &(m_BS->GetFormat()),
    	                            m_BS->GetDimCount(),
    	                            m_BS->GetDimentions(),
    	                            this,
    	                            dev_object);

    return CL_DEV_FAILED(devErr) ? CL_OUT_OF_RESOURCES : CL_SUCCESS;
}

cl_err_code GenericMemObject::CreateDeviceResource(FissionableDevice* pDevice)
{
    DeviceDescriptor* desc = get_device( pDevice );

    if (NULL == desc)
    {
        return CL_INVALID_DEVICE;
    }

    // Assumption: All devices are added at class initialization time so data may be modified at runtime only
    //             because of lazy sharing groups initialization.
    //             m_global_lock may be taken only during sharing group lazy initialization.

    OclAutoMutex lock( &m_global_lock );

    return allocate_object_for_sharing_group(desc->m_sharing_group_id);
}

cl_err_code GenericMemObject::GetDeviceDescriptor(FissionableDevice* pDevice, IOCLDevMemoryObject* *ppDevObject, OclEvent** ppEvent)
{
	assert(NULL != ppDevObject);

    DeviceDescriptor* desc = get_device( pDevice );

    if (NULL == desc)
    {
        return CL_INVALID_DEVICE;
    }

    *ppDevObject = device_object( *desc );
    assert(NULL != *ppDevObject);

    return CL_SUCCESS;
}

cl_err_code	GenericMemObject::MemObjCreateDevMappedRegion(
                                        const FissionableDevice* pDevice,
										cl_dev_cmd_param_map*	cmd_param_map,
										void** pHostMapDataPtr )
{
    IOCLDevMemoryObject* dev_object;
    cl_err_code          err;

    err = GetDeviceDescriptor( const_cast<FissionableDevice*>(pDevice), &dev_object, NULL );

    if (CL_FAILED(err))
    {
        return err;
    }

	cl_dev_err_code dev_err =  dev_object->clDevMemObjCreateMappedRegion(cmd_param_map);

    if (CL_DEV_SUCCEEDED(dev_err))
    {
        *pHostMapDataPtr = cmd_param_map->ptr;
        void* user_provided_ptr = m_BS->GetUserProvidedHostMapPtr();
        if (NULL != user_provided_ptr)
        {
            // if user provided Host Map pointer all mappings should be returned in this area.
            *pHostMapDataPtr = (cl_uchar*)user_provided_ptr + m_BS->GetRawDataOffset( cmd_param_map->origin );
        }
        return CL_SUCCESS;
    }

	return CL_OUT_OF_RESOURCES;
}

cl_err_code	GenericMemObject::MemObjReleaseDevMappedRegion(
                                            const FissionableDevice* pDevice,
											cl_dev_cmd_param_map*	cmd_param_map,
											void* pHostMapDataPtr )
{
    IOCLDevMemoryObject* dev_object;
    cl_err_code          err;

    err = GetDeviceDescriptor( const_cast<FissionableDevice*>(pDevice), &dev_object, NULL );

    if (CL_FAILED(err))
    {
        return err;
    }

	cl_dev_err_code dev_err =  dev_object->clDevMemObjReleaseMappedRegion(cmd_param_map);

	return CL_DEV_SUCCEEDED(dev_err) ? CL_SUCCESS : CL_INVALID_VALUE;
}

cl_err_code GenericMemObject::SynchDataToHost(   cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr )
{
    if (pMapInfo->ptr == pHostMapDataPtr)
    {
        // nothing to do
        return CL_SUCCESS;
    }

    return ReadData( pHostMapDataPtr,
                     pMapInfo->origin, pMapInfo->region,
                     pMapInfo->pitch[0], pMapInfo->pitch[1] );
}

cl_err_code GenericMemObject::SynchDataFromHost( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr )
{
    if (pMapInfo->ptr == pHostMapDataPtr)
    {
        // nothing to do
        return CL_SUCCESS;
    }

    return WriteData( pHostMapDataPtr,
                      pMapInfo->origin, pMapInfo->region,
                      pMapInfo->pitch[0], pMapInfo->pitch[1] );
}

cl_err_code GenericMemObject::NotifyDeviceFissioned(FissionableDevice* parent, size_t count, FissionableDevice** children)
{
    // TODO: DK: What should I do here?
    return CL_SUCCESS;
}

// IOCLDevRTMemObjectService Methods
cl_dev_err_code GenericMemObject::GetBackingStore(cl_dev_bs_flags flags, IOCLDevBackingStore* *ppBS)
{
	assert(NULL!= ppBS);
	assert(NULL!= m_pBackingStore);

	*ppBS = m_pBackingStore;
	return CL_DEV_SUCCESS;
}

cl_dev_err_code GenericMemObject::SetBackingStore(IOCLDevBackingStore* pBS)
{
	pBS->AddPendency();
	IOCLDevBackingStore* pOldBS = m_pBackingStore.exchange(pBS);
	if ( NULL != pOldBS )
	{
		pOldBS->RemovePendency();
	}

	return CL_DEV_SUCCESS;
}

size_t GenericMemObject::GetDeviceAgentListSize() const
{
	return m_device_agents.size();
}

const IOCLDeviceAgent* const *GenericMemObject::GetDeviceAgentList() const
{
	return &(m_device_agents[0]);
}

cl_err_code GenericMemObject::ReadData(void * pData,
                                       const size_t * pszOrigin, const size_t * pszRegion,
                                       size_t szRowPitch, size_t szSlicePitch)
{
	LOG_DEBUG(L"Enter ReadData (szRowPitch=%d, pData=%d, szSlicePitch=%d)", szRowPitch, pData, szSlicePitch);

	SMemCpyParams			sCpyParam;

	// Region
	sCpyParam.uiDimCount = m_BS->GetDimCount();

    // set source pointer (we)
    sCpyParam.pSrc = (cl_char*)m_pBackingStore->GetRawData() + m_BS->GetRawDataOffset( pszOrigin );

    // set destination (them)
    sCpyParam.pDst = (cl_char*)pData;

    // optimize for the case of single dimension
    if (1 == sCpyParam.uiDimCount)
    {
        memcpy( sCpyParam.pDst, sCpyParam.pSrc, pszRegion[0] * m_BS->GetElementSize() );
        return CL_SUCCESS;
    }

    // assumption: origin and region both have MAX_WORK_DIM members
	memcpy(sCpyParam.vRegion, pszRegion, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] = pszRegion[0] * m_BS->GetElementSize();

	// set row Pitch for src (we) and dst (them)
	memcpy(sCpyParam.vSrcPitch, m_BS->GetPitch(), sizeof(sCpyParam.vSrcPitch));
    sCpyParam.vDstPitch[0] = szRowPitch;
    sCpyParam.vDstPitch[0] = szSlicePitch;

	clCopyMemoryRegion(&sCpyParam);

	return CL_SUCCESS;
}

cl_err_code GenericMemObject::WriteData(const void * pData,
                                        const size_t * pszOrigin, const size_t * pszRegion,
                                        size_t szRowPitch, size_t szSlicePitch)
{
	LOG_DEBUG(L"Enter WriteData (szRowPitch=%d, pData=%d, szSlicePitch=%d)", szRowPitch, pData, szSlicePitch);

	SMemCpyParams			sCpyParam;

	// Region
	sCpyParam.uiDimCount = m_BS->GetDimCount();

    // set source pointer (them)
    sCpyParam.pSrc = (cl_char*)pData;

    // set destination (we)
    sCpyParam.pDst = (cl_char*)m_pBackingStore->GetRawData() + m_BS->GetRawDataOffset( pszOrigin );

    // optimize for the case of single dimension
    if (1 == sCpyParam.uiDimCount)
    {
        memcpy( sCpyParam.pDst, sCpyParam.pSrc, pszRegion[0] * m_BS->GetElementSize() );
        return CL_SUCCESS;
    }

    // assumption: origin and region both have MAX_WORK_DIM members
	memcpy(sCpyParam.vRegion, pszRegion, sizeof(sCpyParam.vRegion));
	sCpyParam.vRegion[0] = pszRegion[0] * m_BS->GetElementSize();

	// set row Pitch for src (them) and dst (we)
    sCpyParam.vSrcPitch[0] = szRowPitch;
    sCpyParam.vSrcPitch[0] = szSlicePitch;
	memcpy(sCpyParam.vDstPitch, m_BS->GetPitch(), sizeof(sCpyParam.vDstPitch));

	clCopyMemoryRegion(&sCpyParam);

	return CL_SUCCESS;
}

cl_err_code GenericMemObject::CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const
{
    size_t past_last_end_offset[MAX_WORK_DIM] = {0};
    unsigned int dim_count = (unsigned int)m_BS->GetDimCount();

    for (unsigned int i = 0;  i < dim_count; ++i)
    {
        past_last_end_offset[i] = pszOrigin[i] + pszRegion[i];
    }

	return (m_BS->GetRawDataOffset( past_last_end_offset ) <= m_BS->GetRawDataSize()) ? CL_SUCCESS : CL_INVALID_MEM_OBJECT;
}

cl_err_code GenericMemObject::CheckBoundsRect( const size_t* pszOrigin, const size_t* pszRegion,
                                               size_t szRowPitch, size_t szSlicePitch) const
{
    size_t past_last_end_offset[MAX_WORK_DIM] = {0};
    size_t pitches[2] = { szRowPitch, szSlicePitch };
    unsigned int dim_count = (unsigned int)m_BS->GetDimCount();

    for (unsigned int i = 0; i < dim_count; ++i)
    {
        past_last_end_offset[i] = pszOrigin[i] + pszRegion[i];
    }

    size_t raw_past_end_offset = m_BS->calculate_offset( m_BS->GetElementSize(),
                                                         dim_count,
                                                         past_last_end_offset,
                                                         pitches );

    return (raw_past_end_offset <= m_BS->GetRawDataSize()) ? CL_SUCCESS : CL_INVALID_MEM_OBJECT;
}

void * GenericMemObject::GetBackingStoreData( const size_t * pszOrigin ) const
{
    return (char*)m_pBackingStore->GetRawData() +
            (pszOrigin ? m_BS->GetRawDataOffset( pszOrigin ) : 0);
}

cl_err_code GenericMemObject::CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type, const void * buffer_create_info, MemoryObject** ppBuffer)
{
	const cl_buffer_region* region = static_cast<const cl_buffer_region*>(buffer_create_info);

    if (m_clMemObjectType != CL_MEM_OBJECT_BUFFER)
    {
        return CL_INVALID_OPERATION;
    }

    // alignment must be power of 2
	if ( 0 != (region->origin & (m_BS->GetRequiredAlignment() - 1)) )
	{
		return CL_MISALIGNED_SUB_BUFFER_OFFSET;
	}

	if ( 0 == clFlags )
	{
		clFlags = m_clFlags;
	}

	GenericMemObjectSubBuffer* pSubBuffer = new GenericMemObjectSubBuffer( m_pContext, (ocl_entry_points*)m_handle.dispatch, m_clMemObjectType );
	if ( NULL == pSubBuffer )
	{
		return CL_OUT_OF_HOST_MEMORY;
	}

	cl_err_code err = pSubBuffer->InitializeSubObject(clFlags, *this, &(region->origin),  &(region->size));
	if ( CL_FAILED(err) )
	{
		pSubBuffer->Release();
		LOG_ERROR(TEXT("SubBuffer creation error: %x"), err);
		return err;
	}

	assert(NULL != ppBuffer);
	*ppBuffer = pSubBuffer;
	return CL_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
/// GenericMemObjectBackingStore
//////////////////////////////////////////////////////////////////////////
GenericMemObjectBackingStore::GenericMemObjectBackingStore(
                               cl_mem_flags		        clMemFlags,
							   const cl_image_format*	pclImageFormat,
							   unsigned int		        dim_count,
							   const size_t*	        dimensions,
							   const size_t*            pitches,
							   void*			        pHostPtr,
							   size_t                   alignment ) :
	m_ptr(NULL), m_dim_count(dim_count), m_pHostPtr(pHostPtr), m_creation_flags(clMemFlags),
    m_data_valid(NULL != pHostPtr), m_alignment(alignment), m_parent(NULL), m_refCount(1)
{
    if (pclImageFormat)
    {
        m_format       = *pclImageFormat;
    }
    else
    {
        memset( &m_format, 0, sizeof( m_format ) );
    }

    m_element_size = get_element_size( pclImageFormat );


    // caclulate pitches and dimensions
    assert( NULL != dimensions );

    memset( m_dimensions, 0, sizeof(m_dimensions) );
    memset( m_pitches, 0, sizeof(m_pitches) );

    m_dimensions[0]     = dimensions[0];
    size_t prev_pitch   = m_dimensions[0] * m_element_size;

    for ( cl_uint i = 1; i < dim_count; ++i)
    {
        m_dimensions[i]     = dimensions[i];
        size_t next_pitch   = dimensions[i] * prev_pitch;

        if ((NULL != pitches) && (pitches[i-1] > next_pitch))
        {
            next_pitch = pitches[i-1];
        }

        m_pitches[i-1] = next_pitch;
        prev_pitch     = next_pitch;
    }

    // calc raw data size
    m_raw_data_size = calculate_size(m_element_size, m_dim_count, m_dimensions, m_pitches);

    assert( IS_ALIGNED_ON(m_alignment, m_alignment) && "Alignment is not power of 2" );

    // can user data be used?
    if ((NULL != pHostPtr)                              && // user provided some pointer
        (0 == (m_creation_flags & CL_MEM_COPY_HOST_PTR))&& // this pointer is not for copy from only
        (IS_ALIGNED_ON( pHostPtr, m_alignment )))          // this pointer alignment is ok
    {
        m_ptr = pHostPtr;
    }
}

GenericMemObjectBackingStore::GenericMemObjectBackingStore(
                               cl_mem_flags             clMemFlags,
                               IOCLDevBackingStore*     parent_ps, // used for raw data only
                               const size_t*	        origin,
                               const size_t*            region,
                               GenericMemObjectBackingStore&  copy_setting_from ):
	m_ptr(NULL), m_dim_count(0), m_pHostPtr(NULL), m_creation_flags(clMemFlags),
    m_data_valid(true), m_alignment(0), m_parent(NULL), m_refCount(1)
{
    size_t raw_data_offset = copy_setting_from.GetRawDataOffset(origin);

    m_ptr = (cl_uchar*)parent_ps->GetRawData() + raw_data_offset;

    m_dim_count = copy_setting_from.m_dim_count;

    memcpy( m_dimensions, region, sizeof(m_dimensions) );

    memcpy( m_pitches, copy_setting_from.m_pitches, sizeof(m_pitches) );

    m_format       = copy_setting_from.m_format;
    m_element_size = copy_setting_from.m_element_size;
    m_pHostPtr     = (m_pHostPtr) ? (cl_uchar*)m_pHostPtr + raw_data_offset : NULL;
    m_alignment    = copy_setting_from.m_alignment;

    // calc raw data size
    m_raw_data_size = calculate_size(m_element_size, m_dim_count, m_dimensions, m_pitches);

    m_parent->AddPendency();
}



GenericMemObjectBackingStore::~GenericMemObjectBackingStore()
{
    if (m_parent)
    {
        // subobject
        m_parent->RemovePendency();
    }
    else if (m_ptr && (m_ptr != m_pHostPtr))
	{
        ALIGNED_FREE( m_ptr );
	}
}

int GenericMemObjectBackingStore::AddPendency()
{
	return m_refCount++;
}

int GenericMemObjectBackingStore::RemovePendency()
{
	long prevVal = m_refCount--;
	if ( 1 == prevVal )
	{
		delete this;
	}
	return prevVal;
}

size_t GenericMemObjectBackingStore::get_element_size(const cl_image_format* format)
{
    if (NULL == format)
    {
        return 1;
    }

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

size_t GenericMemObjectBackingStore::calculate_offset( size_t elem_size, unsigned int  dim_count,
                                                       const size_t  origin[], const size_t  pitches[] )
{
    size_t offset = elem_size * origin[0];
    for (size_t i = 1; i < dim_count; ++i)
    {
        offset += pitches[i-1]*origin[i];
    }
    return offset;
}

size_t GenericMemObjectBackingStore::calculate_size( size_t elem_size, unsigned int  dim_count,
                                                     const size_t  dimensions[], const size_t  pitches[] )
{
    return (1 == dim_count) ?
                        dimensions[0] * elem_size :
                        pitches[dim_count-2] * dimensions[dim_count-1];
}

size_t GenericMemObjectBackingStore::GetRawDataOffset( const size_t* origin ) const
{
    return calculate_offset( m_element_size, m_dim_count, origin, m_pitches );
}

bool GenericMemObjectBackingStore::AllocateData( void )
{
    if (NULL != m_ptr)
    {
        return true;
    }

    // to get the best possible performance from external devices (DMA transfers) prefer 4K page alignment if possible
    size_t alignment = MAX( m_alignment, PAGE_4K_SIZE );

    m_ptr = ALIGNED_MALLOC( m_raw_data_size, alignment );

    if (m_ptr && IsCopyRequired())
    {
        memcpy( m_ptr, m_pHostPtr, m_raw_data_size );

        if (m_creation_flags & CL_MEM_COPY_HOST_PTR)
        {
            // user provided the host pointer only for one-time init and not for mapping
            m_pHostPtr = NULL;
        }
    }

    return NULL != m_ptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedSubBuffer C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////

GenericMemObjectSubBuffer::GenericMemObjectSubBuffer(Context * pContext, ocl_entry_points * pOclEntryPoints, cl_mem_object_type clObjType)
	: GenericMemObject(pContext, pOclEntryPoints, clObjType)
{
}

GenericMemObjectSubBuffer::~GenericMemObjectSubBuffer()
{
	if ( NULL != m_pParentObject )
	{
		m_pParentObject->RemovePendency(this);
	}
}

cl_err_code GenericMemObjectSubBuffer::Initialize(
			cl_mem_flags		clMemFlags,
			const cl_image_format*	pclImageFormat,
			unsigned int		dim_count,
			const size_t*		dimension,  // size == region
			const size_t*       pitches,    // origin
			void*				pHostPtr
			)
{
	assert(0);
	return CL_INVALID_OPERATION;
}

cl_err_code GenericMemObjectSubBuffer::create_device_object(
                                                    cl_mem_flags clMemFlags,
                                                    FissionableDevice* dev,
                                                    GenericMemObjectBackingStore* bs,
                                                    IOCLDevMemoryObject** dev_object )
{
    // get parent device object for this device
    IOCLDevMemoryObject* parent_MemObject = NULL;

    cl_err_code err = m_pParentObject->CreateDeviceResource( dev );
    if (CL_FAILED(err))
    {
        return CL_OUT_OF_RESOURCES;
    }

    err = m_pParentObject->GetDeviceDescriptor( dev, &parent_MemObject, NULL );

    if (CL_FAILED(err))
    {
        return CL_OUT_OF_RESOURCES;
    }

    cl_dev_err_code devErr = parent_MemObject->clDevMemObjCreateSubObject(
    	                            clMemFlags,
    	                            m_stOrigin,
    	                            bs->GetDimentions(),
    	                            dev_object);

    return CL_DEV_FAILED(devErr) ? CL_OUT_OF_RESOURCES : CL_SUCCESS;
}


bool GenericMemObjectSubBuffer::IsSupportedByDevice(FissionableDevice* pDevice)
{
	// Need to check only for sub-buffers

    cl_dev_alloc_prop device_properties;
    cl_dev_err_code   err = pDevice->GetDeviceAgent()->clDevGetMemoryAllocProperties(
                                    m_clMemObjectType,
                                    &device_properties );

    assert( CL_DEV_SUCCEEDED( err ) );

	return IS_ALIGNED_ON(m_stOrigin[0], device_properties.alignment );
}

