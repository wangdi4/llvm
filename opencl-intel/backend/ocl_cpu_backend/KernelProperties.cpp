// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "KernelProperties.h"
#include "cpu_dev_limits.h"
#include "exceptions.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <iostream>
#include <string.h>
#include <unordered_set>

using namespace llvm;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

KernelJITProperties::KernelJITProperties()
    : m_useVTune(false), m_vectorSize(1) {}

KernelJITProperties::~KernelJITProperties()
{}

void KernelJITProperties::Serialize(IOutputStream &ost,
                                    SerializationStatus * /*stats*/) const {
  Serializer::SerialPrimitive<bool>(&m_useVTune, ost);
  Serializer::SerialPrimitive<unsigned int>(&m_vectorSize, ost);
}

void KernelJITProperties::Deserialize(IInputStream &ist,
                                      SerializationStatus * /*stats*/) {
  Serializer::DeserialPrimitive<bool>(&m_useVTune, ist);
  Serializer::DeserialPrimitive<unsigned int>(&m_vectorSize, ist);
}

KernelProperties::KernelProperties()
    : m_hasNoBarrierPath(false), m_hasMatrixCall(false), m_hasGlobalSync(false),
      m_DAZ(false), m_optWGSize(0),
      m_totalImplSize(0), m_barrierBufferSize(0), m_privateMemorySize(0),
      m_maxPrivateMemorySize(0), m_reqdNumSG(0), m_kernelExecutionLength(0),
      m_vectorizationWidth(1), m_minGroupSizeFactorial(1),
      m_isVectorizedWithTail(false), m_uiSizeT(sizeof(void *)),
      m_bIsBlock(false), m_bIsAutorun(false), m_bNeedSerializeWGs(false),
      m_bIsTask(false), m_bCanUseGlobalWorkOffset(true),
      m_bIsNonUniformWGSizeSupported(false), m_canUniteWG(false),
      m_verctorizeOnDimention(0), m_debugInfo(false) {
  memset(m_reqdWGSize, 0, MAX_WORK_DIM * sizeof(size_t));
  memset(m_hintWGSize, 0, MAX_WORK_DIM * sizeof(size_t));
}

