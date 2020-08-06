//===--- Intel_TraceBackDebug.h - TraceBack Debug Handler -------*- C++ -*-===//
//
// Copyright (C) 2020 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file defines the debug handler for traceback, which collects the debug
/// information when traversing the machine functions and instructions, and
/// finally writing them into asm/object file.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_CODEGEN_ASMPRINTER_TRACEBACKDEBUG_H
#define LLVM_LIB_CODEGEN_ASMPRINTER_TRACEBACKDEBUG_H

#include "llvm/ADT/ilist.h"
#include "llvm/CodeGen/DebugHandlerBase.h"
#include "llvm/DebugInfo/Intel_TraceBack/TraceDINode.h"

namespace llvm {

class DICompileUnit;
class DISubprogram;
class MCObjectStreamer;

/// Collects and handles traceback information.
class TraceBackDebug : public DebugHandlerBase {
public:
  TraceBackDebug(AsmPrinter *AP);

  void setSymbolSize(const MCSymbol *Sym, uint64_t Size) override {}

  /// Process beginning of an instruction.
  void beginInstruction(const MachineInstr *MI) override;

  /// Emit .trace section that should come after the content.
  void endModule() override;

protected:
  /// Gather pre-function debug information.
  void beginFunctionImpl(const MachineFunction *MF) override;

  /// Gather and emit post-function debug information.
  void endFunctionImpl(const MachineFunction *MF) override;

private:
  /// Add a line node for the machine instruction \p MI.
  void addLineInfo(const MachineInstr *MI);

private:
  /// The CU in which the previous subprogram was contained.
  const DICompileUnit *PrevCU = nullptr;
  /// The module used to store all debug info in .trace section
  std::unique_ptr<TraceModule> DebugModule;
};

} // end namespace llvm
#endif // LLVM_LIB_CODEGEN_ASMPRINTER_TRACEBACKDEBUG_H
