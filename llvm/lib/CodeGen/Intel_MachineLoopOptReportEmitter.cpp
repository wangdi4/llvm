//===- MachineLoopOptReportEmitter.cpp - Prints Loop Optimization reports -------===//
//
// Copyright (C) 2018-2019 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements Machine Loop Optimization Report emitter.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/LoopOptReport.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/LoopOptReportPrintUtils.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/TargetMachine.h"

#define DEBUG_TYPE "intel-mir-optreport-emitter"

using namespace llvm;
using namespace llvm::OptReportUtils;

static cl::opt<bool> DisableMIROptReportEmitter(
    "disable-intel-mir-optreport-emitter", cl::init(false), cl::Hidden,
    cl::desc("Disable MIR optimization report emitter"));

namespace {
struct MachineLoopOptReportEmitter : public MachineFunctionPass {
  static char ID;

  MachineLoopOptReportEmitter() : MachineFunctionPass(ID) {
    initializeMachineLoopOptReportEmitterPass(
        *PassRegistry::getPassRegistry());
  }

  void printLoopOptReportRecursive(MachineLoop *L, unsigned Depth,
                                   formatted_raw_ostream &FOS);

  bool runOnMachineFunction(MachineFunction &F) override;

  // We don't modify the program, so we preserve all analyses.
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

void MachineLoopOptReportEmitter::printLoopOptReportRecursive(
    MachineLoop *ML, unsigned Depth, formatted_raw_ostream &FOS) {

  MDNode *LoopID          = ML->getLoopID();
  LoopOptReport OptReport = LoopOptReport::findOptReportInLoopID(LoopID);

  printLoopHeaderAndOrigin(FOS, Depth, OptReport, ML->getStartLoc());

  if (OptReport)
    printOptReport(FOS, Depth + 1, OptReport);

  //
  // TODO (vzakhari 4/23/2018): the true/false successors of conditional
  //       branches are not ordered, so the sibling loops inside if/else
  //       clauses may be printed in any order.
  //       We can try to order them based on the source positions.
  //       When the loops are created due to the loop unswitching, we can
  //       use the loop version number stored in the opt-report to order
  //       the different versions.
  //
  auto EntryNode = GraphTraits<const MachineLoop *>::getEntryNode(ML);
  for (auto I = GraphTraits<const MachineLoop *>::child_begin(EntryNode),
           E = GraphTraits<const MachineLoop *>::child_end(EntryNode);
       I != E; ++I)
    printLoopOptReportRecursive(*I, Depth + 1, FOS);

  printLoopFooter(FOS, Depth);

  if (OptReport && OptReport.nextSibling())
    printEnclosedOptReport(FOS, Depth, OptReport.nextSibling());
}

bool MachineLoopOptReportEmitter::runOnMachineFunction(MachineFunction &MF) {
  if (DisableMIROptReportEmitter)
    return false;

  const Function &F = MF.getFunction();
  const MachineLoopInfo &MLI = getAnalysis<MachineLoopInfo>();
  formatted_raw_ostream OS(dbgs());
  OS << "Global Mloop optimization report for : " << F.getName() << "\n";

  // First check that there are attached reports to the function itself.
  LoopOptReport FunOR = LoopOptReportTraits<Function>::getOptReport(F);
  if (FunOR)
    printEnclosedOptReport(OS, 0, FunOR.firstChild());

  // Traversal through all loops of the program in lexicographical order.
  // Due to the specifics of loop build algorithm, it is achieved via reverse
  // iteration.
  for (auto I = MLI.rbegin(); I != MLI.rend(); ++I)
    printLoopOptReportRecursive(*I, 0, OS);

  OS << "================================================================="
        "\n\n";

  return false;
}

void MachineLoopOptReportEmitter::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineLoopInfo>();
  MachineFunctionPass::getAnalysisUsage(AU);
}
} // namespace


char MachineLoopOptReportEmitter::ID = 0;
char &llvm::MachineLoopOptReportEmitterID = MachineLoopOptReportEmitter::ID;
static const char mlore_name[] = "The pass emits optimization reports";

INITIALIZE_PASS_BEGIN(MachineLoopOptReportEmitter, DEBUG_TYPE,
                      mlore_name, false, false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_END(MachineLoopOptReportEmitter, DEBUG_TYPE,
                    mlore_name, false, false)
