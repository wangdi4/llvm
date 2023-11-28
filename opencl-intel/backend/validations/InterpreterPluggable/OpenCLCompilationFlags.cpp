// INTEL CONFIDENTIAL
//
// Copyright 2013 Intel Corporation.
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

#include "OpenCLCompilationFlags.h"
#include "llvm/IR/Constants.h"

using namespace llvm;

namespace Validation {

unsigned CompilationFlags::getCLVersionFromMetadata(Module *M) {
  assert(M && "Invalid module");

  auto OCLVersionMD = M->getNamedMetadata("opencl.ocl.version");

  if (!OCLVersionMD || !OCLVersionMD->getNumOperands())
    return 120;

  auto Op0 = OCLVersionMD->getOperand(0);

  return mdconst::extract<ConstantInt>(Op0->getOperand(0))->getZExtValue() *
             100 +
         mdconst::extract<ConstantInt>(Op0->getOperand(1))->getZExtValue() * 10;
}

unsigned CompilationFlags::getCLVersionFromFlags(const std::string &Flags) {
  unsigned DefaultVersion = 120;

  if (Flags.empty())
    return DefaultVersion;

  std::string::size_type n = Flags.find("-cl-std=CL");
  if (n == std::string::npos)
    return DefaultVersion;

  return std::stoi(Flags.substr(n + 10, 1)) * 100 +
         std::stoi(Flags.substr(n + 10 + 2, 1)) * 10;
}

bool CompilationFlags::hasFastRelaxedMathFlag(const std::string &Flags) {
  if (Flags.empty())
    return false;

  return Flags.find("-cl-fast-relaxed-math") != std::string::npos;
}

bool CompilationFlags::hasUniformWGSizeFlag(const std::string &Flags) {
  if (Flags.empty())
    return false;

  return Flags.find("-cl-uniform-work-group-size") != std::string::npos;
}

} // namespace Validation
