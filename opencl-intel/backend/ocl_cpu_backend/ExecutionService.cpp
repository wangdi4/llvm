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

ExecutionService::ExecutionService(const ICLDevBackendOptions* pOptions)
    : m_pPrinter(NULL), m_pDeviceCommandManager(NULL)
{
    void *pPrinter = NULL;
    size_t size = sizeof(pPrinter);
    if(NULL != pOptions && 
       pOptions->GetValue(CL_DEV_BACKEND_OPTION_BUFFER_PRINTER, &pPrinter, &size))
    {
        m_pPrinter = (ICLDevBackendBufferPrinter*)pPrinter;
    }

    // obtain Device Command Manager
    assert(pOptions && "pOptions are NULL");
    void *pDCM;
    size_t pDCM_Size = sizeof(pDCM);
    if(NULL != pOptions && 
       pOptions->GetValue(CL_DEV_BACKEND_OPTION_IDEVICE_COMMAND_MANAGER, 
                          &pDCM, &pDCM_Size))
    {
      m_pDeviceCommandManager = static_cast<IDeviceCommandManager*>(pDCM);
    }
}

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
        
        const RuntimeServiceSharedPtr& rs = pKernelImpl->GetRuntimeService();
        // if RuntimeService is initilized then pass reference to Mapper
        // otherwise pass NULL
        const IBlockToKernelMapper * pMapper = rs.get() ? rs->GetBlockToKernelMapper() : NULL;
        
        *ppBinary = m_pBackendFactory->CreateBinary(
                                        m_pPrinter,
                                        m_pDeviceCommandManager,
                                        pMapper,
                                        pKernelProps,
                                        *pKernelImpl->GetKernelParamsVector(),
                                        &workSizes,
                                        pKernelImpl->GetKernelJIT(0),
                                        pKernelImpl->GetKernelJITCount() > 1 ? pKernelImpl->GetKernelJIT(1) : NULL,
                                        (char*)pContext,
                                        contextSize);
            
#ifdef OCL_DEV_BACKEND_PLUGINS  
        // Notify the plugin manager
        m_pluginManager.OnCreateBinary( pKernel, 
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
