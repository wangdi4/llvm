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
  std::string file_prefix = "";
  std::string file_ext = ".dll";
#else
  std::string file_prefix = "lib";
  std::string file_ext = ".so";
#endif
  std::string MPIRPath = file_prefix + "dspba_mpir" + file_ext;
  std::string MPFRPath = file_prefix + "dspba_mpfr" + file_ext;
  std::string FIX_P_M_Path =
      file_prefix + "ac_types_fixed_point_math_x86" + file_ext;
  std::string VPFP_Path = file_prefix + "ac_types_vpfp_library" + file_ext;
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
  // Load the fixed point math libary.
  if (continue_load &&
      sys::DynamicLibrary::LoadLibraryPermanently(FIX_P_M_Path.c_str(), &Err)) {
    continue_load = false;
    m_builtinLibLog += "Loading ac_types_fixed_point_math_x86 library failed: ";
    m_builtinLibLog += Err;
  }
  // Load the variable precision floating point library.
  if (continue_load &&
      sys::DynamicLibrary::LoadLibraryPermanently(VPFP_Path.c_str(), &Err)) {
    m_builtinLibLog += "Loading ac_types_vpfp_library failed: ";
    m_builtinLibLog += Err;
  }
  CPUBuiltinLibrary::Load();
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
