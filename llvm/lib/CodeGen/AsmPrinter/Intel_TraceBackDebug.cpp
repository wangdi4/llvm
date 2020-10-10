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
}

void TraceBackDebug::addLineInfo(const MachineInstr *MI) {
  requestLabelBeforeInsn(MI);
  DebugHandlerBase::beginInstruction(MI);
  MCSymbol *Begin = getLabelBeforeInsn(MI);
  unsigned Line = MI->getDebugLoc().getLine();
  DebugModules.back().addLine(Line, Begin);
}

// Get the filename(excluding the path to it).
static std::string getFilename(const DIFile *File) {
  const auto &PathName = File->getFilename().str();
  const auto Pos = PathName.find_last_of("/\\");
  return (Pos == std::string::npos) ? PathName : PathName.substr(Pos + 1);
}

// Returns true if machine function has prologue.
static bool hasPrologue(const MachineFunction &MF) {
  // The first non-meta and non-frame setup instruction marks the end of the
  // prologue, so we can check the first instruction to determine whether a
  // function has a prologue.
  for (const auto &MBB : MF)
    for (const auto &MI : MBB) {
      if (MI.isMetaInstruction() || MI.getFlag(MachineInstr::FrameSetup))
        return true;
      else
        return false;
    }
  return false;
}

static bool isInSameSection(const MCSymbol *LHS, const MCSymbol *RHS) {
  if (!LHS || !RHS)
    return false;
  return &(LHS->getSection()) == &(RHS->getSection());
}

void TraceBackDebug::beginFunctionImpl(const MachineFunction *MF) {
  const Function &F = MF->getFunction();
  const DISubprogram *SP = F.getSubprogram();
  const DIFile *File = SP->getFile();

  if (FileToIndex.find(File) == FileToIndex.end()) {
    // Insert a new {file, index} pair to the map. The functions from different
    // files can be overlapped in a IR file, e.g, the first is f1 from file1,
    // the second is f2 from file2, then f3 from file1, so we need to use a map
    // here to check if a new file is found.
    FileToIndex.insert({File, FileToIndex.size()});
  }

  MCSymbol *Begin = Asm->getSymbol(&F);
  if (!isInSameSection(Begin, PrevFnSym)) {
    // Add a new module.
    DebugModules.push_back(new TraceModule(Asm->getPointerSize()));
  }

  if (File != PrevFile || DebugModules.back().empty()) {
    // Add a new file to debug module.
    DebugModules.back().addFile(getFilename(File), FileToIndex[File]);
  }

  unsigned Line = SP->getScopeLine(); // Start line of the function.
  const std::string &RoutineName = SP->getName().str();
  assert(Begin && Begin->isDefined() && "Expect defined begin label");
  DebugModules.back().addRoutine(RoutineName, Line, Begin, hasPrologue(*MF));
}

void TraceBackDebug::endFunctionImpl(const MachineFunction *MF) {
  MCSymbol *End = Asm->getFunctionEnd();
  assert(End && End->isDefined() && "Expect defined end label");
  DebugModules.back().endRoutine(End);

  // Track the begin symbol of the previous machine function.
  // It is used to check if a new section starts in beginFunctionImpl.
  PrevFnSym = Asm->getFunctionBegin();

  // Track the file where the previous subprogram is contained.
  // It is used to check if a new file starts in beginFunctionImpl.
  PrevFile = MF->getFunction().getSubprogram()->getFile();
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
      DebugModules.back().getLastLineNo() == DL.getLine()) {
    DebugHandlerBase::beginInstruction(MI);
    return;
  }

  addLineInfo(MI);
}

void TraceBackDebug::endModule() {
  for (auto &Module : DebugModules) {
    Module.endModule();
    Module.emit(*(Asm->OutStreamer));
  }
}
