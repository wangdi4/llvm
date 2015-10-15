// Copyright (c) 2006-2013 Intel Corporation
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

#pragma once

#include "GenericMemObj.h"

namespace Intel { namespace OpenCL { namespace Framework {

/**
 * This class represents an SVM buffer
 */
class SVMBuffer : public GenericMemObject
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

    /**
     * @return the address of the SVM buffer
     */
    void* GetAddr() { return GetBackingStoreData(); }
    const void* GetAddr() const { return GetBackingStoreData(); };

    /**
     * @return the size in bytes of the SVM buffer
     */
    size_t GetSize() const { return m_pBackingStore->GetRawDataSize(); };

    /**
     * @param ptr    a pointer to some address
     * @param size    a size in bytes
     * @return whether the memory region defined by ptr and size is contained inside this SVMBuffer
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

private:

    SVMBuffer(const SharedPtr<Context>& pContext) : GenericMemObject(pContext, CL_MEM_OBJECT_BUFFER) { }

};

/**
 * This class represents an SVM pointer as an argument to a kernel (both a pointer inside an SVM buffer and )
 */
class SVMPointerArg : public MemoryObject
{
public:

    PREPARE_SHARED_PTR(SVMPointerArg)
    
    // overriden methods:

    virtual cl_err_code Initialize(cl_mem_flags    clMemFlags,    const cl_image_format* pclImageFormat, unsigned int    dim_count, const size_t* dimension,    const size_t* pitches,
        void* pHostPtr,    cl_rt_memobj_creation_flags    creation_flags);

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
     * @param pSvmBuf    an SVMBuffer (it will be NULL for system pointers)
     * @param pArgValue    pointer to the SVM memory inside pSvmBuf or system pointer
     */
    SVMPointerArg(const SharedPtr<SVMBuffer>& pSvmBuf) :
       MemoryObject(pSvmBuf != 0 ? pSvmBuf->GetContext() : SharedPtr<Context>(NULL)) { }    

    /**
     * This class implements IOCLDevMemoryObject for SVM buffers as an arguments to a kernel
     */
    class SVMPointerArgDevMemoryObject : public IOCLDevMemoryObject
    {
    public:

        /**
         * Constructor
         * @param pSvmPtrArg        the SVMPointerArg that creates this SVMPointerArgDevMemoryObject
         * @param pSvmBufDevMemObj    the IOCLDevMemoryObject of the SVMBuffer object or NULL in case of system pointer
         * @param szOffset            offset of the pointer argument relative to the beginning of the SVM buffer
         */
        SVMPointerArgDevMemoryObject(const SharedPtr<SVMPointerArg>& pSvmPtrArg, IOCLDevMemoryObject* pSvmBufDevMemObj, size_t szOffset);

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

        IOCLDevMemoryObject* m_pSvmBufDevMemObj;
        cl_mem_obj_descriptor m_objDecr;

    };

};

/**
 * This class represents a pointer inside an SVM buffer as an argument to a kernel
 */
class SVMBufferPointerArg : public SVMPointerArg
{
    PREPARE_SHARED_PTR(SVMBufferPointerArg)

    /**
     * @param pSvmBuf    an SVMBuffer (it will be NULL for system pointers)
     * @param pArgValue    pointer to the SVM memory inside pSvmBuf or system pointer
     * @return a new SVMBufferPointerArg
     */
    static SharedPtr<SVMBufferPointerArg> Allocate(SVMBuffer* pSvmBuf, const void* pArgValue)
    {
        return new SVMBufferPointerArg(pSvmBuf, pArgValue);
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

    virtual void* GetBackingStoreData(const size_t* pszOrigin = NULL) const;

    virtual cl_err_code CreateDeviceResource(const SharedPtr<FissionableDevice>& pDevice);

    virtual cl_err_code GetDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject, SharedPtr<OclEvent> OUT* ppEvent);

    virtual cl_err_code UpdateDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject);

private:

    SVMBufferPointerArg(SVMBuffer* pSvmBuf, const void* pArgValue) :
       SVMPointerArg(pSvmBuf), m_pSvmBuf(pSvmBuf), m_szOffset((char*)pArgValue - (char*)pSvmBuf->GetAddr())
       {
           assert(pArgValue >= pSvmBuf->GetAddr());
       }

    SVMBuffer* m_pSvmBuf;
    const size_t m_szOffset;

};

/**
 * This class represents a system pointer as an argument to a kernel
 */
class SVMSystemPointerArg : public SVMPointerArg
{
public:

    PREPARE_SHARED_PTR(SVMSystemPointerArg)

    /**
     * @param pArgValue the system pointer
     * @return a new SVMSystemPointerArg
     */
    static SharedPtr<SVMSystemPointerArg> Allocate(const void* pArgValue)
    {
        return new SVMSystemPointerArg(pArgValue);
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

    virtual void* GetBackingStoreData(const size_t* pszOrigin = NULL) const { return const_cast<void*>(m_ptr); }

    virtual cl_err_code CreateDeviceResource(const SharedPtr<FissionableDevice>& pDevice) { return CL_SUCCESS; }

    virtual cl_err_code GetDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject, SharedPtr<OclEvent> OUT* ppEvent)
    {
        *ppDevObject = new SVMPointerArgDevMemoryObject(this, NULL, 0);
        return CL_SUCCESS;
    }

    virtual cl_err_code UpdateDeviceDescriptor(const SharedPtr<FissionableDevice>& IN pDevice, IOCLDevMemoryObject* OUT* ppDevObject) { return CL_SUCCESS; }

private:

    SVMSystemPointerArg(const void* pArgValue) : SVMPointerArg(NULL), m_ptr(pArgValue) { }

    const void* const m_ptr;
};

}}}
