/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ExecutionService.h

\*****************************************************************************/
#pragma once

#include "cl_dev_backend_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class Kernel;
class KernelProperties;
class Binary;

class ExecutionService: public ICLDevBackendExecutionService
{
public:
        ExecutionService();
    /**
     * Creates binary object from the given kernel code and the context, the generated binary
     * will be binded for the given work description.
     *
     * @param pKenrel pointer to the kernel that you want to execute
     * @param pContext context which contains the argument values as required for the 
     *  given kernel
     * @param contextSize context size in bytes
     * @param pWorkDescription description of the OCL task
     * @param pBinary will be modified to contain the created binary
     *
     * @returns in case of success CL_DEV_SUCCESS and pBinary will point to the created binary
     *  otherwise pBinary will be NULL and will return:
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
        ICLDevBackendBinary_** ppBinary) const;
    
    /**
     * Creates Executable Object, which will be used by specific execution threads
     *
     * @param pBinary pointer to binary object to create executable from.
     * @param pExecutionMemoryResources buffer which should contain pointers to the allocated
     *  memory buffers as requested in the ExecutionMemoryResourcesDescription (in the same
     *  order)
     * @param resourcesCount contains how many entries in the passed buffer
     * @param pExecutable will point to the created Executable object
     *
     * @returns 
     *  CL_DEV_OUT_OF_MEMORY and pExecutable = NULL in case of memory failure
     *  CL_DEV_SUCCESS and pExecutable will point the created obj in case of success;
     *  if the resourcesCount don't match the required ExecutionMemoryResourcesDescription
     *  or pExecutionMemoryResources is not valid the behavior is undefined
     *
     *  NOTE: the execution memory resources should be freed by the user, and should be
     *      released after releasing all the executables which rely on them
     */
    virtual cl_dev_err_code CreateExecutable(
        ICLDevBackendBinary_* pBinary, 
        void** pExecutionMemoryResources, 
        unsigned int resourcesCount, 
        ICLDevBackendExecutable_** ppExecutable) const;
        
    /**
     * @returns the target machine description size in bytes
     */
    virtual size_t GetTargetMachineDescriptionSize() const;
    
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
        size_t descriptionSize) const;
    
    /**
     * Releases the Execution Service
     */
    virtual void Release();
    
protected:
    virtual Binary* CreateBinaryImp(const Kernel* pKernelImpl,
        const KernelProperties* pKernelProps,
        cl_work_description_type* workSizes,
        void* pContext,
        size_t contextSize) const = 0;
        
};

}}}