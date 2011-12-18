#ifndef ICLDevBackendExecutionService_H
#define ICLDevBackendExecutionService_H

#include "ICLDevBackendKernel.h"
#include "ICLDevBackendBinary.h"
#include "ICLDevBackendExecutable.h"
#include "cl_device_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 * This interface class is responsible for the execution service for a given kernel 
 * it also contains machine detection unit to better know the target specifications
 */
class ICLDevBackendExecutionService
{
public:
    virtual ~ICLDevBackendExecutionService() {}

    /**
     * Creates binary object from the given kernel code and the context, the generated binary
     * will be binded for the given work description.
     *
     * @param pKenrel pointer to the kernel that you want to execute
     * @param pContext context which contains the argument values as required for the 
     *  given kernel
     * @param contextSize context size in bytes
     * @param pWorkDescription description of the OCL task
     * @param ppBinary pointer which will be modified to point to the created binary
     *
     * @returns in case of success CL_DEV_SUCCESS and ppBinary will point to the created binary
     *  otherwise ppBinary will be NULL and will return:
     *  CL_DEV_OUT_OF_MEMORY if there's no sufficient memory 
     *  CL_DEV_INVALID_TARGET if the given kernel compiled for another target and cannot
     *      executed on this target (target mismatch)
     *  CL_DEV_INVALID_CONTEXT if the given context do not match the kernel requirements
     *  CL_DEV_ERROR_FAIL in any other error
     */
    virtual cl_dev_err_code CreateBinary(
        const ICLDevBackendKernel_* pKernel, 
        void* pContext,
        size_t contextSize, 
        const cl_work_description_type* pWorkDescription, 
        ICLDevBackendBinary_** ppBinary) const = 0;

    /**
     * Creates Executable Object, which will be used by specific execution threads
     *
     * @param pBinary pointer to binary object to create executable from.
     * @param ppExecutionMemoryResources buffer which should contain pointers to the allocated
     *  memory buffers as requested in the ExecutionMemoryResourcesDescription (in the same
     *  order)
     * @param resourcesCount contains how many entries in the passed buffer
     * @param ppExecutable pointer which will point to the created Executable object
     *
     * @returns 
     *  CL_DEV_OUT_OF_MEMORY and ppExecutable = NULL in case of memory failure
     *  CL_DEV_SUCCESS and ppExecutable will point the created obj in case of success;
     *  if the resourcesCount don't match the required ExecutionMemoryResourcesDescription
     *  or ppExecutionMemoryResources is not valid the behavior is undefined
     *
     *  NOTE: the execution memory resources should be freed by the user, and should be
     *      released after releasing all the executables which rely on them
     */
    virtual cl_dev_err_code CreateExecutable(
        ICLDevBackendBinary_* pBinary, 
        void** ppExecutionMemoryResources, 
        unsigned int resourcesCount, 
        ICLDevBackendExecutable_** ppExecutable) const = 0;
		
    /**
     * @returns the target machine description size in bytes
     */
    virtual size_t GetTargetMachineDescriptionSize() const = 0;

    /**
     * Gets the target machine description in the already allocated buffer
     *
     * @param pTargetDescription pointer to the allocated buffer to be filled with the
     *  target machine description
     * @param descriptionSize the size of the allocated buffer
     *
     * @returns CL_DEV_SUCCESS and the pTargetDescription will be filled with the
     *  description in case of success; otherwise:
     *  CL_DEV_INVALID_VALUE in case pTargetDescription == NULL 
     *  CL_DEV_ERROR_FAIL in any other error
     */
    virtual cl_dev_err_code GetTargetMachineDescription(
        void* pTargetDescription, 
        size_t descriptionSize) const = 0;

    /**
     * Releases the Execution Service
     */
    virtual void Release() = 0;
};

}}} // namespace

#endif // ICLDevBackendExecutionService_H
