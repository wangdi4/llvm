#ifndef ICLDevBackendSerializationService_H
#define ICLDevBackendSerializationService_H

#include "ICLDevBackendProgram.h"
#include "cl_device_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

// Defines possible values for serialization behavior
typedef enum _cl_serialization_type
{
	SERIALIZE_TO_DEVICE = 0,
	SERIALIZE_FULL_IMAGE
} cl_serialization_type;

/**
 * This interface class is responsible for allocating\freeing the JIT memory
 */
class ICLDevBackendJITAllocator
{
public:
    /**
     * @effects returns pointer to the aligned chunk of memory with
     *    the required size; NULL in case of failure
     */
    virtual void* AllocateExecutable(size_t size, size_t alignment) = 0;

    /**
     * @effects cleans the memory pointed by the ptr
     */
    virtual void FreeExecutable(void* ptr) = 0;
};

/**
 * This interface class is responsible for the serilization service e.g. turn the
 * given object into blob and restore from blob;
 */
class ICLDevBackendSerializationService
{
public:
    /**
     * Gets the required blob size for serialization
     * 
     * @param serializationType to specify the required serialization type
     * @param pKernel pointer to the kernel you want to serialize
     * @param pSize pointer which will be modified to get the required blob size
     *
     * @returns CL_DEV_SUCCESS and pSize will be modified to the required size in case of sucess
     *  CL_DEV_INVALID_VALUE in case pSize == NULL; CL_DEV_ERROR_FAIL otherwise
     */
    virtual cl_dev_err_code GetSerializationBlobSize(
        cl_serialization_type serializationType,
        const ICLDevBackendProgram_* pProgram, size_t* pSize) const = 0;

    /**
     * Serialize the given kernel into the given buffer (already allocated)
     *
     * @param serializationType to specify the required serialization type
     * @param pKernel pointer to the kernel you want to serialize
     * @param pBlob pointer to already allocated buffer to get the serialization data
     * @param bufferSize the size of the allocated blob
     *
     * @returns 
     *  if bufferSize >= required Blob Size
     *      CL_DEV_SUCCESS and pBlob will get the serialized data in case of sucess
     *      CL_DEV_INVALID_VALUE in case pBlob == NULL; CL_DEV_ERROR_FAIL otherwise
     *  else if the bufferSize don't have the required size CL_DEV_ERROR_FAIL will
     *      be returned
     */
    virtual cl_dev_err_code SerializeProgram(
        cl_serialization_type serializationType, 
        const ICLDevBackendProgram_* pProgram, 
        void* pBlob, size_t blobSize) const = 0;
        
    /**
     * De-Serialize the given buffer to kernel object
     *
     * @param pKernel pointer to the kernel which will be resotored from the buffer
     * @param pBlob pointer to buffer which contains the serialization data
     * @param blobSize the size of the blob
     *
     * @returns 
     *  in case of success CL_DEV_SUCCESS and pKernel will point to the restored kernel
     *  else CL_DEV_ERROR_FAIL will be returned and pKernel will point to NULL
     */
    virtual cl_dev_err_code DeSerializeProgram(
        ICLDevBackendProgram_** ppProgram, 
        void* pBlob,
        size_t blobSize) const = 0;

    /**
     * Releases the Serialization Service
     */
    virtual void Release() = 0;
};

}}} // namespace

#endif // ICLDevBackendSerializationService_H