void KernelProperties::Serialize(IOutputStream &ost,
                                 SerializationStatus * /*stats*/) const {
  // Need to revert dbgPrint flag
  // Serializer::SerialPrimitive<bool>(&m_dbgPrint, ost);
  Serializer::SerialPrimitive<bool>(&m_bIsBlock, ost);
  Serializer::SerialPrimitive<bool>(&m_hasNoBarrierPath, ost);
  if (OCL_CACHED_BINARY_VERSION >= 15)
    Serializer::SerialPrimitive<bool>(&m_hasMatrixCall, ost);
  Serializer::SerialPrimitive<bool>(&m_hasGlobalSync, ost);
  if (OCL_CACHED_BINARY_VERSION < 16) {
    bool DeprecatedUseNativeSubgroups = true;
    Serializer::SerialPrimitive<bool>(&DeprecatedUseNativeSubgroups, ost);
  }
  Serializer::SerialPrimitive<bool>(&m_DAZ, ost);
  Serializer::SerialPrimitive<unsigned int>(&m_optWGSize, ost);
  Serializer::SerialPrimitive(&m_cpuId, ost);

  // using unsigned long long int instead of size_t is because that size_t
  // varies in it's size relating to the platform (32/64 bit)
  unsigned long long int dim = MAX_WORK_DIM;
  Serializer::SerialPrimitive<unsigned long long int>(&dim, ost);

  for (int i = 0; i < MAX_WORK_DIM; ++i) {
    unsigned long long int tmp = (unsigned long long int)m_reqdWGSize[i];
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  }
  for (int i = 0; i < MAX_WORK_DIM; ++i) {
    unsigned long long int tmp = (unsigned long long int)m_hintWGSize[i];
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  }

  unsigned long long int tmp = (unsigned long long int)m_totalImplSize;
  Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  tmp = (unsigned long long int)m_barrierBufferSize;
  Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  tmp = (unsigned long long int)m_privateMemorySize;
  Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  // There is no need to serialize m_maxPrivateMemorySize which is a runtime
  // config. Its serialization is kept for backward compatibility.
  tmp = (unsigned long long int)m_maxPrivateMemorySize;
  Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  tmp = (unsigned long long int)m_reqdNumSG;
  Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  Serializer::SerialPrimitive<bool>(&m_isVectorizedWithTail, ost);
  tmp = (unsigned long long int)m_kernelExecutionLength;
  Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  tmp = (unsigned long long int)m_vectorizationWidth;
  Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  tmp = (unsigned long long int)m_reqdSubGroupSize;
  Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  Serializer::SerialString(m_kernelAttributes, ost);
  Serializer::SerialPrimitive<unsigned int>(&m_minGroupSizeFactorial, ost);
  Serializer::SerialPrimitive<unsigned int>(&m_uiSizeT, ost);
  Serializer::SerialPrimitive<bool>(&m_bIsNonUniformWGSizeSupported, ost);
  Serializer::SerialPrimitive<bool>(&m_canUniteWG, ost);
  Serializer::SerialPrimitive<unsigned int>(&m_verctorizeOnDimention, ost);
  Serializer::SerialPrimitive<bool>(&m_debugInfo, ost);
  Serializer::SerialPrimitive<bool>(&m_bIsAutorun, ost);
  Serializer::SerialPrimitive<bool>(&m_bNeedSerializeWGs, ost);
  Serializer::SerialPrimitive<bool>(&m_bIsTask, ost);
  Serializer::SerialPrimitive<bool>(&m_bCanUseGlobalWorkOffset, ost);
  Serializer::SerialPrimitive<DeviceMode>(&m_targetDevice, ost);
  tmp = (unsigned long long int)m_cpuMaxWGSize;
  Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
}

