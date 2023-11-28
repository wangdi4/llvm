// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "VectorizerUtils.h"
#include "llvm/Support/CommandLine.h"

extern llvm::cl::opt<llvm::VFISAKind> IsaEncodingOverride;

namespace llvm {
namespace VectorizerUtils {

VFISAKind getCPUIdISA(const Intel::OpenCL::Utils::CPUDetect *CPUId) {
  if (IsaEncodingOverride.getNumOccurrences())
    return IsaEncodingOverride.getValue();

  assert(CPUId && "Valid CPUDetect is expected!");
  if (CPUId->HasAVX512Core())
    return llvm::VFISAKind::AVX512;
  if (CPUId->HasAVX2())
    return llvm::VFISAKind::AVX2;
  if (CPUId->HasAVX1())
    return llvm::VFISAKind::AVX;
  return llvm::VFISAKind::SSE;
}

} // namespace VectorizerUtils
} // namespace llvm
