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

File Name:  IAbstractBackendFactory.h

\*****************************************************************************/
#pragma once

#include <vector>
#include <string>
#include "cl_dev_backend_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class Program;
class Kernel;
class KernelProperties;
class KernelJITProperties;
class IKernelJITContainer;
class Binary;
class Executable;

/**
 * This interface is used to generate the suitable objects for CPU\MIC devices
 */
class IAbstractBackendFactory
{
public:
    virtual Program* CreateProgram() = 0;
    virtual Kernel* CreateKernel()   = 0;

    virtual Kernel* CreateKernel(
        const std::string& name,
        const std::vector<cl_kernel_argument>& args,
        KernelProperties* pProps) = 0;

    virtual KernelProperties* CreateKernelProperties() = 0;
    virtual KernelJITProperties* CreateKernelJITProperties() = 0;

    virtual Binary* CreateBinary(
        const KernelProperties* pKernelProperties,
        const std::vector<cl_kernel_argument>& args,
        const cl_work_description_type* pWorkInfo,
        const IKernelJITContainer* pScalarJIT,
        const IKernelJITContainer* pVectorJIT,
        char* IN pArgsBuffer, 
        size_t IN ArgBuffSize) = 0;

    virtual Executable* CreateExecutable(const Binary* pBinary) = 0;
};

}}} // namespace
