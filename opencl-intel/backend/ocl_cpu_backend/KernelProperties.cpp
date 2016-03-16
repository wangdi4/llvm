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
#include <iostream>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

KernelJITProperties::KernelJITProperties():
    m_useVTune(false),
    m_vectorSize(1)
{}

KernelJITProperties::~KernelJITProperties()
{}

void KernelJITProperties::Serialize(IOutputStream& ost, SerializationStatus* stats) const
{
    Serializer::SerialPrimitive<bool>(&m_useVTune, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_vectorSize, ost);
}

void KernelJITProperties::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    Serializer::DeserialPrimitive<bool>(&m_useVTune, ist);
    Serializer::DeserialPrimitive<unsigned int>(&m_vectorSize, ist);
}

KernelProperties::KernelProperties():
    m_hasBarrier(false),
    m_hasGlobalSync(false),
    m_DAZ(false),
    m_optWGSize(0),
    m_totalImplSize(0),
    m_barrierBufferSize(0),
    m_privateMemorySize(0),
    m_reqdNumSG(0),
    m_minGroupSizeFactorial(1),
    m_isVectorizedWithTail(false),
    m_uiSizeT(sizeof(void*)),
    m_bIsBlock(false),
    m_canUniteWG(false),
    m_verctorizeOnDimention(0),
    m_debugInfo(false)
{
    memset(m_reqdWGSize, 0, MAX_WORK_DIM * sizeof(size_t));
    memset(m_hintWGSize, 0, MAX_WORK_DIM * sizeof(size_t));
}

void KernelProperties::Serialize(IOutputStream& ost, SerializationStatus* stats) const
{
    // Need to revert dbgPrint flag
    //Serializer::SerialPrimitive<bool>(&m_dbgPrint, ost);
    Serializer::SerialPrimitive<bool>(&m_hasBarrier, ost);
    Serializer::SerialPrimitive<bool>(&m_DAZ, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_optWGSize, ost);
    Serializer::SerialPrimitive(&m_cpuId, ost);

    // using unsigned long long int instead of size_t is because that size_t
    // varies in it's size relating to the platform (32/64 bit)
    unsigned long long int dim = MAX_WORK_DIM;
    Serializer::SerialPrimitive<unsigned long long int>(&dim, ost);

    for(int i = 0; i < MAX_WORK_DIM; ++i)
    {
        unsigned long long int tmp = (unsigned long long int)m_reqdWGSize[i];
        Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    }
    for(int i = 0; i < MAX_WORK_DIM; ++i)
    {
        unsigned long long int tmp = (unsigned long long int)m_hintWGSize[i];
        Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    }

    unsigned long long int tmp = (unsigned long long int)m_totalImplSize;
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    tmp = (unsigned long long int)m_barrierBufferSize;
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    tmp = (unsigned long long int)m_privateMemorySize;
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    tmp = (unsigned long long int)m_reqdNumSG;
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    Serializer::SerialPrimitive<bool>(&m_isVectorizedWithTail, ost);
    tmp = (unsigned long long int)m_kernelExecutionLength;
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_minGroupSizeFactorial, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_uiSizeT, ost);
    Serializer::SerialPrimitive<bool>(&m_bIsNonUniformWGSizeSupported, ost);
    Serializer::SerialPrimitive<bool>(&m_canUniteWG, ost);
    Serializer::SerialPrimitive<unsigned int>(&m_verctorizeOnDimention, ost);
}

void KernelProperties::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    // Need to revert dbgPrint flag
    //Serializer::DeserialPrimitive<bool>(&m_dbgPrint, ist);
    Serializer::DeserialPrimitive<bool>(&m_hasBarrier, ist);
    Serializer::DeserialPrimitive<bool>(&m_DAZ, ist);
    Serializer::DeserialPrimitive<unsigned int>(&m_optWGSize, ist);
    Serializer::DeserialPrimitive(&m_cpuId, ist);

    // using unsigned long long int instead of size_t is because that size_t
    // varies in it's size relating to the platform (32/64 bit)
    unsigned long long int tmp;
    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    assert((MAX_WORK_DIM == tmp) && "WORK DIM dont match!");

    for(int i = 0; i < MAX_WORK_DIM; ++i)
    {
        Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
        m_reqdWGSize[i] = (size_t)tmp;
    }
    for(int i = 0; i < MAX_WORK_DIM; ++i)
    {
        Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
        m_hintWGSize[i] = (size_t)tmp;
    }

    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_totalImplSize = (size_t)tmp;
    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_barrierBufferSize = (size_t)tmp;
    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_privateMemorySize = (size_t)tmp;
    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_reqdNumSG = (size_t)tmp;
    Serializer::DeserialPrimitive<bool>(&m_isVectorizedWithTail, ist);
    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_kernelExecutionLength = tmp;
    unsigned int ui_tmp;
    Serializer::DeserialPrimitive<unsigned int>(&ui_tmp, ist);
    m_minGroupSizeFactorial = ui_tmp;
    Serializer::DeserialPrimitive<unsigned int>(&ui_tmp, ist);
    m_uiSizeT = ui_tmp;
    Serializer::DeserialPrimitive<bool>(&m_bIsNonUniformWGSizeSupported, ist);
    Serializer::DeserialPrimitive<bool>(&m_canUniteWG, ist);
    Serializer::DeserialPrimitive<unsigned int>(&m_verctorizeOnDimention, ist);
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

size_t KernelProperties::GetRequiredNumSubGroups() const
{
    return m_reqdNumSG;
}

size_t KernelProperties::GetMaxNumSubGroups() const
{
    return 1;
}

size_t KernelProperties::GetNumberOfSubGroups(size_t size, const size_t* WGSizes) const
{
    return 1;
}

size_t KernelProperties::GetMaxSubGroupSize(size_t size, const size_t* WGSizes) const
{
    size_t maxSGSize = 1;
    for(size_t i = 0; i < size; ++i)
        maxSGSize *= WGSizes[i];
    return maxSGSize;
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

bool KernelProperties::HasDebugInfo() const
{
    return m_debugInfo;
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
