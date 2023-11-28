// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#include "Program.h"
#include "BackendUtils.h"
#include "BitCodeContainer.h"
#include "Kernel.h"
#include "ObjectCodeContainer.h"
#include "Serializer.h"
#include "cache_binary_handler.h"
#include "cl_device_api.h"
#include "cl_sys_defines.h"
#include "exceptions.h"

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
Program::Program()
    : m_pObjectCodeContainer(nullptr), m_pIRCodeContainer(nullptr),
      m_kernels(nullptr), m_codeProfilingStatus(PROFILING_NONE) {}

Program::~Program() {
  m_kernels.reset(nullptr);

  for (auto &gv : m_globalVariables) {
    free(gv.name);
    free(gv.deco_name);
  }

  delete m_pObjectCodeContainer;
  delete m_pIRCodeContainer;
}

unsigned long long int Program::GetProgramID() const {
  assert(false && "NotImplemented");
  return 0;
}

const char *Program::GetBuildLog() const {
  return m_buildLog.empty() ? "" : m_buildLog.c_str();
}

const ICLDevBackendCodeContainer *Program::GetProgramIRCodeContainer() const {
  return m_pIRCodeContainer;
}

const ICLDevBackendCodeContainer *Program::GetProgramCodeContainer() const {
  return m_pObjectCodeContainer;
}

const ICLDevBackendProgramJITCodeProperties *
Program::GetProgramJITCodeProperties() const {
  assert(false && "NotImplemented");
  return nullptr;
}

cl_dev_err_code
Program::GetKernelByName(const char *IN pKernelName,
                         const ICLDevBackendKernel_ **OUT pKernel) const {
  if (!m_kernels.get() || m_kernels->Empty()) {
    return CL_DEV_INVALID_KERNEL_NAME;
  }

  try {
    *pKernel = (ICLDevBackendKernel_ *)m_kernels->GetKernel(pKernelName);
    return CL_DEV_SUCCESS;
  } catch (Exceptions::DeviceBackendExceptionBase &) {
    return CL_DEV_INVALID_KERNEL_NAME;
  }
}

int Program::GetKernelsCount() const {
  if (!m_kernels.get()) {
    return 0;
  }

  return m_kernels->GetCount();
}

int Program::GetNonBlockKernelsCount() const {
  if (!m_kernels.get()) {
    return 0;
  }

  return m_kernels->GetCount() - m_kernels->GetBlockCount();
}

cl_dev_err_code
Program::GetKernel(int kernelIndex,
                   const ICLDevBackendKernel_ **OUT ppKernel) const {
  if (!m_kernels.get() || m_kernels->Empty()) {
    return CL_DEV_INVALID_OPERATION;
  }
  // TODO: exception handling

  *ppKernel = m_kernels->GetKernel(kernelIndex);
  return CL_DEV_SUCCESS;
}

void Program::SetGlobalVariables(std::vector<cl_prog_gv> gvs) {
  m_globalVariables = std::move(gvs);
}

void Program::RecordCtorDtors(llvm::Module &M) {
  BackendUtils::recordGlobalCtorDtors(M, m_globalCtors, m_globalDtors);
}

void Program::SetObjectCodeContainer(ObjectCodeContainer *pObjCodeContainer) {
  delete m_pObjectCodeContainer;
  m_pObjectCodeContainer = pObjCodeContainer;
}

ObjectCodeContainer *Program::GetObjectCodeContainer() {
  return m_pObjectCodeContainer;
}

void Program::SetBitCodeContainer(BitCodeContainer *bitCodeContainer) {
  delete m_pIRCodeContainer;
  m_pIRCodeContainer = bitCodeContainer;
}

/// Currently the hash is computed from m_pIRCodeContainer.
std::string Program::GenerateHash() {
  if (!m_hash.empty())
    return m_hash;

  assert(m_pIRCodeContainer && "invalid IR code container");
  const llvm::MemoryBuffer *Buf = m_pIRCodeContainer->GetMemoryBuffer();

  // Compute hash.
  MD5 Hash;
  std::array<uint8_t, 16> Bytes = Hash.hash(ArrayRef<uint8_t>(
      (const uint8_t *)Buf->getBufferStart(), Buf->getBufferSize()));
  SmallString<16> Str;
  raw_svector_ostream OS(Str);
  for (int i = 0; i < 8; ++i)
    OS << format("%.2x", Bytes[i] ^ Bytes[i + 8]);
  m_hash = Str.str().str();

  return m_hash;
}

void Program::SetBuildLog(const std::string &buildLog) {
  m_buildLog = buildLog;
}

void Program::SetKernelSet(std::unique_ptr<KernelSet> pKernels) {
  m_kernels = std::move(pKernels);
}

void Program::SetModule(std::unique_ptr<llvm::Module> M) {
  assert(m_pIRCodeContainer && "code container should be initialized by now");
  m_pIRCodeContainer->SetModule(std::move(M));
}

void Program::SetModule(llvm::orc::ThreadSafeModule TSM) {
  assert(m_pIRCodeContainer && "code container should be initialized by now");
  m_pIRCodeContainer->SetModule(std::move(TSM));
}

llvm::Module *Program::GetModule() {
  assert(m_pIRCodeContainer && "code container should be initialized by now");
  return m_pIRCodeContainer->GetModule();
}

std::unique_ptr<llvm::Module> Program::GetModuleOwner() {
  assert(m_pIRCodeContainer && "code container should be initialized by now");
  return m_pIRCodeContainer->GetModuleOwner();
}

