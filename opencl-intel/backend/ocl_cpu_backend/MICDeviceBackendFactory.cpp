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

File Name:  MICDeviceBackendFactory.cpp

\*****************************************************************************/

#include "MICDeviceBackendFactory.h"
#include "MICProgram.h"
#include "MICKernel.h"
#include "MICKernelProperties.h"
#include "MICExecutable.h"
#include "Binary.h"
#include <assert.h>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

MICDeviceBackendFactory* MICDeviceBackendFactory::s_pInstance = NULL;

void MICDeviceBackendFactory::Init()
{
    assert(!s_pInstance);
    s_pInstance = new MICDeviceBackendFactory();
}

void MICDeviceBackendFactory::Terminate()
{
    if( NULL != s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = NULL;
    }
}

MICDeviceBackendFactory* MICDeviceBackendFactory::GetInstance()
{
    assert(s_pInstance);
    return s_pInstance;
}

Program* MICDeviceBackendFactory::CreateProgram()
{
    return new MICProgram();
}

Kernel* MICDeviceBackendFactory::CreateKernel()
{
    return new MICKernel();
}

Kernel* MICDeviceBackendFactory::CreateKernel(
    const std::string& name,
    const std::vector<cl_kernel_argument>& args,
    KernelProperties* pProps)
{
    return new MICKernel(name, args, pProps);
}

KernelProperties* MICDeviceBackendFactory::CreateKernelProperties()
{
    return new MICKernelProperties();
}

KernelJITProperties* MICDeviceBackendFactory::CreateKernelJITProperties()
{
    return new MICKernelJITProperties();
}

Binary* MICDeviceBackendFactory::CreateBinary( 
    const KernelProperties* pKernelProperties,
    const std::vector<cl_kernel_argument>& args,
    const cl_work_description_type* pWorkInfo,
    const IKernelJITContainer* pScalarJIT,
    const IKernelJITContainer* pVectorJIT,
    char* IN pArgsBuffer, 
    size_t IN ArgBuffSize)
{
    return new Binary(this,
                    pKernelProperties,
                    args,
                    pWorkInfo,
                    pScalarJIT,
                    pVectorJIT,
                    pArgsBuffer, 
                    ArgBuffSize);
}

Executable* MICDeviceBackendFactory::CreateExecutable(const Binary* pBinary)
{
    return new MICExecutable(pBinary);
}


}}} // namespace
