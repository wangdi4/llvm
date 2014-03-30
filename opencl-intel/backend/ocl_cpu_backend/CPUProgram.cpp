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

File Name:  CPUProgram.cpp

\*****************************************************************************/
#define NOMINMAX

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "CPUProgram.h"
#include "BitCodeContainer.h"
#include "Kernel.h"


namespace Intel { namespace OpenCL { namespace DeviceBackend {

void CPUProgram::ReleaseExecutionEngine()
{
    // We have to remove the built-ins module from execEngine
    // since this module is owned by compiler
    if (m_pBIModule) 
      m_pExecutionEngine->removeModule((llvm::Module*)m_pBIModule);
    if (m_pCodeContainer->GetModule())
      m_pExecutionEngine->removeModule((llvm::Module*)m_pCodeContainer->GetModule());
    delete m_pExecutionEngine;
}

CPUProgram::~CPUProgram()
{
    // Freeing the execution engine is sufficient to cleanup all memory in
    // MCJIT
    ReleaseExecutionEngine();
}

void* CPUProgram::GetPointerToFunction(llvm::Function* F) {
    return m_pExecutionEngine->getPointerToFunction(F);
}

}}}
