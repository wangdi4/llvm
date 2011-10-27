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

#include "ProgramContainerMemoryBuffer.h"
#include "BitCodeContainer.h"
#include "llvm/Module.h"


namespace Intel { namespace OpenCL { namespace DeviceBackend {

BitCodeContainer::BitCodeContainer(const cl_prog_container_header* pContainer):
    m_pModule(NULL),
    m_pBuffer(NULL)
{
    assert(pContainer && "Code container pointer must be valid");
    m_pBuffer = ProgramContainerMemoryBuffer::Create( pContainer);
}

BitCodeContainer::~BitCodeContainer()
{
    delete (llvm::Module*)m_pModule; 
    delete m_pBuffer;
}

const cl_prog_container_header* BitCodeContainer::GetCode() const
{
    return m_pBuffer->GetContainerHeader();
}

size_t BitCodeContainer::GetCodeSize() const
{
    return m_pBuffer->GetProgramSize();
}

void   BitCodeContainer::SetModule( void* pModule)
{ 
    m_pModule = pModule; 
}

void*  BitCodeContainer::GetModule() const 
{ 
    return m_pModule; 
}

const cl_llvm_prog_header* BitCodeContainer::GetProgramHeader() const 
{ 
    return m_pBuffer->GetProgHeader();
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
