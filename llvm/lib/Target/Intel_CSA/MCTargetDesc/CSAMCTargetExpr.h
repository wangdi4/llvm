//====- CSAMCExpr.h - CSA specific MC expression classes ------*- C++ -*-=====//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of printing out register names for CSA
// LICs.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_MCTARGETDESC_CSAMCEXPR_H
#define LLVM_LIB_TARGET_CSA_MCTARGETDESC_CSAMCEXPR_H

#include "llvm/MC/MCExpr.h"

namespace llvm {

class CSAMCExpr : public MCTargetExpr {
private:
  const unsigned Register;
  const StringRef Name;

  explicit CSAMCExpr(unsigned regno, const StringRef name)
      : Register(regno), Name(name) {}

public:
  static const CSAMCExpr *create(unsigned regno, StringRef name,
                                 MCContext &Ctx);

  /// Get the underlying virtual register number of this LIC.
  unsigned getRegister() const { return Register; }

  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res, const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override {
    return false;
  }

  void visitUsedExpr(MCStreamer &Streamer) const override {}
  MCFragment *findAssociatedFragment() const override { return nullptr; }
  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override {}

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  static bool classof(const CSAMCExpr *) { return true; }
};

} // end namespace llvm.

#endif
