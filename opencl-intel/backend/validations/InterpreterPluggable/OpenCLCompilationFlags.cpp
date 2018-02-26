/*****************************************************************************\

Copyright (c) Intel Corporation (2013-2018).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OpenCLCompilationFlags.cpp

\*****************************************************************************/
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

  return
    mdconst::extract<ConstantInt>(Op0->getOperand(0))->getZExtValue() * 100 +
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

}
