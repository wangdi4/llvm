//===---- MachineOptReportEmitter.cpp - Prints Optimization reports -------===//
//
// Copyright (C) 2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements Machine Optimization Report emitter.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Intel_OptReport/OptReport.h"
#include "llvm/Analysis/Intel_OptReport/OptReportBuilder.h"
#include "llvm/Analysis/Intel_OptReport/OptReportOptionsPass.h"
#include "llvm/Analysis/Intel_OptReport/OptReportPrintUtils.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetMachine.h"

#define DEBUG_TYPE "intel-mir-optreport-emitter"

using namespace llvm;
using namespace llvm::OptReportUtils;

static cl::opt<bool> DisableMIROptReportEmitter(
    "disable-intel-mir-optreport-emitter", cl::init(false), cl::Hidden,
    cl::desc("Disable MIR optimization report emitter"));

namespace {
struct MachineOptReportEmitter : public MachineFunctionPass {
  static char ID;

  MachineOptReportEmitter() : MachineFunctionPass(ID) {
    initializeMachineOptReportEmitterPass(*PassRegistry::getPassRegistry());
  }

  void printOptReportRecursive(MachineLoop *L, unsigned Depth, raw_ostream &OS,
                               bool AbsolutePaths);

  bool runOnMachineFunction(MachineFunction &F) override;

  // We don't modify the program, so we preserve all analyses.
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

void MachineOptReportEmitter::printOptReportRecursive(MachineLoop *ML,
                                                      unsigned Depth,
                                                      raw_ostream &OS,
                                                      bool AbsolutePaths) {

  MDNode *LoopID = ML->getLoopID();
  OptReport OR = OptReport::findOptReportInLoopID(LoopID);

  printNodeHeaderAndOrigin(OS, Depth, OR, ML->getStartLoc(), AbsolutePaths);

  if (OR)
    printOptReport(OS, Depth + 1, OR, AbsolutePaths);

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
    printOptReportRecursive(*I, Depth + 1, OS, AbsolutePaths);

  printNodeFooter(OS, Depth, OR);

  if (OR && OR.nextSibling())
    printEnclosedOptReport(OS, Depth, OR.nextSibling(), AbsolutePaths);
}

bool MachineOptReportEmitter::runOnMachineFunction(MachineFunction &MF) {
  if (DisableMIROptReportEmitter)
    return false;

  const Function &F = MF.getFunction();
  const MachineLoopInfo &MLI = getAnalysis<MachineLoopInfo>();
  const OptReportOptions &Options = getAnalysis<OptReportOptionsPass>().Impl;
  raw_ostream &OS = OptReportOptions::getOutputStream();
  OS << "Global Mloop optimization report for : " << F.getName() << "\n";

  // First check that there are attached reports to the function itself.
  OptReport FunOR = OptReportTraits<Function>::getOptReport(F);
  if (FunOR)
    printEnclosedOptReport(OS, 0, FunOR.firstChild(),
                           Options.shouldPrintAbsolutePaths());

  // Traversal through all loops of the program in lexicographical order.
  // Due to the specifics of loop build algorithm, it is achieved via reverse
  // iteration.
  for (auto I = MLI.rbegin(); I != MLI.rend(); ++I)
    printOptReportRecursive(*I, 0, OS, Options.shouldPrintAbsolutePaths());

  OS << "================================================================="
        "\n\n";

  return false;
}

void MachineOptReportEmitter::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesAll();
  AU.addRequired<MachineLoopInfo>();
  AU.addRequired<OptReportOptionsPass>();
  MachineFunctionPass::getAnalysisUsage(AU);
}
} // namespace

char MachineOptReportEmitter::ID = 0;
char &llvm::MachineOptReportEmitterID = MachineOptReportEmitter::ID;
static const char mlore_name[] = "The pass emits optimization reports";

INITIALIZE_PASS_BEGIN(MachineOptReportEmitter, DEBUG_TYPE, mlore_name, false,
                      false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_DEPENDENCY(OptReportOptionsPass)
INITIALIZE_PASS_END(MachineOptReportEmitter, DEBUG_TYPE, mlore_name, false,
                    false)
