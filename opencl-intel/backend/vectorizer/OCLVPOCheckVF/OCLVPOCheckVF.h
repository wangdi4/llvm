// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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

#ifndef __OCL_VPO_CHECK_VF_H__
#define __OCL_VPO_CHECK_VF_H__

#include "Compiler.h"
#include "InstCounter.h"
#include "VecConfig.h"

#include <llvm/Pass.h>

#include <string>

namespace intel {

class OCLVPOCheckVF : public llvm::ModulePass {

  using TStringToVFState = Intel::OpenCL::DeviceBackend::TStringToVFState;

public:
  static char ID;

  OCLVPOCheckVF(const OptimizerConfig &config, TStringToVFState &checkState)
      : llvm::ModulePass(ID), m_cpuId(config.GetCpuId()),
        m_transposeSize(config.GetTransposeSize()), m_canFallBack(false),
        m_checkState(checkState) {}

  ~OCLVPOCheckVF() {}

  llvm::StringRef getPassName() const override { return "OCLVPOCheckVF"; }

  bool runOnModule(llvm::Module &M) override;

  void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
    AU.addRequired<BuiltinLibInfo>();
  }

private:
  const Intel::CPUId &m_cpuId;

  ETransposeSize m_transposeSize;

  bool m_canFallBack;

  TStringToVFState &m_checkState;

  std::map<llvm::Function *, unsigned> m_kernelToVF;

  /// Check whether the VF are multi-contraint.
  /// Return true if VF is not multi-constraint.
  bool checkVFConstraints(llvm::Function *F);

  /// Determine the vectorization factor according to given constraints.
  void applyVFConstraints(llvm::Function *F);

  /// Check whether there are unsupported patterns for vectorization
  /// in the kernel.
  bool hasUnsupportedPatterns(llvm::Function *F);

  /// Check whether subgroups semantics is broken.
  /// Return true if subgroups semantics is maintained.
  bool checkSGSemantics(llvm::Function *F,
                        const std::set<llvm::Function *> &sgFuncUsers);

  /// Find all unimplement horizontal builtins (sub_group/work_group calls
  /// except for barrier) with given VF in the kernel.
  std::vector<std::pair<std::string, unsigned>>
  checkHorizontalOps(llvm::Function *F);
};
} // namespace intel
#endif
