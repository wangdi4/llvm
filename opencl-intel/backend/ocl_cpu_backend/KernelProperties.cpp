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
    m_hasGlobalSync(false),
    m_DAZ(false),
    m_optWGSize(0),
    m_totalImplSize(0),
    m_barrierBufferSize(0),
    m_privateMemorySize(0),
    m_minGroupSizeFactorial(1),
    m_isVectorizedWithTail(false),
    m_uiSizeT(sizeof(void*)),
    m_bIsBlock(false)
{
    memset(m_reqdWGSize, 0, MAX_WORK_DIM * sizeof(size_t));
    memset(m_hintWGSize, 0, MAX_WORK_DIM * sizeof(size_t));
}

size_t KernelProperties::GetKernelExecutionLength() const
{
    return m_kernelExecutionLength;
}

const char *KernelProperties::GetKernelAttributes() const
{
    return m_kernelAttributes.c_str();
}

unsigned int KernelProperties::GetKernelPackCount() const
{
    return m_optWGSize;
}

const size_t* KernelProperties::GetRequiredWorkGroupSize() const
{
    return m_reqdWGSize[0] ? m_reqdWGSize : NULL;
}

size_t KernelProperties::GetBarrierBufferSize() const
{
    return m_barrierBufferSize;
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

bool KernelProperties::HasGlobalSyncOperation() const
{
    return m_hasGlobalSync;
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

bool KernelProperties::IsBlock() const
{
    return m_bIsBlock;
}

size_t KernelProperties::GetMaxWorkGroupSize(size_t const wgSizeUpperBound,
                                             size_t const wgPrivateMemSizeUpperBound) const {
  assert(GetBarrierBufferSize() <= GetPrivateMemorySize() &&
         "kernel's private memory size must include barrier buffer size");

  // If there is no barrier buffer the private memory is reused by each WI.
  // So it can run WG with max. possbile size.
  size_t ret = wgSizeUpperBound;

  // Return 0 if there is not enough private memory. But take into account what
  // in the vectorized JIT each lane has it's own private memory (not only barrier buffer).
  if(wgPrivateMemSizeUpperBound < GetPrivateMemorySize() * GetMinGroupSizeFactorial()) {
    // Private memory requirements exceed available resources.
    ret = 0;
  } else if(GetBarrierBufferSize() > 0) {
    // TODO: CSSD100016517 workaround:
    //       At the moment GetPrivateMemorySize() returns here the same value as GetBarrierBufferSize().
    //       It is not what it must to do. See the declaration.
    // Each work-item has it's own barrier buffer. But other private memory
    // is shared among all of them (with exception for vectorized JIT, see above).
    size_t const vecSharedPrivateMemory = (GetPrivateMemorySize() - GetBarrierBufferSize()) *
                                           GetMinGroupSizeFactorial();

    // Find out how much memory is left for barrier buffers.
    size_t leftForBuffers = wgPrivateMemSizeUpperBound - vecSharedPrivateMemory;
    // And use this value to compute maximum possible work-group size.
    ret = leftForBuffers / GetBarrierBufferSize();
    // Take into account what max. WG size cannot exceed device limits.
    ret = wgSizeUpperBound < ret ? wgSizeUpperBound : ret;
  }

  // If max. work-group size is greater than the vector width prune it to
  // the closest multiple of the vector width.
  if(ret > GetMinGroupSizeFactorial())
    ret &= ~(GetMinGroupSizeFactorial() - 1);

  return ret;
}

bool KernelProperties::IsNonUniformWGSizeSupported() const
{
    return m_bIsNonUniformWGSizeSupported;
}

}}}