void Program::Serialize(IOutputStream &ost, SerializationStatus *stats) const {
  Serializer::SerialString(m_buildLog, ost);

  if (OCL_CACHED_BINARY_VERSION >= 17)
    // Profiling
    Serializer::SerialPrimitive<unsigned int>(&m_codeProfilingStatus, ost);

  unsigned int kernelsCount = m_kernels->GetCount();
  Serializer::SerialPrimitive<unsigned int>(&kernelsCount, ost);
  for (unsigned int i = 0; i < m_kernels->GetCount(); ++i) {
    Kernel *currentKernel = m_kernels->GetKernel(i);
    Serializer::SerialPointerHint(
        const_cast<const void **>(reinterpret_cast<void **>(&currentKernel)),
        ost);
    if (nullptr != currentKernel) {
      currentKernel->Serialize(ost, stats);
    }
  }

  // Global variables
  unsigned long long int tmp =
      (unsigned long long int)m_globalVariableTotalSize;
  Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  unsigned int gvCount = (unsigned int)m_globalVariables.size();
  Serializer::SerialPrimitive<unsigned int>(&gvCount, ost);
  for (auto &gv : m_globalVariables) {
    std::string name = gv.name;
    Serializer::SerialString(name, ost);
    if (OCL_CACHED_BINARY_VERSION >= 19) {
      std::string deco_name = gv.deco_name;
      Serializer::SerialString(deco_name, ost);
      Serializer::SerialPrimitive<unsigned int>(&gv.host_access, ost);
    }
    if (OCL_CACHED_BINARY_VERSION >= 20)
      Serializer::SerialPrimitive<bool>(&gv.device_image_scope, ost);
    tmp = (unsigned long long int)gv.size;
    Serializer::SerialPrimitive<unsigned long long int>(&tmp, ost);
  }

  // Global Ctor Names
  unsigned int ctorCount = (unsigned int)m_globalCtors.size();
  Serializer::SerialPrimitive<unsigned int>(&ctorCount, ost);
  for (const std::string &ctor : m_globalCtors)
    Serializer::SerialString(ctor, ost);
  // Global Dtor Names
  unsigned int dtorCount = (unsigned int)m_globalDtors.size();
  Serializer::SerialPrimitive<unsigned int>(&dtorCount, ost);
  for (const std::string &dtor : m_globalDtors)
    Serializer::SerialString(dtor, ost);
}

void Program::Deserialize(IInputStream &ist, SerializationStatus *stats) {
  Serializer::DeserialString(m_buildLog, ist);

  if (stats->m_binaryVersion >= 17)
    // Profiling
    Serializer::DeserialPrimitive<unsigned int>(&m_codeProfilingStatus, ist);

  if (m_codeProfilingStatus != PROFILING_NONE)
    LoadProfileLib();

  unsigned int kernelsCount = 0;
  Serializer::DeserialPrimitive<unsigned int>(&kernelsCount, ist);
  m_kernels.reset(new KernelSet());
  for (unsigned int i = 0; i < kernelsCount; ++i) {
    Kernel *currentKernel = nullptr;
    Serializer::DeserialPointerHint((void **)(&currentKernel), ist);
    if (nullptr != currentKernel) {
      currentKernel = stats->GetBackendFactory()->CreateKernel();
      currentKernel->Deserialize(ist, stats);
      m_kernels->AddKernel(std::unique_ptr<Kernel>(currentKernel));
    }
  }

  // Global variables
  unsigned long long int tmp;
  Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
  m_globalVariableTotalSize = (size_t)tmp;
  unsigned int gvCount;
  Serializer::DeserialPrimitive<unsigned int>(&gvCount, ist);
  m_globalVariables.resize(gvCount);
  for (unsigned int i = 0; i < gvCount; ++i) {
    std::string name;
    Serializer::DeserialString(name, ist);
    m_globalVariables[i].name = STRDUP(name.c_str());
    if (OCL_CACHED_BINARY_VERSION >= 19) {
      std::string deco_name;
      Serializer::DeserialString(deco_name, ist);
      m_globalVariables[i].deco_name = STRDUP(deco_name.c_str());
      Serializer::DeserialPrimitive<unsigned int>(
          &m_globalVariables[i].host_access, ist);
    }
    if (OCL_CACHED_BINARY_VERSION >= 20)
      Serializer::DeserialPrimitive<bool>(
          &m_globalVariables[i].device_image_scope, ist);
    Serializer::DeserialPrimitive<unsigned long long int>(&tmp, ist);
    m_globalVariables[i].size = (size_t)tmp;
  }

  // Global Ctor Names
  unsigned int ctorCount;
  Serializer::DeserialPrimitive<unsigned int>(&ctorCount, ist);
  for (unsigned int i = 0; i < ctorCount; ++i) {
    std::string name;
    Serializer::DeserialString(name, ist);
    m_globalCtors.push_back(name);
  }
  // Global Dtor Names
  unsigned int dtorCount;
  Serializer::DeserialPrimitive<unsigned int>(&dtorCount, ist);
  for (unsigned int i = 0; i < dtorCount; ++i) {
    std::string name;
    Serializer::DeserialString(name, ist);
    m_globalDtors.push_back(name);
  }
}

bool Program::HasCachedExecutable() const {
  if (!m_pObjectCodeContainer)
    return false;
  Intel::OpenCL::ELFUtils::CacheBinaryReader reader(
      m_pObjectCodeContainer->GetCode(), m_pObjectCodeContainer->GetCodeSize());
  return reader.IsCachedObject();
}
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
