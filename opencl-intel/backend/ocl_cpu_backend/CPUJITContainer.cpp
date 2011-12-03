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

File Name:  CPUJITContainer.cpp

\*****************************************************************************/
#include "CPUJITContainer.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

CPUJITContainer::CPUJITContainer(const void* pFuncCode,
                                 llvm::Function* pFunction,
                                 llvm::Module* pModule,
                                 CPUCompiler* pCompiler,
                                 KernelJITProperties* pProps):
    m_pFuncCode(pFuncCode),
    m_pFunction(pFunction),
    m_pModule(pModule),
    m_pCompiler(pCompiler),
    m_pProps(pProps) // get ownership of the pProps pointer
{}

CPUJITContainer::~CPUJITContainer()
{
    assert(m_pCompiler && "ExecEngine must be initialized");
    m_pCompiler->freeMachineCodeForFunction(m_pFunction);
    delete m_pProps;
}

}}} // namespace

