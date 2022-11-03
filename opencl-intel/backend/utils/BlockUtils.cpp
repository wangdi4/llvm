/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  BlockUtils.cpp

\*****************************************************************************/

#define DEBUG_TYPE "BlockUtils"

#include "BlockUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Regex.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/// global var for speedup regexp initialization
namespace {
// hide regex in anonymous namespace
// should detect following function names:
// "__enqueue_simple_block_block_invoke"
// "globalBlock_block_invoke"
// TODO: revisit when issue below is fixed
// CSSD100018605 [OpenCL2.0] Clang should provide reliable way to detect block
// functions and compute block context size
llvm::Regex s_IsBlockRegex("_block_invoke(_[0-9]+)?$");
} // namespace

bool BlockUtils::isBlockInvokeFunction(const llvm::Function &F) {
  return s_IsBlockRegex.match(F.getName());
}

std::string BlockUtils::CreateBlockInvokeKernelName(const std::string &F) {
  assert(s_IsBlockRegex.match(F) && "string is not block literal");
  return "__.kernel" + F;
}

bool BlockUtils::IsBlockInvocationKernel(const llvm::Function &F) {
  bool res = false;
  const llvm::StringRef name(F.getName());
  LLVM_DEBUG(
      llvm::dbgs() << "BlockUtils::IsBlockInvocationKernel Entry point \n");
  LLVM_DEBUG(llvm::dbgs() << F.getName() << " input checked name \n");

  // if there is __kernel__ inside and block_invoke
  // it is great chance it is block_invoke kernel
  if ((name.find("__.kernel__") != llvm::StringRef::npos) &&
      (name.find("block_invoke") != llvm::StringRef::npos))
    res = true;

  LLVM_DEBUG(llvm::dbgs() << "Ret val is " << res << " \n");
  return res;
}

std::string
BlockUtils::ObtainBlockInvokeFuncNameFromKernel(const std::string &str) {
  assert(s_IsBlockRegex.match(str) && "string is not block literal");
  const llvm::StringRef s(str);
  return std::string(s.drop_front(strlen("__.kernel")));
}
} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
