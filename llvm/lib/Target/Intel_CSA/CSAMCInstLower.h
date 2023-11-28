//===-- CSAMCInstLower.h - Lower MachineInstr to MCInst ---------*- C++ -*-===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_CSA_CSAMCINSTLOWER_H
#define LLVM_LIB_TARGET_CSA_CSAMCINSTLOWER_H

#include "llvm/Support/Compiler.h"

namespace llvm {
class AsmPrinter;
class MCContext;
class MCInst;
class MCOperand;
class MCSymbol;
class MachineInstr;
class MachineModuleInfoMachO;
class MachineOperand;

/// CSAMCInstLower - This class is used to lower an MachineInstr
/// into an MCInst.
class LLVM_LIBRARY_VISIBILITY CSAMCInstLower {
  MCContext &Ctx;

  AsmPrinter &Printer;

public:
  CSAMCInstLower(MCContext &ctx, AsmPrinter &printer)
      : Ctx(ctx), Printer(printer) {}

  void Lower(const MachineInstr *MI, MCInst &OutMI) const;

  MCOperand LowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;

  MCSymbol *GetGlobalAddressSymbol(const MachineOperand &MO) const;
  MCSymbol *GetExternalSymbolSymbol(const MachineOperand &MO) const;
  MCSymbol *GetBlockAddressSymbol(const MachineOperand &MO) const;
  MCSymbol *GetJumpTableSymbol(const MachineOperand &MO) const;
  MCSymbol *GetConstantPoolIndexSymbol(const MachineOperand &MO) const;
};

} // namespace llvm

#endif
