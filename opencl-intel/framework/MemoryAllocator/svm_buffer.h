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

#pragma once

#include "GenericMemObj.h"

namespace Intel { namespace OpenCL { namespace Framework {

/**
 * This class represents an shared memory buffer
 */
class SharedBuffer : public GenericMemObject
{
public:

    SharedBuffer(const SharedPtr<Context>& pContext) :
        GenericMemObject(pContext, CL_MEM_OBJECT_BUFFER) {}

    /**
     * @return the address of the shared buffer
     */
    void* GetAddr() { return GetBackingStoreData(); }
    const void* GetAddr() const { return GetBackingStoreData(); };

    /**
     * @return the size in bytes of the shared buffer
     */
    size_t GetSize() const { return m_pBackingStore->GetRawDataSize(); };

    /**
     * @param ptr    a pointer to some address
     * @param size    a size in bytes
     * @return whether the memory region defined by ptr and size is contained inside this SharedBuffer
     */
    bool IsContainedInBuffer(const void* ptr, size_t size) const
    {
        return ptr >= GetAddr() && (char*)ptr + size <= (char*)GetAddr() + GetSize();
    }

    // overriden methods:

    int ValidateChildFlags(const cl_mem_flags childFlags)
    {
        // We don't use this flag, but our child does. This is by design, so we have to disable this flag for validation.
        return GenericMemObject::ValidateChildFlags(childFlags & ~CL_MEM_USE_HOST_PTR);
    }
};

/**
 * This class represents an SVM buffer
 */
class SVMBuffer : public SharedBuffer
{
public:

    PREPARE_SHARED_PTR(SVMBuffer)

    /**
     * @param pContext Context used to create the SVM buffer
     * @return a newly allocated SVMBuffer
     */
    static SharedPtr<SVMBuffer> Allocate(const SharedPtr<Context>& pContext)
    {
        return new SVMBuffer(pContext);
    }

private:

    SVMBuffer(const SharedPtr<Context>& pContext) : SharedBuffer(pContext) { }

};

/**
 * This class represents an shared memory pointer as an argument to a kernel (both a pointer inside a shared buffer and )
 */
class SharedPointerArg : public MemoryObject
{
public:

    PREPARE_SHARED_PTR(SharedPointerArg)

    // overriden methods:

    virtual cl_err_code Initialize(cl_mem_flags clMemFlags,
                                   const cl_image_format* pclImageFormat,
                                   unsigned int dim_count,
                                   const size_t* dimension,
                                   const size_t* pitches,
                                   void* pHostPtr,
                                   cl_rt_memobj_creation_flags creation_flags,
                                   size_t force_alignment = 0);

    virtual cl_err_code UpdateHostPtr(cl_mem_flags clMemFlags, void* pHostPtr);

    virtual cl_err_code ReadData(void* pOutData, const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);

    virtual cl_err_code WriteData(const void* pOutData,    const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch = 0, size_t szSlicePitch = 0);

    virtual bool IsSynchDataWithHostRequired(cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr) const;

    virtual cl_err_code SynchDataToHost(cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr);

    virtual cl_err_code SynchDataFromHost(cl_dev_cmd_param_map* IN pMapInfo, void* IN pHostMapDataPtr);

    virtual cl_err_code CreateSubBuffer(cl_mem_flags clFlags, cl_buffer_create_type buffer_create_type,    const void* buffer_create_info, SharedPtr<MemoryObject>* ppBuffer);

    virtual bool IsSupportedByDevice(const SharedPtr<FissionableDevice>& pDevice);

protected:

    virtual    cl_err_code    MemObjCreateDevMappedRegion(const SharedPtr<FissionableDevice>&, cl_dev_cmd_param_map* cmd_param_map, void** pHostMapDataPtr);

    virtual    cl_err_code    MemObjReleaseDevMappedRegion(const SharedPtr<FissionableDevice>&, cl_dev_cmd_param_map*    cmd_param_map, void* pHostMapDataPtr, bool force_unmap = false);

    /**
     * @param pBuf    an shared memory Buffer (it will be nullptr for system pointers)
     * @param pArgValue    pointer to the memory inside pBuf or system pointer
     */
    SharedPointerArg(const SharedPtr<SharedBuffer>& pBuf) :
       MemoryObject(pBuf != 0 ? pBuf->GetContext() : SharedPtr<Context>(nullptr)) { }

    /**
     * This class implements IOCLDevMemoryObject for shared buffers as an arguments to a kernel
     */
    class PointerArgDevMemoryObject : public IOCLDevMemoryObject
    {
    public:

        /**
         * Constructor
         * @param pPtrArg        the PointerArg that creates this PointerArgDevMemoryObject
         * @param pBufDevMemObj    the IOCLDevMemoryObject of the Buffer object or nullptr in case of system pointer
         * @param szOffset            offset of the pointer argument relative to the beginning of the buffer
         */
        PointerArgDevMemoryObject(const SharedPtr<SharedPointerArg>& pPtrArg, IOCLDevMemoryObject* pBufDevMemObj, size_t szOffset);

        // overriden methods:

        virtual cl_dev_err_code clDevMemObjCreateMappedRegion(cl_dev_cmd_param_map* pMapParams);

        virtual cl_dev_err_code clDevMemObjUnmapAndReleaseMappedRegion(cl_dev_cmd_param_map* pMapParams);

        virtual cl_dev_err_code clDevMemObjReleaseMappedRegion(cl_dev_cmd_param_map* pMapParams);

