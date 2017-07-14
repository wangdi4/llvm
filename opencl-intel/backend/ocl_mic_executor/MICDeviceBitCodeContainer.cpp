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

File Name:  DeviceBitCodeContainer.cpp

\*****************************************************************************/

#include "BitCodeContainer.h"

/**
 * NOTE: This is an empty version of BitCodeContainer on the Device this unit
 * shouldn't be executed on the Device, it's just in order to compile Program
 * on the Device without any dependency with llvm
 */

namespace Intel { namespace OpenCL { namespace DeviceBackend {
BitCodeContainer::BitCodeContainer(const void *pBinary, size_t uiBinarySize, const char* name ):
    m_pModule(nullptr),
    m_pBuffer(nullptr)
{
}

BitCodeContainer::~BitCodeContainer()
{
}

const void* BitCodeContainer::GetCode() const
{
    return nullptr;
}

size_t BitCodeContainer::GetCodeSize() const
{
    return 0;
}

void   BitCodeContainer::SetModule( void* pModule)
{
    m_pModule = pModule;
}

void*  BitCodeContainer::GetModule() const
{
    return nullptr;
}

void* BitCodeContainer::GetMemoryBuffer() const
{
    return nullptr;
}

void BitCodeContainer::Release()
{
    delete this;
}
}}} // namespace
