//===-- Nios2SEISelDAGToDAG.cpp - A Dag to Dag Inst Selector for Nios2SE ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Subclass of Nios2DAGToDAGISel specialized for nios232.
//
//===----------------------------------------------------------------------===//

#include "Nios2SEISelDAGToDAG.h"

#include "MCTargetDesc/Nios2BaseInfo.h"
#include "Nios2.h"
#include "Nios2AnalyzeImmediate.h"
#include "Nios2MachineFunction.h"
#include "Nios2RegisterInfo.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;

#define DEBUG_TYPE "nios2-isel"

bool Nios2SEDAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
  Subtarget = &static_cast<const Nios2Subtarget &>(MF.getSubtarget());
  return Nios2DAGToDAGISel::runOnMachineFunction(MF);
}

/// Select multiply instructions.
std::pair<SDNode *, SDNode *>
Nios2SEDAGToDAGISel::selectMULT(SDNode *N, unsigned Opc, const SDLoc &DL, EVT Ty,
                             bool HasLo, bool HasHi) {
  SDNode *Lo = 0, *Hi = 0;

  return std::make_pair(Lo, Hi);
}

void Nios2SEDAGToDAGISel::processFunctionAfterISel(MachineFunction &MF) {
}

void Nios2SEDAGToDAGISel::selectAddESubE(unsigned MOp, SDValue InFlag,
                                           SDValue CmpLHS, const SDLoc &DL,
                                           SDNode *Node) const {
}

//@selectNode
bool Nios2SEDAGToDAGISel::trySelect(SDNode *Node) {
  return false;
}

FunctionPass *llvm::createNios2SEISelDag(Nios2TargetMachine &TM,
                                        CodeGenOpt::Level OptLevel) {
  return new Nios2SEDAGToDAGISel(TM, OptLevel);
}