        virtual cl_dev_err_code clDevMemObjGetDescriptor(cl_device_type dev_type, cl_dev_subdevice_id node_id, cl_dev_memobj_handle* handle);

        virtual cl_dev_err_code clDevMemObjCreateSubObject(cl_mem_flags mem_flags, const size_t IN* origin, const size_t IN* size, IOCLDevRTMemObjectService IN* pBSService,
            IOCLDevMemoryObject* OUT* ppSubObject);

        virtual cl_dev_err_code clDevMemObjUpdateBackingStore(void* operation_handle, cl_dev_bs_update_state* pUpdateState);

        virtual cl_dev_err_code clDevMemObjUpdateFromBackingStore(void* operation_handle, cl_dev_bs_update_state* pUpdateState);

        virtual cl_dev_err_code clDevMemObjInvalidateData();

        virtual cl_dev_err_code clDevMemObjRelease();

    private:

        IOCLDevMemoryObject*  m_pBufDevMemObj;
        cl_mem_obj_descriptor m_objDecr;

    };

};

/**
 * This class represents a pointer inside a shared buffer as an argument to a kernel
 */
class BufferPointerArg : public SharedPointerArg
{
    PREPARE_SHARED_PTR(BufferPointerArg)

    /**
     * @param pSharedBuf   a shared buffer (it will be nullptr for system pointers)
     * @param pArgValue    pointer to the shared memory inside pSharedBuf or system pointer
     * @return a new BufferPointerArg
     */
    static SharedPtr<BufferPointerArg> Allocate(SharedBuffer* pSharedBuf, const void* pArgValue)
    {
        return new BufferPointerArg(pSharedBuf, pArgValue);
    }

    // overriden methods:

    virtual cl_err_code LockOnDevice(IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT MemObjUsage* pOutActuallyUsage, OUT SharedPtr<OclEvent>& pOutEvent);

    virtual cl_err_code UnLockOnDevice(IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage);

    virtual cl_err_code GetDimensionSizes(size_t* pszRegion) const;

    virtual size_t GetRowPitchSize() const;

    virtual size_t GetSlicePitchSize() const;

    virtual size_t GetPixelSize() const;

    virtual void GetLayout(OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch) const;

    virtual cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const;

    virtual cl_err_code CheckBoundsRect(const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const;

    virtual void* GetBackingStoreData(const size_t* pszOrigin = nullptr) const;

    virtual cl_err_code CreateDeviceResource(const SharedPtr<FissionableDevice>& pDevice);

    virtual cl_err_code GetDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject, SharedPtr<OclEvent> OUT* ppEvent);

    virtual cl_err_code UpdateDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject);

private:

    BufferPointerArg(SharedBuffer* pBuf, const void* pArgValue) :
       SharedPointerArg(pBuf), m_pBuf(pBuf), m_szOffset((char*)pArgValue - (char*)pBuf->GetAddr())
       {
           assert(pArgValue >= pBuf->GetAddr());
       }

    SharedBuffer* m_pBuf;
    const size_t m_szOffset;

};

/**
 * This class represents a system pointer as an argument to a kernel
 */
class SystemPointerArg : public SharedPointerArg
{
public:

    PREPARE_SHARED_PTR(SystemPointerArg)

    /**
     * @param pArgValue the system pointer
     * @return a new SystemPointerArg
     */
    static SharedPtr<SystemPointerArg> Allocate(const void* pArgValue)
    {
        return new SystemPointerArg(pArgValue);
    }

    // overriden methods:

    virtual cl_err_code LockOnDevice(IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage, OUT MemObjUsage* pOutActuallyUsage, OUT SharedPtr<OclEvent>& pOutEvent)
    {
        return CL_SUCCESS;
    }

    virtual cl_err_code UnLockOnDevice(IN const SharedPtr<FissionableDevice>& dev, IN MemObjUsage usage)
    {
        return CL_SUCCESS;
    }

    virtual cl_err_code GetDimensionSizes(size_t* pszRegion) const { ASSERT_RET_VAL(false, "this method should never be called", CL_INVALID_OPERATION); }

    virtual size_t GetRowPitchSize() const { return 0; }

    virtual size_t GetSlicePitchSize() const { return 0; }

    virtual size_t GetPixelSize() const { return 0; }

    virtual void GetLayout(OUT size_t* dimensions, OUT size_t* rowPitch, OUT size_t* slicePitch) const { }

    virtual cl_err_code CheckBounds(const size_t* pszOrigin, const size_t* pszRegion) const { return CL_SUCCESS; }

    virtual cl_err_code CheckBoundsRect(const size_t* pszOrigin, const size_t* pszRegion, size_t szRowPitch, size_t szSlicePitch) const { return CL_SUCCESS; }

    virtual void* GetBackingStoreData(const size_t* pszOrigin = nullptr) const { return const_cast<void*>(m_ptr); }

    virtual cl_err_code CreateDeviceResource(const SharedPtr<FissionableDevice>& pDevice) { return CL_SUCCESS; }

    virtual cl_err_code GetDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject, SharedPtr<OclEvent> OUT* ppEvent)
    {
        *ppDevObject = new PointerArgDevMemoryObject(this, nullptr, 0);
        return CL_SUCCESS;
    }

    virtual cl_err_code UpdateDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject) { return CL_SUCCESS; }

private:

    SystemPointerArg(const void* pArgValue) : SharedPointerArg(nullptr), m_ptr(pArgValue) { }

    const void* const m_ptr;
};

}}}
