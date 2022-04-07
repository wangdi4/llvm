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

#include "RemoveAtexit.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "OCLPassSupport.h"

#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/IRBuilder.h"

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

extern "C" {
ModulePass *createRemoveAtExitPass() { return new intel::RemoveAtExit(); }
}

namespace intel {
char RemoveAtExit::ID = 0;

OCL_INITIALIZE_PASS(RemoveAtExit, "remove-atexit",
                    "remove atexit call temporarily.",
                    false, false)

RemoveAtExit::RemoveAtExit() : ModulePass(ID) {}

bool RemoveAtExit::runOnModule(Module &M) {
  bool Changed = false;
  for (auto &Func : M) {
    if (CompilationUtils::isGlobalCtorDtorOrCPPFunc(&Func))
       Changed |= runOnFunction(&Func);
  }

  return Changed;
}

bool RemoveAtExit::runOnFunction(Function *F) {
  bool Changed = false;
  inst_iterator ie;
  for (inst_iterator ii = inst_begin(F), ie = inst_end(F);
       ii != ie; ++ii) {
    CallInst *pCall = dyn_cast<CallInst>(&*ii);
    if (!pCall) {
      continue;
    }
    // Call instruction
    Function *pCallee = pCall->getCalledFunction();
    if (pCallee == nullptr) {
      continue;
    }
    if (pCallee->getName().equals("__cxa_atexit")) {
      // It should has no users.
      assert(pCall->users().empty());
      pCall->eraseFromParent();
      Changed = true;
      break;
    }
  }
  return Changed;
}

} // namespace intel
