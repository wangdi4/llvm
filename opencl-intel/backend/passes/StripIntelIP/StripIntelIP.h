//==---------------------------- StripIntelIP.h ----------------*- C++ -*---==//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
// ===--------------------------------------------------------------------=== //

#ifndef __STRIP_INTEL_IP_H__
#define __STRIP_INTEL_IP_H__

#include "llvm/IR/Module.h"

namespace intel {

class StripIntelIP : public llvm::ModulePass {
public:
  static char ID;
  StripIntelIP();

  llvm::StringRef getPassName() const override { return "StripIntelIP"; }

  bool runOnModule(llvm::Module &M) override;
};

} // namespace intel

#endif // __STRIP_INTEL_IP_H__