void KernelProperties::Deserialize(IInputStream &ist,
                                   SerializationStatus *stats) {
  // Need to revert dbgPrint flag
  // Serializer::DeserialPrimitive<bool>(&m_dbgPrint, ist);
  Serializer::DeserialPrimitive<bool>(&m_bIsBlock, ist);
  Serializer::DeserialPrimitive<bool>(&m_hasNoBarrierPath, ist);
  if (stats->m_binaryVersion >= 15)
    Serializer::DeserialPrimitive<bool>(&m_hasMatrixCall, ist);
  Serializer::DeserialPrimitive<bool>(&m_hasGlobalSync, ist);
  if (stats->m_binaryVersion < 16) {
    bool DeprecatedUseNativeSubgroups = true;
    Serializer::DeserialPrimitive<bool>(&DeprecatedUseNativeSubgroups, ist);
  }
  Serializer::DeserialPrimitive<bool>(&m_DAZ, ist);
  Serializer::DeserialPrimitive<unsigned int>(&m_optWGSize, ist);
  Serializer::DeserialPrimitive(&m_cpuId, ist);

  // using unsigned long long int instead of size_t is because that size_t
  // varies in it's size relating to the platform (32/64 bit)
  unsigned long long int tmp;
  Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
  assert((MAX_WORK_DIM == tmp) && "WORK DIM dont match!");

  for (int i = 0; i < MAX_WORK_DIM; ++i) {
    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_reqdWGSize[i] = (size_t)tmp;
  }
  for (int i = 0; i < MAX_WORK_DIM; ++i) {
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
  m_maxPrivateMemorySize = (size_t)tmp;
  Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
  m_reqdNumSG = (size_t)tmp;
  Serializer::DeserialPrimitive<bool>(&m_isVectorizedWithTail, ist);
  Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
  m_kernelExecutionLength = (size_t)tmp;
  Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
  m_vectorizationWidth = (size_t)tmp;
  // There was a bug in the old compiler (OCL_CACHED_BINARY_VERSION <= 10, see
  // commit 0db0bd83cf658d1dc3e26637b6adeb7a6ed2c098) that the vectorization
  // width was wrongly set as 0 if the kernel is not vectorized nor
  // subgroup-emulated. We need to materialize the unexpected zero vectorization
  // width to 1 to keep backwards compatibility.
  if (stats->m_binaryVersion <= 10 && m_vectorizationWidth == 0)
    m_vectorizationWidth = 1;
  Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
  m_reqdSubGroupSize = (size_t)tmp;
  Serializer::DeserialString(m_kernelAttributes, ist);
  unsigned int ui_tmp;
  Serializer::DeserialPrimitive<unsigned int>(&ui_tmp, ist);
  m_minGroupSizeFactorial = ui_tmp;
  Serializer::DeserialPrimitive<unsigned int>(&ui_tmp, ist);
  m_uiSizeT = ui_tmp;
  Serializer::DeserialPrimitive<bool>(&m_bIsNonUniformWGSizeSupported, ist);
  Serializer::DeserialPrimitive<bool>(&m_canUniteWG, ist);
  Serializer::DeserialPrimitive<unsigned int>(&m_verctorizeOnDimention, ist);
  Serializer::DeserialPrimitive<bool>(&m_debugInfo, ist);
  Serializer::DeserialPrimitive<bool>(&m_bIsAutorun, ist);
  Serializer::DeserialPrimitive<bool>(&m_bNeedSerializeWGs, ist);
  Serializer::DeserialPrimitive<bool>(&m_bIsTask, ist);
  Serializer::DeserialPrimitive<bool>(&m_bCanUseGlobalWorkOffset, ist);
  Serializer::DeserialPrimitive<DeviceMode>(&m_targetDevice, ist);
  Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
  m_cpuMaxWGSize = (size_t)tmp;
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
    return m_reqdWGSize[0] ? m_reqdWGSize : nullptr;
}

size_t KernelProperties::GetBarrierBufferSize() const
{
    return m_barrierBufferSize;
}

size_t KernelProperties::GetPrivateMemorySize() const
{
    return m_privateMemorySize;
}

size_t KernelProperties::GetMaxPrivateMemorySize() const
{
    return m_maxPrivateMemorySize;
}

size_t KernelProperties::GetRequiredNumSubGroups() const
{
    return m_reqdNumSG;
}

size_t KernelProperties::getMaterializedSubGroupSize() const {
  // Vectorization width is usually set by vectorizer or subgroup emulation.
  // This value is 1 (default value) iff neither vectorizer nor subgroup
  // emulation is run.
  // e.g. A kernel w/o any subgroup calls (no need to do subgroup emulation) and
  // compiled w/ '-cl-opt-disable' (vectorizer is disabled).
  // Although in such cases, querying compiled kernel's subgroup size doesn't
  // make much sense, we cannot return 1 directly as the kernel subgroup size
  // anyway, because '1' is not a supported size according to
  // CPU_DEV_SUB_GROUP_SIZES.
  size_t MaterializedSGSize =
      m_vectorizationWidth == 1 ? CPU_DEV_MAX_SG_SIZE : m_vectorizationWidth;
#ifndef NDEBUG
  const static std::unordered_set<size_t> SupportedSGSizes =
      CPU_DEV_SUB_GROUP_SIZES;
  assert(SupportedSGSizes.count(MaterializedSGSize) &&
         "Unexpected subgroup size");
#endif // NDEBUG
  return MaterializedSGSize;
}

size_t KernelProperties::GetMaxNumSubGroups(size_t const WGSizeUpperBound) const {
  return WGSizeUpperBound / getMaterializedSubGroupSize();
}

size_t KernelProperties::GetNumberOfSubGroups(size_t size, const size_t* WGSizes) const
{
    assert((m_verctorizeOnDimention < size) && "Vect dim must be less that NDRange dim!");

    // The following calculation routine will allow for extra
    // subgroup when WGSize % VF != 0.
    size_t SubGroupsOnVectorizedDim = ((WGSizes[m_verctorizeOnDimention] - 1) /
                                       getMaterializedSubGroupSize()) +
                                      1;
    size_t SubGroupsOnOtherDims = 1;
    for(size_t i = 0; i < size; ++i)
        if (i != m_verctorizeOnDimention)
            SubGroupsOnOtherDims *= WGSizes[i];

    return SubGroupsOnOtherDims * SubGroupsOnVectorizedDim;
}

size_t KernelProperties::GetMaxSubGroupSize() const {
  return getMaterializedSubGroupSize();
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

bool KernelProperties::HasNoBarrierPath() const { return m_hasNoBarrierPath; }

bool KernelProperties::HasMatrixCall() const { return m_hasMatrixCall; }

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

bool KernelProperties::IsAutorun() const
{
    return m_bIsAutorun;
}

DeviceMode KernelProperties::TargetDevice() const { return m_targetDevice; }

bool KernelProperties::IsTask() const
{
    return m_bIsTask;
}

bool KernelProperties::CanUseGlobalWorkOffset() const
{
    return m_bCanUseGlobalWorkOffset;
}

bool KernelProperties::NeedSerializeWGs() const
{
    return m_bNeedSerializeWGs;
}

void KernelProperties::GetLocalSizeForSubGroupCount(size_t const desiredSGCount,
                                                    size_t const wgSizeUpperBound,
                                                    size_t const wgPrivateMemSizeUpperBound,
                                                    size_t* pValue,
                                                    size_t const dim) const {
    assert(dim >=1 && "Expect dimensions to fill to be >= 1");

    const size_t maxWorkGroupSize =
        GetMaxWorkGroupSize(wgSizeUpperBound, wgPrivateMemSizeUpperBound);
    bool successFill = true;
    size_t wg_size = getMaterializedSubGroupSize() * desiredSGCount;
    if (wg_size <= maxWorkGroupSize) {
        pValue[0] = wg_size;
        successFill = true;
    } else {
        successFill = false;
    }

    // Desired SG count == 0 can't be fulfilled anyway.
    if (desiredSGCount == 0)
      successFill = false;

    if (successFill) {
        // fill the rest with ones.
        for(size_t i = 1; i < dim; ++i)
            pValue[i] = 1;
    } else {
        // fill everything with zero.
        for (size_t i = 0; i < dim; i++)
            pValue[i] = 0;
    }
}

size_t KernelProperties::GetMaxWorkGroupSize(size_t const wgSizeUpperBound,
                                             size_t const wgPrivateMemSizeUpperBound) const {
  assert(GetBarrierBufferSize() <= GetPrivateMemorySize() &&
         "kernel's private memory size must include barrier buffer size");

  // If there is no barrier buffer the private memory is reused by each WI.
  // So it can run WG with max. possible size.
  size_t maxWorkGroupSize = wgSizeUpperBound;

  // Return 0 if there is not enough private memory. But take into account what
  // in the vectorized JIT each lane has it's own private memory (not only barrier buffer).
  if(wgPrivateMemSizeUpperBound < GetPrivateMemorySize() * GetMinGroupSizeFactorial()) {
    // Private memory requirements exceed available resources.
    maxWorkGroupSize = 0;
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
    maxWorkGroupSize = leftForBuffers / GetBarrierBufferSize();
    // Take into account what max. WG size cannot exceed device limits.
    maxWorkGroupSize = wgSizeUpperBound < maxWorkGroupSize ? wgSizeUpperBound
                                                           : maxWorkGroupSize;
  }

  // If max. work-group size is greater than the vector width prune it to
  // the closest multiple of the vector width.
  if (maxWorkGroupSize > GetMinGroupSizeFactorial())
    maxWorkGroupSize &= ~(GetMinGroupSizeFactorial() - 1);

  return maxWorkGroupSize;
}

bool KernelProperties::IsNonUniformWGSizeSupported() const
{
    return m_bIsNonUniformWGSizeSupported;
}

void KernelProperties::Print() const {
  outs() << "[Kernel properties]\n";
  // Number of spaces.
  unsigned NS = 4;
  auto PrintArray = [&](StringRef Name, const size_t *A, unsigned Len) {
    outs().indent(NS) << Name << ": {";
    for (unsigned I = 0; I < Len; ++I)
      outs() << A[I] << ((I < Len - 1) ? ", " : "");
    outs() << "}\n";
  };
  outs().indent(NS) << "hasNoBarrierPath: " << m_hasNoBarrierPath << "\n";
  outs().indent(NS) << "hasMatrixCall: " << m_hasMatrixCall << "\n";
  outs().indent(NS) << "hasGlobalSync: " << m_hasGlobalSync << "\n";
  outs().indent(NS) << "DAZ: " << m_DAZ << "\n";
  outs().indent(NS) << "CPUId: " << m_cpuId.str() << "\n";
  outs().indent(NS) << "optWGSize: " << m_optWGSize << "\n";
  PrintArray("reqdWGSize", m_reqdWGSize, MAX_WORK_DIM);
  PrintArray("hintWGSize", m_hintWGSize, MAX_WORK_DIM);
  outs().indent(NS) << "totalImplSize: " << m_totalImplSize << "\n";
  outs().indent(NS) << "barrierBufferSize: " << m_barrierBufferSize << "\n";
  outs().indent(NS) << "privateMemorySize: " << m_privateMemorySize << "\n";
  outs().indent(NS) << "maxPrivateMemorySize: " << m_maxPrivateMemorySize
                    << "\n";
  outs().indent(NS) << "reqdNumSG: " << m_reqdNumSG << "\n";
  outs().indent(NS) << "kernelExecutionLength: " << m_kernelExecutionLength
                    << "\n";
  outs().indent(NS) << "vectorizationWidth: " << m_vectorizationWidth << "\n";
  outs().indent(NS) << "Materialized subgroup size: "
                    << getMaterializedSubGroupSize() << "\n";
  outs().indent(NS) << "reqdSubGroupSize: " << m_reqdSubGroupSize << "\n";
  outs().indent(NS) << "kernelAttributes: " << m_kernelAttributes << "\n";
  outs().indent(NS) << "minGroupSizeFactorial: " << m_minGroupSizeFactorial
                    << "\n";
  outs().indent(NS) << "isVectorizedWithTail: " << m_isVectorizedWithTail
                    << "\n";
  outs().indent(NS) << "sizeT: " << m_uiSizeT << "\n";
  outs().indent(NS) << "isBlock: " << m_bIsBlock << "\n";
  outs().indent(NS) << "isAutorun: " << m_bIsAutorun << "\n";
  outs().indent(NS) << "needSerializeWGs: " << m_bNeedSerializeWGs << "\n";
  outs().indent(NS) << "isTask: " << m_bIsTask << "\n";
  outs().indent(NS) << "canUseGlobalWorkOffset: " << m_bCanUseGlobalWorkOffset
                    << "\n";
  outs().indent(NS) << "isNonUniformWGSizeSupported: "
                    << m_bIsNonUniformWGSizeSupported << "\n";
  outs().indent(NS) << "canUniteWG: " << m_canUniteWG << "\n";
  outs().indent(NS) << "verctorizeOnDimention: " << m_verctorizeOnDimention
                    << "\n";
  outs().indent(NS) << "debugInfo: " << m_debugInfo << "\n";
  outs().indent(NS) << "targetDevice: "
                    << ((m_targetDevice == CPU_DEVICE)        ? "cpu"
                        : "fpga-emu")
                    << "\n";
  outs().indent(NS) << "cpuMaxWGSize: " << m_cpuMaxWGSize << "\n";
}
}}}
