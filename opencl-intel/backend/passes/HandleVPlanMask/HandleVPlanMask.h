// INTEL CONFIDENTIAL
//
// Copyright 2012-2020 Intel Corporation.
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

#ifndef __HANDLE_VPLAN_MASK_H__
#define __HANDLE_VPLAN_MASK_H__

#include <llvm/Pass.h>

using namespace llvm;

namespace intel {
/// @brief Convert VPlan style mask to Volcano style.
class HandleVPlanMask : public ModulePass {
public:
  // Pass identification, replacement for typeid.
  static char ID;

  /// @brief Constructor
  HandleVPlanMask() : ModulePass(ID){};

  /// @brief Provides name of pass
  llvm::StringRef getPassName() const override { return "HandleVPlanMask"; }

  /// @brief LLVM Module pass entry
  /// @param M Module to transform
  /// @returns true if changed
  bool runOnModule(Module &M) override;
};
} // namespace intel

#endif // __HANDLE_VPLAN_MASK_H__
