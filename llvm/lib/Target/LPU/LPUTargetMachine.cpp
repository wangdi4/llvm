//===-- LPUTargetMachine.cpp - Define TargetMachine for LPU ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Top-level implementation for the LPU target.
//
//===----------------------------------------------------------------------===//

#include "LPUTargetMachine.h"
#include "LPU.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineFunctionAnalysis.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/PassManager.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Transforms/Scalar.h"


using namespace llvm;

extern "C" void LLVMInitializeLPUTarget() {
  // Register the target.
  RegisterTargetMachine<LPUTargetMachine> X(TheLPUTarget);
}

LPUTargetMachine::LPUTargetMachine(const Target &T, StringRef TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         Reloc::Model RM, CodeModel::Model CM,
                                         CodeGenOpt::Level OL)
    : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
      TLOF(make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, FS, *this) {
  // Not sure this is needed
  setRequiresStructuredCFG(true);
  initAsmInfo();
  //setAsmVerbosityDefault(true);
}

LPUTargetMachine::~LPUTargetMachine() {}

namespace {
/// LPU Code Generator Pass Configuration Options.
class LPUPassConfig : public TargetPassConfig {
public:
  LPUPassConfig(LPUTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  LPUTargetMachine &getLPUTargetMachine() const {
    return getTM<LPUTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPostRegAlloc() override;

  /*
  FunctionPass *createTargetRegisterAllocator(bool) override;
  void addFastRegAlloc(FunctionPass *RegAllocPass) override;
  void addOptimizedRegAlloc(FunctionPass *RegAllocPass) override;
  */

  void addMachineLateOptimization() override;
  bool addGCPasses() override;
  void addBlockPlacement() override;

  /* These are used by the AMD (R600) target.  May be interesting....
     See AMDGPUTargetMachine.cpp
  void addPreRegAlloc() override;
  void addCodeGenPrepare() override;
  bool addPreISel() override;
  void addPreSched2() override;
  void addPreEmitPass() override;
  */
};
} // namespace

TargetPassConfig *LPUTargetMachine::createPassConfig(PassManagerBase &PM) {
  LPUPassConfig *PassConfig = new LPUPassConfig(this, PM);
  return PassConfig;
}

void LPUPassConfig::addIRPasses() {

  // (Cribbed from NVPTX)
  // The following passes are known to not play well with virtual regs hanging
  // around after register allocation (which in our case, is *all* registers).
  // We explicitly disable them here.  We do, however, need some functionality
  // of the PrologEpilogCodeInserter pass, so we emulate that behavior in the
  // LPUPrologEpilog pass (see LPUPrologEpilogPass.cpp).
  disablePass(&PrologEpilogCodeInserterID);
  disablePass(&MachineCopyPropagationID);
  disablePass(&BranchFolderPassID);
  disablePass(&TailDuplicateID);
  disablePass(&MachineSchedulerID);
  disablePass(&PostMachineSchedulerID);
  disablePass(&PostRASchedulerID);
  // The above still doesn't disable list scheduling...??

  TargetPassConfig::addIRPasses();

}

bool LPUPassConfig::addInstSelector() {
  // Install an instruction selector.
  addPass(createLPUISelDag(getLPUTargetMachine(), getOptLevel()));
  return false;
}

void LPUPassConfig::addPostRegAlloc() {
  addPass(createLPUPrologEpilogPass(), false);
}

/*
FunctionPass *LPUPassConfig::createTargetRegisterAllocator(bool) {
  return nullptr; // No reg alloc
}

void LPUPassConfig::addFastRegAlloc(FunctionPass *RegAllocPass) {
  assert(!RegAllocPass && "LPU uses no regalloc!");
  //addPass(&PHIEliminationID); retain PHIs for now
}

void LPUPassConfig::addOptimizedRegAlloc(FunctionPass *RegAllocPass) {
  assert(!RegAllocPass && "LPU uses no regalloc!");

  //addPass(&ProcessImplicitDefsID);
  //addPass(&LiveVariablesID);
  //addPass(&MachineLoopInfoID);
  //addPass(&PHIEliminationID); retain PHIs for now
}
*/

void LPUPassConfig::addMachineLateOptimization() {
  // none for now
}

bool LPUPassConfig::addGCPasses() {
  // none for now
  return false;
}

void LPUPassConfig::addBlockPlacement() {
  // none for now
}

/*
void LPUPassConfig::addPreEmitPass() {
  // Must run branch selection immediately preceding the asm printer.
  //addPass(createLPUBranchSelectionPass(), false);
}
*/
