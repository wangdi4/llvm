// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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

#include "EyeQBuiltinLibrary.h"
#include "exceptions.h"
#include "SystemInfo.h"
#include "llvm/Support/MemoryBuffer.h"

#if defined(_WIN32)
    #include <windows.h>
#else
    #include <linux/limits.h>
    #define MAX_PATH PATH_MAX
#endif

namespace Intel { namespace OpenCL { namespace DeviceBackend {

using namespace llvm;

void EyeQBuiltinLibrary::Load() {
  char Path[MAX_PATH];
  Utils::SystemInfo::GetModuleDirectory(Path, MAX_PATH);
  std::string PathStr(Path);
  PathStr += "eyeq_builtins";
#if defined(_WIN32)
  PathStr += '\\';
#else
  PathStr += '/';
#endif

  SmallVector<std::string, 1> EyeQBitExactEmulationRTLPaths;
  EyeQBitExactEmulationRTLPaths.push_back(PathStr + "eyeq-opencl-builtins-eyeqemu5.bc");

  for (const std::string &EyeQBitExactEmulationRTLPath : EyeQBitExactEmulationRTLPaths) {
    ErrorOr<std::unique_ptr<MemoryBuffer>> RTLBufferOrErr =
      MemoryBuffer::getFile(EyeQBitExactEmulationRTLPath);
    if(!RTLBufferOrErr)
      throw Exceptions::DeviceBackendExceptionBase(
        std::string("Failed to load " + EyeQBitExactEmulationRTLPath));

    m_RtlBuffersForEyeQEmulationMode.push_back(RTLBufferOrErr.get().release());
  }
  CPUBuiltinLibrary::Load();
}

}}} // namespace
