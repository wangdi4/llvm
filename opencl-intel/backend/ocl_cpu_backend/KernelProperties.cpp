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

File Name:  KernelProperties.cpp

\*****************************************************************************/

#include "KernelProperties.h"
#include "exceptions.h"
#include <string.h>
#include <algorithm>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

KernelJITProperties::KernelJITProperties():
    m_useVTune(false),
    m_vectorSize(1)
{}

KernelProperties::KernelProperties():
    m_hasBarrier(false),
    m_DAZ(false),
    m_optWGSize(0),
    m_totalImplSize(0),
    m_privateMemorySize(0),
    m_minGroupSizeFactorial(1)
{
    memset(m_reqdWGSize, 0, MAX_WORK_DIM*sizeof(size_t));
    memset(m_hintWGSize, 0, MAX_WORK_DIM*sizeof(size_t));
}

size_t KernelProperties::GetKernelExecutionLength() const
{
    return m_kernelExecutionLength;
}

unsigned int KernelProperties::GetKernelPackCount() const
{
    return m_optWGSize;
}

const size_t* KernelProperties::GetRequiredWorkGroupSize() const
{
    return m_reqdWGSize[0] ? m_reqdWGSize : NULL;
}

size_t KernelProperties::GetPrivateMemorySize() const
{
    return m_privateMemorySize;
}

unsigned int KernelProperties::GetMinGroupSizeFactorial() const
{
    return m_minGroupSizeFactorial;
}

size_t KernelProperties::GetImplicitLocalMemoryBufferSize() const
{
    return m_totalImplSize;
}

bool KernelProperties::HasPrintOperation() const
{
    return false;
}

bool KernelProperties::HasBarrierOperation() const
{
    return m_hasBarrier;
}

bool KernelProperties::HasKernelCallOperation() const
{
    return false;
}

void KernelProperties::SetReqdWGSize(const size_t* psize )
{
    std::copy(psize, psize + MAX_WORK_DIM, m_reqdWGSize);
}

void KernelProperties::SetHintWGSize(const size_t* psize )
{
    std::copy(psize, psize + MAX_WORK_DIM, m_hintWGSize);
}

}}}
