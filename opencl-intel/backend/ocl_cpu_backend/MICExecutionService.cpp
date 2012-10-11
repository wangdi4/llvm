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

File Name:  MICExecutionService.cpp

\*****************************************************************************/

#include "TargetArch.h"
#include "DynamicLibraryLoader.h"
#include "MICExecutionService.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "Binary.h"
#include "Executable.h"
#include "MICDeviceBackendFactory.h"
#include "MICSerializationService.h"
#include "MICDetect.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

MICExecutionService::MICExecutionService(const ICLDevBackendOptions* pOptions, 
    const Intel::CPUId& cpuId) : ExecutionService(pOptions)
{
    m_pBackendFactory = MICDeviceBackendFactory::GetInstance(); 
    m_TargetDescription.SetCPUId(cpuId);
    
    m_Loader.SetCPUId(cpuId);
   
    m_Loader.Load();

    std::map<std::string, unsigned long long int> functionsTable;
    m_Loader.GetLibraryFunctions(functionsTable);
    m_TargetDescription.SetFunctionsTable(functionsTable);
}

size_t MICExecutionService::GetTargetMachineDescriptionSize() const
{
    size_t size = 0;
    MICSerializationService serializationService(NULL);

    serializationService.GetTargetDescriptionBlobSize(&m_TargetDescription, &size);
    return size;
}

cl_dev_err_code MICExecutionService::GetTargetMachineDescription(
    void* pTargetDescription, 
    size_t descriptionSize) const
{
    assert(pTargetDescription && "Target Description buffer is null");
    assert((descriptionSize >= GetTargetMachineDescriptionSize()) && "Too small buffer size");
    MICSerializationService serializationService(NULL);

    serializationService.SerializeTargetDescription(&m_TargetDescription, pTargetDescription, 
        descriptionSize);

    return CL_DEV_SUCCESS;
}

cl_dev_err_code MICExecutionService::CreateExecutable(
        ICLDevBackendBinary_* pBinary, 
        void** pExecutionMemoryResources, 
        unsigned int resourcesCount, 
        ICLDevBackendExecutable_** ppExecutable) const
{
	ICLDevBackendExecutable_ *pExecutable = m_pBackendFactory->CreateExecutable(static_cast<Binary*>(pBinary));
	if ( NULL == pExecutable )
	{
		return CL_DEV_OUT_OF_MEMORY;
	}
	
	*ppExecutable = pExecutable;
	return CL_DEV_SUCCESS;
}

}}}
