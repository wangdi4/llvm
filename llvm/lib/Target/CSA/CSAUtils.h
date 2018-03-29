//===-- llvm/lib/Target/CSA/CSAUtils.h ----------*- C++ -*-===//
//
//===----------------------------------------------------------------------===//
//
// This file contains code common to multiple passes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAUTILS_H
#define LLVM_LIB_TARGET_CSA_CSAUTILS_H

#include "llvm/CodeGen/MachineBasicBlock.h"
#include "CSA.h"
using namespace llvm;

namespace csa_utils {
  bool isAlwaysDataFlowLinkageSet(void);
  unsigned createUseTree(MachineBasicBlock *, MachineBasicBlock::iterator, unsigned, const SmallVector<unsigned, 4>, unsigned unusedReg = CSA::IGN);
  unsigned createPickTree(MachineBasicBlock *, MachineBasicBlock::iterator, const TargetRegisterClass *, 
                          const SmallVector<unsigned, 4>, const SmallVector<unsigned, 4>, unsigned unusedReg = CSA::IGN);
  void createSwitchTree(MachineBasicBlock *, MachineBasicBlock::iterator, const TargetRegisterClass *, 
                          const SmallVector<unsigned, 4>, SmallVector<unsigned, 4> &, unsigned, unsigned, unsigned unusedReg = CSA::IGN);
}

#endif