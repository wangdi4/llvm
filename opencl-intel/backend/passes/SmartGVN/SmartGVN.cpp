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

File Name:  SmartGVN.cpp

\*****************************************************************************/

#include "SmartGVN.h"

#include <OCLPassSupport.h>

#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Dominators.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Analysis/MemoryDependenceAnalysis.h>
#include <llvm/Analysis/BasicAliasAnalysis.h>
#include <llvm/Support/CommandLine.h>

using namespace llvm;

static cl::opt<unsigned int>
GVNHNumOfInstructions("gvn-inst-number", cl::init(500), cl::Hidden,
cl::desc("The size of basic block inside a loop which is large enough "
         "for disabling GVN-PRE optimization for load instructions."));

static cl::opt<unsigned int>
GVNHLoadRatio("gvn-load-ratio", cl::init(10), cl::Hidden,
cl::desc("The percentage of load instructions in a basic block which disables "
         "GVN-PRE optimization for load intructions."));

namespace intel {

SmartGVN::SmartGVN(bool doNoLoadAnalysis) :
  ModulePass(ID), noLoadAnalysis(doNoLoadAnalysis)
{}

bool SmartGVN::runOnModule(Module &M)
{
  // First, analyse the function and decide whether we will run GVN with NoLoad
  // parameter "on" or "off".

  bool GVNNoLoads = false;
  if (noLoadAnalysis) {
    // Go over all functions in the module
    for (Module::iterator i = M.begin(), e = M.end(); i != e; ++i) {
      // Ignore declarations.
      if (i->isDeclaration()) continue;

      GVNNoLoads = GVNNoLoads || isNoLoadsCandidate(&*i);
    }
  }

  { // With NoLoads option on - it will not hoist loads out of the loops.
    legacy::PassManager pm;
    pm.add(llvm::createBasicAAWrapperPass());
    pm.add(new llvm::DominatorTreeWrapperPass());
    pm.add(new llvm::MemoryDependenceWrapperPass());
    pm.add(createGVNPass(GVNNoLoads));
    pm.run(M);
  }

  return true;
}

bool SmartGVN::isNoLoadsCandidate(Function *func)
{
  LoopInfo &LI = getAnalysis<LoopInfoWrapperPass>(*func).getLoopInfo();
  // We consider the function to be a good candidate for GVN with NoLoads if:
  // 1. It has a loop which consists of single basic block + loop header.
  // 2. This basic block has a lot of instructions with long live-interval
  // variables, which are loaded from memory.
  for (Function::iterator i = func->begin(), e = func->end(); i != e; ++i) {
    BasicBlock* BB = &*i;
    if (Loop *L = LI.getLoopFor(BB)) {
      if (L->getBlocks().size() <= 2) {
        // Number of instructions in the loop
        size_t numOfInstructions = i->getInstList().size();
        // count number of loads nodes to estimate number of "live variables"
        unsigned int numOfLoadNodes = 0;
        for (BasicBlock::iterator bbi = BB->begin(), bbe = BB->end(); bbi != bbe;
             ++bbi) {
          if (isa<LoadInst>(&*bbi)) {
            ++numOfLoadNodes;
          }
        }

        // number of loads > GVNHLoadRatio% of the total number of instructions
        if (numOfInstructions >= GVNHNumOfInstructions &&
            numOfLoadNodes >= numOfInstructions/GVNHLoadRatio) {
          return true;
        }
      }
    }
  }
  return false;
}

char SmartGVN::ID = 0;
OCL_INITIALIZE_PASS(SmartGVN, "SmartGVN", "Smart GVN", false, false)

} // Namespace intel

extern "C" {
llvm::ModulePass *createSmartGVNPass(bool doNoLoadAnalysis)
{
  return new intel::SmartGVN(doNoLoadAnalysis);
}
}

