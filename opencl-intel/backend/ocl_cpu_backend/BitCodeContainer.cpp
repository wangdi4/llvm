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

File Name:  BitCodeContainer.cpp

\*****************************************************************************/

#include "BitCodeContainer.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/IR/LLVMContext.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {
BitCodeContainer::BitCodeContainer(const void *pBinary, size_t uiBinarySize, const char* name):
    m_pModule(NULL)
{
    assert(pBinary && "Code container pointer must be valid");
    m_pBuffer = llvm::MemoryBuffer::getMemBufferCopy(llvm::StringRef((const char*)pBinary, uiBinarySize), name);
}

BitCodeContainer::~BitCodeContainer()
{
    if(m_pModule) 
    {
        llvm::Module* pModule = static_cast<llvm::Module*>(m_pModule);
        llvm::LLVMContext& context = pModule->getContext();
        delete pModule;

        // Unused metadata nodes are left alive during deletion of Module
        // Module owns Functions which are often used in metadata
        // during function destruction MDNodes referring to the function are
        // marked as non-unique and are placed to Nonuniqued nodes container in LLVMContext
        // LLVMContext will delete Nonuniqued only during its own deletion at clReleaseContext
        // As a result if we have multiple calls to clBuildProgram on the same context e.g. in loop
        // then number of unused MDNodes grows and we have memory leak reported
        // cleanup() was added to LLVMContext and is called to find and free memory by unused Metadata nodes
        // see ticket CSSD100018078 for details or contact Oleg
        // oleg: clean up unused MDNodes in LLVMContext
        context.cleanup();
    }

    delete m_pBuffer;
}

const void* BitCodeContainer::GetCode() const
{
    return m_pBuffer->getBufferStart();
}

size_t BitCodeContainer::GetCodeSize() const
{
    return m_pBuffer->getBufferSize();
}

void   BitCodeContainer::SetModule( void* pModule)
{
    m_pModule = pModule;
}

void*  BitCodeContainer::GetModule() const
{
    return m_pModule;
}

void* BitCodeContainer::GetMemoryBuffer() const
{
    return m_pBuffer;
}

void BitCodeContainer::Release()
{
    delete this;
}
}}} // namespace