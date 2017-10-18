/*****************************************************************************\

Copyright (c) Intel Corporation (2010 - 2017).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CPUBuiltinLibrary.cpp

\*****************************************************************************/

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

  // Load SVML functions
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
