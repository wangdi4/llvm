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

#include "GenericMemObj.h"

#include <Device.h>
#include <algorithm>
#include <assert.h>
#include "cl_sys_defines.h"

#include "MemoryObjectFactory.h"
#include "cl_shared_ptr.hpp"

using namespace std;
using namespace Intel::OpenCL::Framework;

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
GenericMemObject::GenericMemObject(const SharedPtr<Context>& pContext, cl_mem_object_type clObjType) :    
    MemoryObject(pContext),
    m_active_groups_count(0),
    m_BS(nullptr),
    m_hierarchicalMemoryMode(MEMORY_MODE_NORMAL)    
{
    INIT_LOGGER_CLIENT(TEXT("GenericMemObject"), LL_DEBUG);

    m_clMemObjectType = clObjType;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemoryObject D'tor
///////////////////////////////////////////////////////////////////////////////////////////////////
GenericMemObject::~GenericMemObject()
{
    LOG_DEBUG(TEXT("%s"), TEXT("Enter GenericMemObject D'tor"));

    remove_device_objects();

    if ( nullptr != m_pBackingStore )
    {
        m_pBackingStore->RemovePendency();
    }

    if ( nullptr != m_BS )
    {
        m_BS->RemovePendency();
    }

    // Notify user that memory object was already destructed and raw data may be deleted
    NotifyDestruction();
    RELEASE_LOGGER_CLIENT;
}

void GenericMemObject::remove_device_objects(void)
{
    if (m_active_groups_count > 0)
    {
        for (unsigned int i = 0; i < MAX_DEVICE_SHARING_GROUP_ID; ++i)
        {
            SharingGroup& group = m_sharing_groups[i];

            if (nullptr != group.m_dev_mem_obj)
            {
                group.m_dev_mem_obj->clDevMemObjRelease();
                group.m_dev_mem_obj = nullptr;
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

static inline bool is_image_ok_for_device( unsigned int        dim_count,
                                           const size_t*    dimension,
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
                               cl_mem_flags                clMemFlags,
                               const cl_image_format*    pclImageFormat,
                               unsigned int                dim_count,
                               const size_t*            dimension,
                               const size_t*            pitches,
                               void*                    pHostPtr,
                               cl_rt_memobj_creation_flags creation_flags,
                               size_t                      force_alignment
                               )
{
    assert(0 == m_pParentObject); // Should never be called for sub mem objects!

    m_clFlags = clMemFlags;
    m_pHostPtr = pHostPtr;

    // assign default value
    if (0 == m_clFlags)
    {
        m_clFlags |= CL_MEM_READ_WRITE;
    }

    m_uiNumDim      = dim_count;
    if ((dim_count < 1) || (dim_count > MAX_WORK_DIM))
    {
        // error
        return CL_INVALID_VALUE;
    }

    if (nullptr == dimension)
    {
        return CL_INVALID_VALUE;
    }

    // caclulate alignment requirements and filter out non-conformant devices
    cl_uint             dev_count = 0;
    SharedPtr<FissionableDevice> *pDevices  = m_pContext->GetDevices(&dev_count);

    assert( (0 != dev_count) && (nullptr != pDevices) );

    IOCLDevRawMemoryAllocator* pDeviceRawMemAllocator = nullptr;

    bool   used_by_DMA = false;
    size_t alignment = 1; // no alignment
    size_t preferred_alignment = alignment;
    size_t object_size = GenericMemObjectBackingStore::calculate_size(
                            GenericMemObjectBackingStore::get_element_size( pclImageFormat ),
                            dim_count, dimension, pitches );

    cl_err_code no_devices_error_code = CL_SUCCESS;

    for ( cl_uint dev_idx = 0; dev_idx < dev_count; ++dev_idx)
    {
        const SharedPtr<FissionableDevice>& dev = pDevices[ dev_idx ];
        assert( dev );

        cl_dev_alloc_prop device_properties;
        cl_dev_err_code   err;

        err = dev->GetDeviceAgent()->clDevGetMemoryAllocProperties( m_clMemObjectType, &device_properties );

        assert( CL_DEV_SUCCEEDED( err ) );
        assert( (unsigned int)device_properties.imageSharingGroupId  < MAX_DEVICE_SHARING_GROUP_ID );
        assert( (unsigned int)device_properties.bufferSharingGroupId < MAX_DEVICE_SHARING_GROUP_ID );
        if ((unsigned int)device_properties.bufferSharingGroupId >= MAX_DEVICE_SHARING_GROUP_ID ||
            (unsigned int)device_properties.imageSharingGroupId  >= MAX_DEVICE_SHARING_GROUP_ID)
        {
            return CL_INVALID_VALUE;
        }
        assert( 0 == (device_properties.alignment & (device_properties.alignment - 1))
                        && "Device Mem alignment requirement is not a power of 2");
        assert( 0 == (device_properties.preferred_alignment & (device_properties.preferred_alignment - 1))
                        && "Device Mem preferred_alignment requirement is not a power of 2");
        assert( device_properties.alignment <= device_properties.preferred_alignment );

        unsigned int sharingGroupId;

        // filter out devices that do not support this memory object
        if (CL_MEM_OBJECT_BUFFER == m_clMemObjectType || CL_MEM_OBJECT_PIPE == m_clMemObjectType)
        {
            sharingGroupId = (unsigned int)device_properties.bufferSharingGroupId;

            if (! is_buffer_ok_for_device(object_size, device_properties))
            {
                no_devices_error_code = CL_INVALID_BUFFER_SIZE;
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

        alignment            = MAX( alignment,            device_properties.alignment );
        preferred_alignment = MAX( preferred_alignment, device_properties.preferred_alignment );
        used_by_DMA            = used_by_DMA || device_properties.usedByDMA;

        if (device_properties.mustAllocRawMemory)
        {
            // device must allocate raw memory for BS for zero-copy sharing between RT and device

            if (nullptr != pDeviceRawMemAllocator)
            {
                // more than one device must allocate the raw memory
                // we dont support this case - skip this device
                continue;
            }

            pDeviceRawMemAllocator = dev->GetDeviceAgent()->clDevGetRawMemoryAllocator();
            assert(nullptr != pDeviceRawMemAllocator && "Device should return valid allocator when its mustAllocRawMemory is true");
            if (nullptr == pDeviceRawMemAllocator)
            {
                continue;
            }
        }

        // add device to the list
        m_device_descriptors.push_back( DeviceDescriptor(dev, sharingGroupId,
                                              device_properties.alignment) );
        DeviceDescriptor& last_added = m_device_descriptors.back();
        
        m_sharing_groups[ sharingGroupId ].m_device_list.push_back( &last_added );
        m_device_2_descriptor_map[ dev.GetPtr() ] = &last_added;

        // add device agent to the list
        m_device_agents.push_back( dev->GetDeviceAgent() );
    }

    // If force_alignment is not 0, e.g. in the case of USM allocations, assign
    // it to preferred_alignment since memory allocation in
    // GenericMemObjectBackingStore actually uses preferred_alignment.
    if (0 != force_alignment)
        preferred_alignment = force_alignment;

    if (m_device_descriptors.empty())
    {
        // all devices were filtered out
        return no_devices_error_code;
    }

    // Create Backing Store
    m_BS = new GenericMemObjectBackingStore(   m_clFlags,
                                               pclImageFormat,
                                               dim_count,
                                               dimension,
                                               pitches,
                                               pHostPtr,
                                               alignment,
                                               preferred_alignment,
                                               used_by_DMA,
                                               GetContext()->GetMemoryObjectsHeap(),
                                               creation_flags,
                                               pDeviceRawMemAllocator);

    if (nullptr == m_BS)
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

    data_sharing_set_init_state( m_BS->IsDataValid() );
    
    // Now we should set backing store
    // Get access to internal pointer
    m_pMemObjData = m_pBackingStore->GetRawData();

    m_stMemObjSize  = m_BS->GetRawDataSize();

    return CL_SUCCESS;
}

cl_err_code GenericMemObject::InitializeSubObject(
                                            cl_mem_flags      clMemFlags,
                                            GenericMemObject& parent,
                                            const size_t     origin[MAX_WORK_DIM],
                                            const size_t     region[MAX_WORK_DIM])
{
    // sub-buffer related - used by internal functions call later in this function
    SharedPtr<GenericMemObject> pParent = &parent;
    m_pParentObject = &parent;
    MEMCPY_S( m_stOrigin, sizeof(m_stOrigin), origin, sizeof(size_t[MAX_WORK_DIM]) );

    // copy everything from GenericMemObject class only excluding parent classes

    m_clFlags   = clMemFlags;

    // Create Backing Store
    assert( (nullptr != parent.m_pBackingStore->GetRawData()) &&
            "Parent Memory Object Backing Store must be finalized during SubObject creation" );

    m_BS = new GenericMemObjectBackingStore( clMemFlags,
                                             parent.m_pBackingStore,
                                             origin,
                                             region,
                                             *(parent.m_BS));

    if (nullptr == m_BS)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    // just start with both pointers equal
    m_BS->AddPendency();
    m_pBackingStore = m_BS;

    m_pHostPtr = m_BS->GetUserProvidedHostMapPtr();
    m_uiNumDim = (cl_uint)m_BS->GetDimCount();

    size_t start_offset = parent.m_BS->GetRawDataOffset(origin);

    for (unsigned int group_idx = 0; group_idx < MAX_DEVICE_SHARING_GROUP_ID; ++group_idx)
    {
        const SharingGroup& p_grp = parent.m_sharing_groups[group_idx];

        if (p_grp.m_device_list.empty())
        {
            continue;
        }

        SharingGroup& grp = m_sharing_groups[group_idx];

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

            m_device_descriptors.push_back( DeviceDescriptor( *p_dev_desc ) );
            DeviceDescriptor& last_added = m_device_descriptors.back();

            grp.m_device_list.push_back( &last_added );            

            m_device_2_descriptor_map[ last_added.m_pDevice.GetPtr() ] = &last_added;
            m_device_agents.push_back( p_dev_desc->m_pDevice->GetDeviceAgent() );
            
        }

        cl_err_code dev_err = allocate_object_for_sharing_group( group_idx );

        if ( CL_FAILED(dev_err) )
        {
            remove_device_objects();
            return CL_OUT_OF_RESOURCES;
        }       

        if (DATA_COPY_STATE_VALID == p_grp.m_data_copy_state)
        {
            grp.m_data_copy_state = DATA_COPY_STATE_VALID;
            m_data_valid_state.m_contains_valid_data = true;
            m_data_valid_state.m_data_sharing_state = MULTIPLE_VALID_WITH_PARALLEL_WRITERS_HISTORY;
        }
        else
        {
            grp.m_data_copy_state = DATA_COPY_STATE_INVALID;
        }

    }

    if (m_device_descriptors.empty())
    {
        // all devices were filtered out
        return CL_MISALIGNED_SUB_BUFFER_OFFSET;
    }

    // Now we should set backing store
    // Get access to internal pointer
    m_pMemObjData = m_pBackingStore->GetRawData();

    // sub-buffer related
    m_stMemObjSize  = m_BS->GetRawDataSize();

    return CL_SUCCESS;
}

cl_err_code GenericMemObject::UpdateHostPtr(cl_mem_flags clMemFlags, void* pHostPtr)
{
    // Currently GenericMemObject doesn't support updating memory flags
    if (m_clFlags != clMemFlags)
    {
        return CL_INVALID_OPERATION;
    }

    m_pHostPtr = pHostPtr;
    return CL_SUCCESS;
}

const GenericMemObject::DeviceDescriptor* GenericMemObject::get_device( const FissionableDevice* dev ) const
{
    TDevice2DescPtrMap::const_iterator found = m_device_2_descriptor_map.find( dev );
    return (found != m_device_2_descriptor_map.end()) ? found->second : nullptr;
}

cl_err_code GenericMemObject::allocate_object_for_sharing_group( unsigned int group_id )
{
    assert( group_id < MAX_DEVICE_SHARING_GROUP_ID );
    SharingGroup& group      = m_sharing_groups[group_id];
    IOCLDevMemoryObject* obj = device_object(group);

    if (nullptr != obj)
    {
        return CL_SUCCESS;
    }

    if (group.m_device_list.empty())
    {
        return CL_INVALID_VALUE;
    }

    const SharedPtr<FissionableDevice>& dev = group.m_device_list.front()->m_pDevice;

    cl_err_code err = create_device_object( m_clFlags, dev, m_BS, &obj );

    if ( CL_FAILED(err) )
    {
        return err;
    }

    ++m_active_groups_count;
    group.m_dev_mem_obj = obj;

    return CL_SUCCESS;
}

cl_err_code GenericMemObject::create_device_object( cl_mem_flags clMemFlags,
                                                    const SharedPtr<FissionableDevice>& dev,
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

cl_err_code GenericMemObject::CreateDeviceResource(const SharedPtr<FissionableDevice>& pDevice)
{
    DeviceDescriptor* desc = get_device( pDevice.GetPtr() );

    if (nullptr == desc)
    {
        return CL_INVALID_DEVICE;
    }

    // Assumption: All devices are added at class initialization time so data may be modified at runtime only
    //             because of lazy sharing groups initialization.
    //             m_global_lock may be taken only during sharing group lazy initialization.

    OclAutoMutex lock( &m_global_lock );

    return allocate_object_for_sharing_group((unsigned int)desc->m_sharing_group_id);
}

cl_err_code GenericMemObject::GetDeviceDescriptor(const SharedPtr<FissionableDevice>& pDevice, IOCLDevMemoryObject* *ppDevObject, SharedPtr<OclEvent>* ppEvent)
{
    assert(nullptr != ppDevObject);

    DeviceDescriptor* desc = get_device( pDevice.GetPtr() );

    if (nullptr == desc)
    {
        return CL_INVALID_DEVICE;
    }

    *ppDevObject = device_object( *desc );
    assert(nullptr != *ppDevObject);

    return CL_SUCCESS;
}

cl_err_code GenericMemObject::UpdateDeviceDescriptor(const SharedPtr<FissionableDevice>& pDevice, IOCLDevMemoryObject* *ppDevObject)
{
    assert(0 && "GenericMemObject is not supporting this operation");
    return CL_INVALID_OPERATION;
}

cl_err_code    GenericMemObject::MemObjCreateDevMappedRegion(
                                        const SharedPtr<FissionableDevice>& pDevice,
                                        cl_dev_cmd_param_map*    cmd_param_map,
                                        void** pHostMapDataPtr )
{
    IOCLDevMemoryObject* dev_object;
    cl_err_code          err;

    err = GetDeviceDescriptor(pDevice, &dev_object, nullptr);

    if (CL_FAILED(err))
    {
        return err;
    }

    cl_dev_err_code dev_err =  dev_object->clDevMemObjCreateMappedRegion(cmd_param_map);

    if (CL_DEV_SUCCEEDED(dev_err))
    {
        *pHostMapDataPtr = cmd_param_map->ptr;
        void* user_provided_ptr = m_BS->GetUserProvidedHostMapPtr();
        if (nullptr != user_provided_ptr)
        {
            // if user provided Host Map pointer all mappings should be returned in this area.
            *pHostMapDataPtr = (cl_uchar*)user_provided_ptr + m_BS->GetRawDataOffset( cmd_param_map->origin );
        }
        return CL_SUCCESS;
    }

    return CL_OUT_OF_RESOURCES;
}

cl_err_code    GenericMemObject::MemObjReleaseDevMappedRegion(
                                            const SharedPtr<FissionableDevice>& pDevice,
                                            cl_dev_cmd_param_map*    cmd_param_map,
                                            void* pHostMapDataPtr,
                                            bool force_unmap )
{
    IOCLDevMemoryObject* dev_object;
    cl_err_code          err;

    err = GetDeviceDescriptor(pDevice, &dev_object, nullptr );

    if (CL_FAILED(err))
    {
        return err;
    }

    cl_dev_err_code dev_err =  (force_unmap) ? 
                            dev_object->clDevMemObjUnmapAndReleaseMappedRegion(cmd_param_map)
                            :
                            dev_object->clDevMemObjReleaseMappedRegion(cmd_param_map);

    return CL_DEV_SUCCEEDED(dev_err) ? CL_SUCCESS : CL_INVALID_VALUE;
}

bool GenericMemObject::IsSynchDataWithHostRequired( cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr ) const
{
    return (pMapInfo->ptr != pHostMapDataPtr);
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

cl_err_code GenericMemObject::NotifyDeviceFissioned(const SharedPtr<FissionableDevice>& parent, size_t count, SharedPtr<FissionableDevice>* children)
{
    // TODO: DK: What should I do here?
    return CL_SUCCESS;
}

// IOCLDevRTMemObjectService Methods
cl_dev_err_code GenericMemObject::GetBackingStore(cl_dev_bs_flags flags, IOCLDevBackingStore* *ppBS)
{
    assert(nullptr!= ppBS);
    assert(nullptr!= m_pBackingStore);

    *ppBS = m_pBackingStore;
    return CL_DEV_SUCCESS;
}

// IOCLDevRTMemObjectService Methods
cl_dev_err_code GenericMemObject::GetBackingStore(cl_dev_bs_flags flags, const IOCLDevBackingStore* *ppBS) const
{
    assert(nullptr!= ppBS);
    assert(nullptr!= m_pBackingStore);

    *ppBS = m_pBackingStore;
    return CL_DEV_SUCCESS;
}

cl_dev_err_code GenericMemObject::SetBackingStore(IOCLDevBackingStore* pBS)
{
    pBS->AddPendency();
    IOCLDevBackingStore* pOldBS = m_pBackingStore.exchange(pBS);
    if ( nullptr != pOldBS )
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
    LOG_DEBUG("Enter ReadData (szRowPitch=%d, pData=%d, szSlicePitch=%d)", szRowPitch, pData, szSlicePitch);

    SMemCpyParams            sCpyParam;

    // Region
    sCpyParam.uiDimCount = (cl_uint)m_BS->GetDimCount();

    // set source pointer (we)
    sCpyParam.pSrc = (cl_char*)m_pBackingStore->GetRawData() + m_BS->GetRawDataOffset( pszOrigin );

    // set destination (them)
    sCpyParam.pDst = (cl_char*)pData;

    // optimize for the case of single dimension
    if (1 == sCpyParam.uiDimCount)
    {
        //useless memcpy_s, this function is called from many different places
        MEMCPY_S( sCpyParam.pDst, pszRegion[0] * m_BS->GetElementSize(), sCpyParam.pSrc, pszRegion[0] * m_BS->GetElementSize() );
        return CL_SUCCESS;
    }

    // assumption: origin and region both have MAX_WORK_DIM members
    MEMCPY_S(sCpyParam.vRegion, sizeof(sCpyParam.vRegion), pszRegion, sizeof(sCpyParam.vRegion));
    sCpyParam.vRegion[0] = pszRegion[0] * m_BS->GetElementSize();

    // set row Pitch for src (we) and dst (them)
    MEMCPY_S(sCpyParam.vSrcPitch, sizeof(sCpyParam.vSrcPitch), m_BS->GetPitch(), sizeof(sCpyParam.vSrcPitch));
    sCpyParam.vDstPitch[0] = szRowPitch;
    sCpyParam.vDstPitch[1] = szSlicePitch;

    clCopyMemoryRegion(&sCpyParam);

    return CL_SUCCESS;
}

cl_err_code GenericMemObject::WriteData(const void * pData,
                                        const size_t * pszOrigin, const size_t * pszRegion,
                                        size_t szRowPitch, size_t szSlicePitch)
{
    LOG_DEBUG("Enter WriteData (szRowPitch=%d, pData=%d, szSlicePitch=%d)", szRowPitch, pData, szSlicePitch);

    SMemCpyParams            sCpyParam;

    // Region
    sCpyParam.uiDimCount = (cl_uint)m_BS->GetDimCount();

    // set source pointer (them)
    sCpyParam.pSrc = (cl_char*)pData;

    // set destination (we)
    sCpyParam.pDst = (cl_char*)m_pBackingStore->GetRawData() + m_BS->GetRawDataOffset( pszOrigin );

    // optimize for the case of single dimension
    if (1 == sCpyParam.uiDimCount)
    {
        //useless memcpy_s, this function is called from many different places
        MEMCPY_S( sCpyParam.pDst, pszRegion[0] * m_BS->GetElementSize(), sCpyParam.pSrc, pszRegion[0] * m_BS->GetElementSize() );
        return CL_SUCCESS;
    }

    // assumption: origin and region both have MAX_WORK_DIM members
    MEMCPY_S(sCpyParam.vRegion, sizeof(sCpyParam.vRegion), pszRegion, sizeof(sCpyParam.vRegion));
    sCpyParam.vRegion[0] = pszRegion[0] * m_BS->GetElementSize();

    // set row Pitch for src (them) and dst (we)
    sCpyParam.vSrcPitch[0] = szRowPitch;
    sCpyParam.vSrcPitch[1] = szSlicePitch;
    MEMCPY_S(sCpyParam.vDstPitch, sizeof(sCpyParam.vDstPitch) , m_BS->GetPitch(), sizeof(sCpyParam.vDstPitch));

    clCopyMemoryRegion(&sCpyParam);

    return CL_SUCCESS;
}

cl_err_code GenericMemObject::CheckBounds( const size_t* pszOrigin, const size_t* pszRegion) const
{
    const size_t* my_dims  = m_BS->GetDimentions();
    unsigned int dim_count = (unsigned int)m_BS->GetDimCount();

    for (unsigned int i = 0; i < dim_count; ++i)
    {
        if ((pszOrigin[i] + pszRegion[i]) > my_dims[i])
        {
            return CL_INVALID_VALUE;
        }
    }
    if (CL_MEM_OBJECT_IMAGE2D == m_clMemObjectType || CL_MEM_OBJECT_IMAGE1D_ARRAY == m_clMemObjectType)
    {
        if (0 != pszOrigin[2] || 1 != pszRegion[2])
        {
            return CL_INVALID_VALUE;
        }
    }
    else if (CL_MEM_OBJECT_IMAGE1D == m_clMemObjectType || CL_MEM_OBJECT_IMAGE1D_BUFFER == m_clMemObjectType)
    {
        if (0 != pszOrigin[1] || 0 != pszOrigin[2] || 1 != pszRegion[1] || 1 != pszRegion[2])
        {
            return CL_INVALID_VALUE;
        }
    }
    return CL_SUCCESS;
}

cl_err_code GenericMemObject::GetDimensionSizes( size_t* pszRegion ) const
{
    assert( pszRegion );
    MEMCPY_S( pszRegion, sizeof(size_t) * MAX_WORK_DIM, m_BS->GetDimentions(), sizeof( size_t ) * m_BS->GetDimCount() );
    return CL_SUCCESS;
}

cl_err_code GenericMemObject::CheckBoundsRect( const size_t* pszOrigin, const size_t* pszRegion,
                                               size_t szRowPitch, size_t szSlicePitch) const
{
    size_t pitches[2] = { szRowPitch, szSlicePitch };
    unsigned int dim_count = (unsigned int)m_BS->GetDimCount();

    size_t raw_base_offset = m_BS->calculate_offset(m_BS->GetElementSize(),
                                                    dim_count,
                                                    pszOrigin,
                                                    pitches );

    size_t raw_delta_size = m_BS->calculate_size(    m_BS->GetElementSize(),
                                                    dim_count,
                                                    pszRegion,
                                                    pitches );

    return ((raw_base_offset + raw_delta_size) <= m_BS->GetRawDataSize()) ? CL_SUCCESS : CL_INVALID_MEM_OBJECT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedImage2D::GetImageInfo()
///////////////////////////////////////////////////////////////////////////////////////////////////
cl_err_code GenericMemObject::GetImageInfo(cl_image_info clParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
{
    LOG_DEBUG(TEXT("%s"), TEXT("Enter:(clParamName=%d, szParamValueSize=%d, pParamValue=%d, pszParamValueSizeRet=%d)"),
        clParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

    if (nullptr == pParamValue && nullptr == pszParamValueSizeRet)
    {
        return CL_INVALID_VALUE;
    }

    if (CL_MEM_OBJECT_BUFFER == m_clMemObjectType || CL_MEM_OBJECT_PIPE == m_clMemObjectType)
    {
        return CL_INVALID_MEM_OBJECT;
    }

    // init MemoryObject fields
    const size_t* dims = m_BS->GetDimentions();
    const size_t* pits = m_BS->GetPitch();
    size_t     elem_size = m_BS->GetElementSize();
    const size_t szRowPitch = pits[0] != 0 ? pits[0] : elem_size * dims[0];
    const size_t szSlicePitch = pits[1] != 0 ? pits[1] : szRowPitch * dims[1];
    size_t  szSize = 0;
    const void * pValue = nullptr;
    size_t    stZero = 0;
    cl_uint uiZero = 0;
    cl_mem imageBuffer = GetParent() != 0 ? const_cast<_cl_mem_int*>(GetParent()->GetHandle()) : CL_INVALID_HANDLE;

    switch (clParamName)
    {
    case CL_IMAGE_FORMAT:
        szSize = sizeof(cl_image_format);
        pValue = (void*)&m_BS->GetFormat();
        break;
    case CL_IMAGE_ELEMENT_SIZE:
        szSize = sizeof(size_t);
        pValue = &elem_size;
        break;
    case CL_IMAGE_ROW_PITCH:
        szSize = sizeof(size_t);
        pValue = (void*)&szRowPitch;
        break;
    case CL_IMAGE_WIDTH:
        szSize = sizeof(size_t);
        pValue = (void*)&dims[0];
        break;

    case CL_IMAGE_HEIGHT:
        if ((CL_MEM_OBJECT_IMAGE1D          != m_clMemObjectType) &&
            (CL_MEM_OBJECT_IMAGE1D_BUFFER != m_clMemObjectType) &&
            (CL_MEM_OBJECT_IMAGE1D_ARRAY  != m_clMemObjectType))
        {
            szSize = sizeof(size_t);
            pValue = (void*)&dims[1];
        }
        else
        {
            szSize = sizeof(size_t);
            pValue = &stZero;
        }
        break;

    case CL_IMAGE_SLICE_PITCH:
        if ((CL_MEM_OBJECT_IMAGE1D          != m_clMemObjectType) &&
            (CL_MEM_OBJECT_IMAGE1D_BUFFER != m_clMemObjectType) &&
            (CL_MEM_OBJECT_IMAGE2D          != m_clMemObjectType))
        {
            szSize = sizeof(size_t);
            pValue = (void*)&szSlicePitch;
        }
        else
        {
            szSize = sizeof(size_t);
            pValue = &stZero;
        }
        break;

    case CL_IMAGE_DEPTH:
        if ((CL_MEM_OBJECT_IMAGE1D          != m_clMemObjectType) &&
            (CL_MEM_OBJECT_IMAGE1D_BUFFER != m_clMemObjectType) &&
            (CL_MEM_OBJECT_IMAGE2D          != m_clMemObjectType) &&
            (CL_MEM_OBJECT_IMAGE1D_ARRAY  != m_clMemObjectType) &&
            (CL_MEM_OBJECT_IMAGE2D_ARRAY  != m_clMemObjectType))
        {
            szSize = sizeof(size_t);
            pValue = (void*)&dims[2];
        }
        else
        {
            szSize = sizeof(size_t);
            pValue = &stZero;
        }
        break;

    case CL_IMAGE_ARRAY_SIZE:
        szSize = sizeof(size_t);
        if (CL_MEM_OBJECT_IMAGE1D_ARRAY == m_clMemObjectType)
        {
            pValue = &dims[1];
        }
        else if (CL_MEM_OBJECT_IMAGE2D_ARRAY == m_clMemObjectType)
        {
            pValue = &dims[2];
        }
        else
        {
            pValue = &stZero;
        }
        break;            
    case CL_IMAGE_NUM_MIP_LEVELS:
    case CL_IMAGE_NUM_SAMPLES:
        szSize = sizeof(cl_uint);
        pValue = &uiZero;
        break;
    case CL_IMAGE_BUFFER:
        szSize = sizeof(imageBuffer);
        pValue = &imageBuffer;
        break;
    default:
        return CL_INVALID_VALUE;
    }

    if (nullptr != pParamValue && szParamValueSize < szSize)
    {
        return CL_INVALID_VALUE;
    }

    if (nullptr != pszParamValueSizeRet)
    {
        *pszParamValueSizeRet = szSize;
    }

    if (nullptr != pParamValue && szSize > 0)
    {
        MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
    }

    return CL_SUCCESS;

}


void * GenericMemObject::GetBackingStoreData( const size_t * pszOrigin ) const
{
    return (char*)m_pBackingStore->GetRawData() +
            (pszOrigin ? m_BS->GetRawDataOffset( pszOrigin ) : 0);
}

cl_err_code GenericMemObject::CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type, const void * buffer_create_info, SharedPtr<MemoryObject>* ppBuffer)
{
    const cl_buffer_region* region = static_cast<const cl_buffer_region*>(buffer_create_info);

    size_t pSzOrigin[MAX_WORK_DIM] = {region->origin};
    size_t pSzSize[MAX_WORK_DIM] = {region->size};

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

    GenericMemObjectSubBuffer* pSubBuffer = new GenericMemObjectSubBuffer( m_pContext, m_clMemObjectType, *this);
    if ( nullptr == pSubBuffer )
    {
        return CL_OUT_OF_HOST_MEMORY;
    }

    cl_err_code err = pSubBuffer->InitializeSubObject(clFlags, *this, pSzOrigin,  pSzSize);
    if ( CL_FAILED(err) )
    {
        pSubBuffer->Release();
        LOG_ERROR(TEXT("SubBuffer creation error: %x"), err);
        return err;
    }

    // If shared context (more than one device)
    if (m_active_groups_count > 1)
    {
        // add pSubBuffer to parent m_subBuffersList
        addSubBuffer(pSubBuffer);
    }

    assert(nullptr != ppBuffer);
    *ppBuffer = pSubBuffer;
    return CL_SUCCESS;
}

void GenericMemObject::addSubBuffer(GenericMemObjectSubBuffer* pSubBuffer)
{
    assert(nullptr != pSubBuffer && "pSubBuffer must be valid pointer");
    acquireBufferSyncLock();
    TSubBufferList* pSubBuffersList = getSubBuffersListPtr();
    // If MEMORY_MODE_NORMAL mode check if the new sub-buffer is overlap with other sub-buffer.
    if (MEMORY_MODE_NORMAL == getHierarchicalMemoryMode())
    {
        size_t candidateOrigin = (size_t)pSubBuffer->m_BS->GetRawData();
        assert(pSubBuffer->m_BS->GetDimCount() == 1 && "SubBuffer dimension must be 1");
        size_t start, end;
        for (unsigned int i = 0; i < pSubBuffersList->size(); i++)
        {
            start = (size_t)pSubBuffersList->at(i)->m_BS->GetRawData();
            end = start + pSubBuffersList->at(i)->m_BS->GetRawDataSize();
            // If the new sub-buffer origin is in current sub-buffer range --> overlap
            if ((candidateOrigin >= start) && (candidateOrigin < end))
            {
                // Switch to overlapping mode.
                setHierarchicalMemoryMode(MEMORY_MODE_OVERLAPPING);
                TSubBufferList* pUpdateParentList = getUpdateParentListPtr();
                // push all the sub-buffers to updateParentList in order to update the parent at the next memory request.
                pUpdateParentList->insert(pUpdateParentList->end(), pSubBuffersList->begin(), pSubBuffersList->end());
                setUpdateParentFlag(true);
                break;
            }
        }
    }
    if (MEMORY_MODE_OVERLAPPING == getHierarchicalMemoryMode())
    {
        // Push the new sub-buffer to updateParentList (The other sub-buffers already close to the parent)
        TSubBufferList* pUpdateParentList = getUpdateParentListPtr();
        // push the new sub-buffer to updateParentList in order to move the new sub-buffer and the parent buffer to single device at the next memory request.
        pUpdateParentList->push_back(pSubBuffer);
        setUpdateParentFlag(true);
    }
    // Add pSubBuffer to m_subBuffersList
    pSubBuffersList->push_back(pSubBuffer);
    pSubBuffer->RegisteredByParent();
    releaseBufferSyncLock();
}

void GenericMemObject::updateHierarchicalMemoryMode()
{
    // If we in MEMORY_MODE_NORMAL mode, the deletion of sub-buffer cannot change the mode --> finish.
    if (MEMORY_MODE_NORMAL == getHierarchicalMemoryMode())
    {
        return;
    }
    TSubBufferList* pSubBuffersList = getSubBuffersListPtr();
    vector<sub_buffer_region> vSubBuffersRegion;
    sub_buffer_region tSubBufferRegion;
    // Add sub-buffers regions to vector
    for (unsigned int i = 0; i < pSubBuffersList->size(); i++)
    {
        vSubBuffersRegion.push_back( sub_buffer_region((size_t)(pSubBuffersList->at(i)->m_BS->GetRawData()), pSubBuffersList->at(i)->m_BS->GetRawDataSize()) );
    }
    // Sort the sub-buffers region according to origion
    sort(vSubBuffersRegion.begin(), vSubBuffersRegion.end(), tSubBufferRegion);
    bool isSwitchToNormal = true;
    // Check in the sorted vector if the next sub-buffer is in the current sub-buffer, if so --> overlapping.
    for (unsigned int i = 1; i < vSubBuffersRegion.size(); i++)
    {
        if (vSubBuffersRegion[i].m_origin < (vSubBuffersRegion[i - 1].m_origin + vSubBuffersRegion[i - 1].m_size))
        {
            isSwitchToNormal = false;
            break;
        }
    }
    if (isSwitchToNormal)
    {
        setHierarchicalMemoryMode(MEMORY_MODE_NORMAL);
    }
}

//////////////////////////////////////////////////////////////////////////
/// GenericMemObjectBackingStore
//////////////////////////////////////////////////////////////////////////
GenericMemObjectBackingStore::GenericMemObjectBackingStore(
                               cl_mem_flags                 clMemFlags,
                               const cl_image_format*       pclImageFormat,
                               unsigned int                 dim_count,
                               const size_t*                dimensions,
                               const size_t*                pitches,
                               void*                        pHostPtr,
                               size_t                       alignment,
                               size_t                       preferred_alignment,
                               bool                         used_by_DMA,
                               ClHeap                       heap,
                               cl_rt_memobj_creation_flags  creation_flags,
                               IOCLDevRawMemoryAllocator*   pRawMemoryAllocator) :
    m_ptr(nullptr), m_dim_count(dim_count), m_pHostPtr(pHostPtr), m_user_flags(clMemFlags),
    m_data_valid(nullptr != pHostPtr), m_used_by_DMA( used_by_DMA), m_alignment(alignment), m_preferred_alignment(preferred_alignment),
    m_raw_data_size(0), m_heap(heap), m_pRawMemoryAllocator(pRawMemoryAllocator), m_parent(nullptr), m_refCount(1)
{
    if (nullptr != pclImageFormat)
    {
        m_format       = *pclImageFormat;
    }
    else
    {
        memset( &m_format, 0, sizeof( m_format ) );
    }

    m_element_size = get_element_size( pclImageFormat );


    // caclulate pitches and dimensions
    assert( nullptr != dimensions );
    calculate_pitches_and_dimentions( m_element_size, dim_count, dimensions, pitches, m_dimensions, m_pitches );

    // calc raw data size
    m_raw_data_size = calculate_size(m_element_size, (cl_uint)m_dim_count, m_dimensions, m_pitches);

    assert( IS_ALIGNED_ON(m_alignment, m_alignment) && "Alignment is not power of 2" );
    assert( IS_ALIGNED_ON(m_preferred_alignment, m_preferred_alignment) && "Preferred alignment is not power of 2" );

    // can user data be used?
    if (nullptr != pHostPtr)                              // user provided some pointer
    {
        assert( (m_user_flags & (CL_MEM_USE_HOST_PTR | CL_MEM_COPY_HOST_PTR)) ||
                (CL_RT_MEMOBJ_FORCE_BS & creation_flags) );
        m_pHostPtr = pHostPtr;

        if ((CL_RT_MEMOBJ_FORCE_BS & creation_flags) || (m_user_flags & CL_MEM_USE_HOST_PTR))
        {
            m_ptr = m_pHostPtr;
        }
    }
}

GenericMemObjectBackingStore::GenericMemObjectBackingStore(
                               cl_mem_flags             clMemFlags,
                               IOCLDevBackingStore*     parent_ps, // used for raw data only
                               const size_t*            origin,
                               const size_t*            region,
                               GenericMemObjectBackingStore&  copy_setting_from ):
    m_ptr(nullptr), m_dim_count(0), m_pHostPtr(nullptr), m_user_flags(clMemFlags),
    m_data_valid(true), m_alignment(0), m_preferred_alignment(0), m_heap(nullptr),
    m_parent(parent_ps), m_refCount(1)
{
    size_t raw_data_offset = copy_setting_from.GetRawDataOffset(origin);

    m_ptr = (cl_uchar*)parent_ps->GetRawData() + raw_data_offset;
    void *parentUserHostPtr = parent_ps->GetUserProvidedHostMapPtr();
    if (parentUserHostPtr)
    {
        m_pHostPtr = (cl_uchar*)parentUserHostPtr + raw_data_offset;
    }

    m_dim_count = copy_setting_from.m_dim_count;

    MEMCPY_S( m_dimensions, sizeof(m_dimensions), region, sizeof(m_dimensions) );

    MEMCPY_S( m_pitches, sizeof(m_pitches), copy_setting_from.m_pitches, sizeof(m_pitches) );

    m_format       = copy_setting_from.m_format;
    m_element_size = copy_setting_from.m_element_size;
    m_alignment    = copy_setting_from.m_alignment;

    // calc raw data size
    m_raw_data_size = calculate_size(m_element_size, (cl_uint)m_dim_count, m_dimensions, m_pitches);

    m_parent->AddPendency();
}

GenericMemObjectBackingStore::~GenericMemObjectBackingStore()
{
    if (m_parent)
    {
        // subobject
        m_parent->RemovePendency();
    }
    else if (nullptr != m_ptr)
    {
        if (m_used_by_DMA)
        {
            clHeapUnmarkSafeForDMA( m_ptr, m_raw_data_size );
        }
        if (m_ptr != m_pHostPtr)
        {
            if (nullptr != m_pRawMemoryAllocator)
            {
                // de-allocate from device
                m_pRawMemoryAllocator->clDevFreeRawMemory( m_ptr );
            }
            else
            {
                clFreeHeapPointer( m_heap, m_ptr );
            }
        }
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
    if (nullptr == format)
    {
        return 1;
    }

    size_t stChannels = 0;
    size_t stChSize = 0;
    switch (format->image_channel_order)
    {
    case CL_R:
    case CL_A:
    case CL_LUMINANCE:
    case CL_INTENSITY:
    case CL_DEPTH:
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
    case CL_sRGBA:
    case CL_sBGRA:
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

void GenericMemObjectBackingStore::calculate_pitches_and_dimentions( 
                                       size_t elem_size, unsigned int  dim_count, 
                                       const size_t user_dims[], const size_t user_pitches[],
                                       size_t  dimensions[], size_t  pitches[] )
{
    assert( nullptr != user_dims );

    memset( dimensions, 0, sizeof(dimensions[0]) * MAX_WORK_DIM );
    memset( pitches, 0, sizeof(pitches[0]) * (MAX_WORK_DIM - 1) );

    dimensions[0]        = user_dims[0];
    size_t prev_pitch   = elem_size;

    for ( cl_uint i = 1; i < dim_count; ++i)
    {
        dimensions[i]     = user_dims[i];
        size_t next_pitch = dimensions[i-1] * prev_pitch;

        if ((nullptr != user_pitches) && (user_pitches[i-1] > next_pitch))
        {
            next_pitch = user_pitches[i-1];
        }

        pitches[i-1] = next_pitch;
        prev_pitch   = next_pitch;
    }
}

size_t GenericMemObjectBackingStore::calculate_size( size_t elem_size, unsigned int  dim_count,
                                                     const size_t  dimensions[], const size_t  pitches[] )
{
    size_t tmp_dims[MAX_WORK_DIM];
    size_t tmp_pitches[MAX_WORK_DIM-1];

    const size_t* working_dims    = dimensions;
    const size_t* working_pitches = pitches;

    if ((nullptr == pitches) || ((dim_count > 1) && (0 == pitches[0])))
    {
        calculate_pitches_and_dimentions( elem_size, dim_count, dimensions, pitches, tmp_dims, tmp_pitches );
        working_dims    = tmp_dims;
        working_pitches = tmp_pitches;
    }

    return (1 == dim_count) ?
                        working_dims[0] * elem_size :
                        working_pitches[dim_count-2] * working_dims[dim_count-1];
}

size_t GenericMemObjectBackingStore::GetRawDataOffset( const size_t* origin ) const
{
    return calculate_offset( m_element_size, (cl_uint)m_dim_count, origin, m_pitches );
}

bool GenericMemObjectBackingStore::AllocateData( void )
{
    // check if data already allocated
    if (nullptr != m_ptr)
    {
        if (m_used_by_DMA)
        {
            if (0 != clHeapMarkSafeForDMA( m_ptr, m_raw_data_size ))
            {
                return false;
            }
        }
        return true;
    }

    if (nullptr != m_pRawMemoryAllocator)
    {
        m_ptr = m_pRawMemoryAllocator->clDevAllocateRawMemory( m_raw_data_size, m_preferred_alignment );
    }
    else
    {
        m_ptr = clAllocateFromHeap( m_heap, m_raw_data_size, m_preferred_alignment, m_used_by_DMA );
    }

    if (nullptr != m_ptr && m_used_by_DMA)
    {
        if (0 != clHeapMarkSafeForDMA( m_ptr, m_raw_data_size ))
        {
            clFreeHeapPointer( m_heap, m_ptr );
            m_ptr = nullptr;
        }
    }

    if (nullptr != m_ptr && nullptr != m_pHostPtr)
    {
        MEMCPY_S( m_ptr, m_raw_data_size, m_pHostPtr, m_raw_data_size );

        if (0 == (m_user_flags & CL_MEM_USE_HOST_PTR))
        {
            // user provided host pointer may be used only for one-time init and not for mapping
            m_pHostPtr = nullptr;
        }
    }

    return nullptr != m_ptr;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// SingleUnifiedSubBuffer C'tor
///////////////////////////////////////////////////////////////////////////////////////////////////

GenericMemObjectSubBuffer::GenericMemObjectSubBuffer(const SharedPtr<Context>& pContext, cl_mem_object_type clObjType, GenericMemObject& buffer)
    : GenericMemObject(pContext, clObjType), m_rBuffer(buffer), m_bRegisteredByParent(false)
{
}

cl_err_code GenericMemObjectSubBuffer::Initialize(
            cl_mem_flags        clMemFlags,
            const cl_image_format*    pclImageFormat,
            unsigned int        dim_count,
            const size_t*        dimension,  // size == region
            const size_t*       pitches,    // origin
            void*                pHostPtr
            )
{
    assert(0);
    return CL_INVALID_OPERATION;
}

void GenericMemObjectSubBuffer::EnterZombieState( EnterZombieStateLevel call_level )
{
    if (m_bRegisteredByParent)
    {
        const_cast<GenericMemObjectSubBuffer*>(this)->ZombieFlashToParent();
    }
    GenericMemObject::EnterZombieState(RECURSIVE_CALL);
}


cl_err_code GenericMemObjectSubBuffer::create_device_object(
                                                    cl_mem_flags clMemFlags,
                                                    const SharedPtr<FissionableDevice>& dev,
                                                    GenericMemObjectBackingStore* bs,
                                                    IOCLDevMemoryObject** dev_object )
{
    // get parent device object for this device
    IOCLDevMemoryObject* parent_MemObject = nullptr;

    cl_err_code err = m_pParentObject->CreateDeviceResource( dev );
    if (CL_FAILED(err))
    {
        return CL_OUT_OF_RESOURCES;
    }

    err = m_pParentObject->GetDeviceDescriptor( dev, &parent_MemObject, nullptr );

    if (CL_FAILED(err))
    {
        return CL_OUT_OF_RESOURCES;
    }

    cl_dev_err_code devErr = parent_MemObject->clDevMemObjCreateSubObject(
                                    clMemFlags,
                                    m_stOrigin,
                                    bs->GetDimentions(),
                                    this,
                                    dev_object);

    return CL_DEV_FAILED(devErr) ? CL_OUT_OF_RESOURCES : CL_SUCCESS;
}


bool GenericMemObjectSubBuffer::IsSupportedByDevice(const SharedPtr<FissionableDevice>& pDevice)
{
    // Need to check only for sub-buffers

    cl_dev_alloc_prop device_properties;
    cl_dev_err_code   err;

    err = pDevice->GetDeviceAgent()->clDevGetMemoryAllocProperties(
                                    m_clMemObjectType,
                                    &device_properties );

    assert( CL_DEV_SUCCEEDED( err ) );

    return IS_ALIGNED_ON(m_stOrigin[0], device_properties.alignment );
}

cl_err_code    GenericMemObjectSubBuffer::GetInfo(cl_int iParamName, size_t szParamValueSize, void * pParamValue, size_t * pszParamValueSizeRet) const
{
    if (CL_MEM_HOST_PTR == iParamName && (m_rBuffer.GetFlags() & CL_MEM_USE_HOST_PTR))
    {
        const void* pHostPtr = nullptr;
        size_t szSize = sizeof(void*);
        const void* pValue = &pHostPtr;

        const IOCLDevBackingStore* pBackingStore, *pBufBackingStore;

        cl_dev_err_code err = m_rBuffer.GetBackingStore(CL_DEV_BS_GET_IF_AVAILABLE, &pBufBackingStore);
        assert(CL_SUCCEEDED(err));
        const void* const pUserHostPtr = pBufBackingStore->GetUserProvidedHostMapPtr();
        assert(nullptr != pUserHostPtr);
        err = GetBackingStore(CL_DEV_BS_GET_IF_AVAILABLE, &pBackingStore);
        assert(CL_SUCCEEDED(err));
        pHostPtr = (char*)pUserHostPtr + pBackingStore->GetDimentions()[0];

        if (nullptr != pParamValue && szParamValueSize < szSize)
        {
            LOG_ERROR(TEXT("szParamValueSize (=%d) < szSize (=%d)"), szParamValueSize, szSize);
            return CL_INVALID_VALUE;
        }
        // return param value size
        if (nullptr != pszParamValueSizeRet)
        {
            *pszParamValueSizeRet = szSize;
        }
        if (nullptr != pParamValue && szSize > 0 && pValue)
        {
            MEMCPY_S(pParamValue, szParamValueSize, pValue, szSize);
        }
        return CL_SUCCESS;
    }

    return MemoryObject::GetInfo(iParamName, szParamValueSize, pParamValue, pszParamValueSizeRet);

}
