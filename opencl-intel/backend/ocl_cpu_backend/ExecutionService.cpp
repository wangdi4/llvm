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

File Name:  ExecutionService.cpp

\*****************************************************************************/

#include "ExecutionService.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "Binary.h"
#include "exceptions.h"

#ifdef OCL_DEV_BACKEND_PLUGINS 
#include "plugin_manager.h"
#endif

namespace Intel { namespace OpenCL { namespace DeviceBackend {

ExecutionService::ExecutionService()
{}

cl_dev_err_code ExecutionService::CreateBinary(
        const ICLDevBackendKernel_* pKernel, 
        void* pContext,
        size_t contextSize, 
        const cl_work_description_type* pWorkDescription, 
        ICLDevBackendBinary_** ppBinary) const
{
    try
    {
        const Kernel* pKernelImpl = static_cast<const Kernel*>(pKernel);
        const KernelProperties* pKernelProps = static_cast<const KernelProperties*>(pKernel->GetKernelProporties());
        cl_work_description_type workSizes;

        pKernelImpl->CreateWorkDescription( pWorkDescription, workSizes);
        
        *ppBinary = CreateBinaryImp(pKernelImpl,
                                    pKernelProps,
                                    &workSizes,
                                    pContext,
                                    contextSize);
            
#ifdef OCL_DEV_BACKEND_PLUGINS  
        // Notify the plugin manager
        PluginManager::Instance().OnCreateBinary( pKernel, 
                                                  pWorkDescription,
                                                  contextSize,
                                                  pContext);
#endif

        return CL_DEV_SUCCESS;
    }
    catch( Exceptions::DeviceBackendExceptionBase& e )
    {
        return e.GetErrorCode();
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
}

cl_dev_err_code ExecutionService::CreateExecutable(
        ICLDevBackendBinary_* pBinary, 
        void** pExecutionMemoryResources, 
        unsigned int resourcesCount, 
        ICLDevBackendExecutable_** ppExecutable) const
{
    assert(false && "NotImplemented");
    return CL_DEV_NOT_SUPPORTED;
}
        
size_t ExecutionService::GetTargetMachineDescriptionSize() const
{
    assert(false && "NotImplemented");
    return 0;    
}

cl_dev_err_code ExecutionService::GetTargetMachineDescription(
        void* pTargetDescription, 
        size_t descriptionSize) const

{
    assert(false && "NotImplemented");
    return CL_DEV_NOT_SUPPORTED;
}

void ExecutionService::Release()
{
    delete this;
}

}}}