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

File Name:  ProgramContainerMemoryBuffer.cpp

\*****************************************************************************/

#include "ProgramContainerMemoryBuffer.h"
#include "llvm/Support/MathExtras.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

ProgramContainerMemoryBuffer::ProgramContainerMemoryBuffer( const cl_prog_container_header* pHeader)
{
    m_pContainerHeader = pHeader;
    m_pProgHeader = (cl_llvm_prog_header*)((const char* )pHeader + sizeof(cl_prog_container_header));

    const char* pIR = (const char*)pHeader+sizeof(cl_prog_container_header)+sizeof(cl_llvm_prog_header);
    size_t size = pHeader->container_size - sizeof(cl_llvm_prog_header);
    
    llvm::MemoryBuffer::init( pIR, pIR + size);
}

ProgramContainerMemoryBuffer* ProgramContainerMemoryBuffer::Create( const cl_prog_container_header* pHeader )
{
    // Allocate space for the ProgramContainerMemoryBuffer and the data. It is important
    // that MemoryBuffer and data are aligned so PointerIntPair works with them.
    size_t headersSize = sizeof(cl_prog_container_header) + sizeof(cl_llvm_prog_header);
    size_t alignedSize = llvm::RoundUpToAlignment(sizeof(ProgramContainerMemoryBuffer) + 
                                                  headersSize, sizeof(void*)); 
    size_t bufferSize  = pHeader->container_size - sizeof(cl_llvm_prog_header);
    size_t realSize    = alignedSize + bufferSize + 1; // 1 is for terminating zero => MemoryBuffer requirement
    char *pMem = static_cast<char*>(operator new(realSize));
    // last byte of the buffer must be zero - MemoryBuffer requirement
    pMem[realSize -1] = 0;

    // The buffer begins after the headers and must be aligned (the IR buffer, not the headers).
    char *pBuf = pMem + alignedSize - headersSize;
    memcpy(pBuf, pHeader, headersSize + bufferSize);

    return new(pMem) ProgramContainerMemoryBuffer((const cl_prog_container_header*)pBuf);
}

}}} // namespace
