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

#include "CPUBuiltinLibrary.h"
#include "exceptions.h"
#include "SystemInfo.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/DynamicLibrary.h"

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <linux/limits.h>
    #define MAX_PATH PATH_MAX
#endif


namespace Intel { namespace OpenCL { namespace DeviceBackend {

using namespace llvm;

void CPUBuiltinLibrary::Load() {
  char Path[MAX_PATH];
  Utils::SystemInfo::GetModuleDirectory(Path, MAX_PATH);

  // Klocwork warning - false alarm the Id is always in correct bounds
  const char* CPUPrefix = m_cpuId.GetCPUPrefix();
  std::string PathStr(Path);

  if (m_useDynamicSvmlLibrary) {
    // Load SVML functions
    assert(CPUPrefix && "CPUPrefix is null");
    std::string SVMLPath = PathStr + "__ocl_svml_" + CPUPrefix;
#if defined (_WIN32)
    SVMLPath += ".dll";
#else
    SVMLPath += ".so";
#endif

    std::string Err;
    if (sys::DynamicLibrary::LoadLibraryPermanently(SVMLPath.c_str(), &Err)) {
      throw Exceptions::DeviceBackendExceptionBase(
          std::string("Loading SVML library failed - ") + Err);
    }
  }

  // Load LLVM built-ins module(s)
  // Load target-specific built-in library
  std::string RTLPath = PathStr + "clbltfn" + CPUPrefix + ".rtl";
  ErrorOr<std::unique_ptr<MemoryBuffer>> RTLBufferOrErr =
    MemoryBuffer::getFile(RTLPath);
  if(!RTLBufferOrErr)
    throw Exceptions::DeviceBackendExceptionBase(
        std::string("Failed to load the builtins rtl library"));

  m_pRtlBuffer = RTLBufferOrErr.get().release();

  // Load shared built-in library
  // Deploy case:
  // lib/
  //    clbltfnshared.rtl
  //    intel64/ or ia32/
  //        other libraries
#if defined (_WIN32)
  std::string RTLSharedPath = PathStr + "..\\clbltfnshared.rtl";
#else
  std::string RTLSharedPath = PathStr + "../clbltfnshared.rtl";
#endif
  // Trying to load it
  ErrorOr<std::unique_ptr<MemoryBuffer>> RTLBufferSharedOrErr =
    MemoryBuffer::getFile(RTLSharedPath);
  if(!RTLBufferSharedOrErr) {
    // TODO: Fix build layout to not support this case
    // Development install case:
    // shared library is in the same directory as other ones
    RTLSharedPath = PathStr + "clbltfnshared.rtl";

    // Trying to load it
    RTLBufferSharedOrErr = MemoryBuffer::getFile(RTLSharedPath);
    if(!RTLBufferSharedOrErr)
      throw Exceptions::DeviceBackendExceptionBase(
          std::string("Failed to load the shared builtins rtl library"));
  }
  m_pRtlBufferSvmlShared = RTLBufferSharedOrErr.get().release();
}

}}} // namespace
