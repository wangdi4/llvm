//==--- AutorunReplicator.h - AutorunReplicator pass           -*- C++ -*---==//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __AUTORUN_REPLICATOR_H__
#define __AUTORUN_REPLICATOR_H__

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace intel {

class AutorunReplicator : public llvm::ModulePass {
public:
  static char ID;
  AutorunReplicator();

  llvm::StringRef getPassName() const override { return "AutorunReplicator"; }

  bool runOnModule(llvm::Module &M) override;

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {}

private:
  struct ComputeID {
    ComputeID() {}

    ComputeID(int X, int Y, int Z) : XDimValue(X), YDimValue(Y), ZDimValue(Z) {}

    int XDimValue = -1;
    int YDimValue = -1;
    int ZDimValue = -1;
  };

  bool createReplicas(llvm::Function *F,
                      llvm::SmallVectorImpl<llvm::Function *> &Replicas);

  void resolveGetComputeID(llvm::CallInst *GetComputeIDCall);

  typedef llvm::DenseMap<llvm::Function *, ComputeID> ComputeIDsMapTy;
  ComputeIDsMapTy m_ComputeIDs;
};

} // namespace intel

#endif // __AUTORUN_REPLICATOR_H__

