//===--- Intel_TraceBackDebug.cpp - TraceBack Debug Handler -----*- C++ -*-===//
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
/// This file implements the methods of the debug handler for traceback.
///
//===----------------------------------------------------------------------===//

#include "Intel_TraceBackDebug.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;

TraceBackDebug::TraceBackDebug(AsmPrinter *A) : DebugHandlerBase(A) {
  assert(Asm->TM.getTargetTriple().isX86() &&
         "traceback is only supported on x86");
  // Module name(always empty) and format version (always 2.00) in ICC's current
  // implementation. Unlike DWARF, we only want to support the latest version of
  // traceback. So the version info is used to debug only for developers.
  DebugModule =
      std::make_unique<TraceModule>(Asm->getPointerSize(), /*Version=*/200,
                                    /*Name=*/std::string());
}

void TraceBackDebug::addLineInfo(const MachineInstr *MI) {
  requestLabelBeforeInsn(MI);
  DebugHandlerBase::beginInstruction(MI);
  MCSymbol *Begin = getLabelBeforeInsn(MI);
  unsigned Line = MI->getDebugLoc().getLine();
  DebugModule->addLine(Line, Begin);
}

// Get the filename of the compile unit (excluding the path to it).
static std::string getFilename(const DICompileUnit *CU) {
  const auto &PathName = CU->getFilename().str();
  const auto Pos = PathName.find_last_of("/\\");
  return (Pos == std::string::npos) ? PathName : PathName.substr(Pos + 1);
}

void TraceBackDebug::beginFunctionImpl(const MachineFunction *MF) {
  const DISubprogram *SP = MF->getFunction().getSubprogram();
  const DICompileUnit *CU = SP->getUnit();
  if (CU != PrevCU) {
    // Add a new file to debug module.
    DebugModule->addFile(getFilename(CU));
  }

  // Add a new routine to debug module.
  unsigned Line = SP->getScopeLine(); // Start line of the function.
  const std::string &RoutineName = SP->getName().str();
  MCSymbol *Begin = Asm->getFunctionBegin();
  assert(Begin && Begin->isDefined() && "Expect defined begin label");
  DebugModule->addRoutine(RoutineName, Line, Begin);
}

void TraceBackDebug::endFunctionImpl(const MachineFunction *MF) {
  MCSymbol *End = Asm->getFunctionEnd();
  assert(End && End->isDefined() && "Expect defined end label");
  DebugModule->endRoutine(End);

  // Track the CU where the previous subprogram was contained.
  // It is be used to check if a new file starts in beginFunctionImpl.
  PrevCU = MF->getFunction().getSubprogram()->getUnit();
}

// Usable locations are valid with non-zero line numbers. A line number of zero
// corresponds to optimized code that doesn't have a distinct source location.
// In this case, we will use the previous source location.
static bool isUsableDebugLoc(DebugLoc DL) { return DL && DL.getLine() != 0; }

// By the design of DebugHandlerBase, beginInstruction is able to transverse
// the instructions even if we don't need to emit debug info while
// beginFunctionImpl is called only when we need to. To traceback, each line
// record should have its parent routine, so we need to check this before
// adding a line record.
static bool doesEmitDebugInfo(const MachineInstr *MI) {
  const MachineFunction &MF = *MI->getMF();
  const DISubprogram *SP = MF.getFunction().getSubprogram();
  return SP && SP->getUnit()->getEmissionKind() != DICompileUnit::NoDebug;
}

void TraceBackDebug::beginInstruction(const MachineInstr *MI) {
  const DebugLoc &DL = MI->getDebugLoc();
  // We won't add a line record for the instruction when
  // - If no need to emit debug info.
  // - If this is a meta-instruction, such as DBG_VALUE and CFI locations.
  // - If the instruction is part of the function frame setup code, do not
  //   emit any line record, as there is no correspondence with any user code.
  // - If we don't have a usable location.
  // - If source line doesn't change.
  if (!doesEmitDebugInfo(MI) || MI->isMetaInstruction() ||
      MI->getFlag(MachineInstr::FrameSetup) || !isUsableDebugLoc(DL) ||
      DebugModule->getLastLineNo() == DL.getLine()) {
    DebugHandlerBase::beginInstruction(MI);
    return;
  }

  addLineInfo(MI);
}

void TraceBackDebug::endModule() { DebugModule->finish(*(Asm->OutStreamer)); }
