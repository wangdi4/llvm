// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
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

#include "FPGABuiltinLibrary.h"
#include "SystemInfo.h"
#include "exceptions.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/MemoryBuffer.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <linux/limits.h>
#define MAX_PATH PATH_MAX
#endif

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

using namespace llvm;

void loadFPGALibraries() {
  char Path[MAX_PATH];
  Utils::SystemInfo::GetModuleDirectory(Path, MAX_PATH);
  std::string PathStr(Path);
  std::string Err;

  // INTEL VPO BEGIN
  // Load OpenMP library
#if defined(_WIN32)
  std::string RTLibOmpName = "libiomp5md.dll";
#else
  std::string RTLibOmpName = "libomp.so";
#endif

  // We have 2 places where OpenMP Runtime library can be located:
  //   - in the same directory as libOclCpuBackend.so
  //   - in the ICC redistributable directory
  //
  // First try to load it from the Backend directory:
  if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(
          (PathStr + RTLibOmpName).c_str(), &Err)) {
    // Apparently it is not there. Let's try to find it by name only
    // (lookup in library path)
    if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(RTLibOmpName.c_str(),
                                                          &Err)) {
      throw Exceptions::DeviceBackendExceptionBase(
          std::string("Failed to load OMP library ") + Err);
    }
  }

  // Load OpenMP library's "taskloop" subset implemented with TBB (Linux only)
  // Windows support is on the way:
  // https://jira01.devtools.intel.com/browse/CORC-2258
#if !defined(_WIN32)
  std::string RTLibOmpTbbName = PathStr + "libomptbb.so";
  if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(RTLibOmpTbbName.c_str(),
                                                        &Err)) {
    throw Exceptions::DeviceBackendExceptionBase(
        std::string("Failed to load OMP-TBB library ") + Err);
  }
#endif
  // INTEL VPO END
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
