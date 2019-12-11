// INTEL CONFIDENTIAL
//
// Copyright 2017-2018 Intel Corporation.
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

#ifndef __AUTORUN_REPLICATOR_H__
#define __AUTORUN_REPLICATOR_H__

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

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

