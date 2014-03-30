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

File Name:  CPUDeviceBackendFactory.h

\*****************************************************************************/
#pragma once
#include "cl_dev_backend_api.h"
#include "IAbstractBackendFactory.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 * This class is used to generate the suitable objects for CPU device
 */
class CPUDeviceBackendFactory : public IAbstractBackendFactory
{
protected:
    CPUDeviceBackendFactory() { }; 
    virtual ~CPUDeviceBackendFactory() { };

public:
    static void Init();
    static void Terminate();

    static CPUDeviceBackendFactory* GetInstance();

    virtual Program* CreateProgram();
    virtual Kernel* CreateKernel();

    virtual Kernel* CreateKernel(
        const std::string& name,
        const std::vector<cl_kernel_argument>& args,
        const std::vector<unsigned int>& memArgs,
        KernelProperties* pProps);

    virtual KernelProperties* CreateKernelProperties();
    virtual KernelJITProperties* CreateKernelJITProperties();
    virtual IKernelJITContainer* CreateKernelJITContainer();
 
protected:
    static CPUDeviceBackendFactory* s_pInstance;
};

}}} // namespace
