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

#include "FPGAEmuBuiltinLibrary.h"
#include "llvm/Support/DynamicLibrary.h"
#include <iostream>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

using namespace llvm;

void FPGAEmuBuiltinLibrary::Load() {
#if defined(_WIN32)
  std::string file_ext = ".dll";
#else
  std::string file_ext = ".so";
#endif
  std::string MPIRPath = "libpsg_mpir" + file_ext;
  std::string MPFRPath = "libpsg_mpfr" + file_ext;
  std::string FIX_P_M_Path = "libhls_fixed_point_math_x86" + file_ext;
  std::string HLS_VPFP_Path = "libhls_vpfp_library" + file_ext;
  bool continue_load = true;

  // Load these 4 libraries one by one basing on their dependency.
  std::string Err;
  if (sys::DynamicLibrary::LoadLibraryPermanently(MPIRPath.c_str(), &Err)) {
    continue_load = false;
    m_builtinLibLog += "Loading MPIR library failed: ";
    m_builtinLibLog += Err;
  }
  // Load mpfr library.
  if (continue_load &&
      sys::DynamicLibrary::LoadLibraryPermanently(MPFRPath.c_str(), &Err)) {
    continue_load = false;
    m_builtinLibLog += "Loading MPFR library failed:";
    m_builtinLibLog += Err;
  }
  // Load hls_fixed_point_math_x86 libary.
  if (continue_load &&
      sys::DynamicLibrary::LoadLibraryPermanently(FIX_P_M_Path.c_str(), &Err)) {
    continue_load = false;
    m_builtinLibLog += "Loading hls_fixed_point_math_x86 library failed: ";
    m_builtinLibLog += Err;
  }
  // Load  hls_vpfp_librarylibrary library.
  if (continue_load && sys::DynamicLibrary::LoadLibraryPermanently(
                           HLS_VPFP_Path.c_str(), &Err)) {
    m_builtinLibLog += "Loading hls_vpfp_library failed: ";
    m_builtinLibLog += Err;
  }
  CPUBuiltinLibrary::Load();
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
