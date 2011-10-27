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

File Name:  MICBinary.cpp

\*****************************************************************************/

#include "MICBinary.h"
#include "Executable.h"
#include "MICExecutable.h" 

namespace Intel { namespace OpenCL { namespace DeviceBackend {

MICBinary::MICBinary( const KernelProperties* pKernelProperties,
            const std::vector<cl_kernel_argument>& args,
            const cl_work_description_type* pWorkInfo,
            const IKernelJITContainer* pScalarJIT,
            const IKernelJITContainer* pVectorJIT,
            char* IN pArgsBuffer, 
            size_t IN ArgBuffSize):
    Binary(pKernelProperties, args, 
        pWorkInfo, 
        pScalarJIT, 
        pVectorJIT, 
        pArgsBuffer, 
        ArgBuffSize)
{}

Executable* MICBinary::CreateExecutableImp(Binary* pBinary) const
{
    return new MICExecutable(this);
}

}}}
