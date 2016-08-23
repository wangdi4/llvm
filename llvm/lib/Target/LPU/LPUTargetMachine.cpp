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
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"


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
  //setRequiresStructuredCFG(true);
  initAsmInfo();
  //setAsmVerbosityDefault(true);
}



void LPUTargetMachine::addAnalysisPasses(PassManagerBase &PM) {
  // Add first the target-independent BasicTTI pass, then our X86 pass. This
  // allows the X86 pass to delegate to the target independent layer when
  // appropriate.
  //PM.add(createBasicTargetTransformInfoPass(this));
  PM.add(createLPUTargetTransformInfoPass(this));
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

  bool addInstSelector() override {
    // Install an instruction selector.
    addPass(createLPUISelDag(getLPUTargetMachine(), getOptLevel()));
    return false;
  }



  bool addPreISel() override {
    //addPass(createUnifyFunctionExitNodesPass());
    return false;
  }

  
#define DEBUG_TYPE "lpu-convert-control"
  void addPreRegAlloc() override {
    std::string Banner;
#if 1
    Banner = std::string("Before Machine Block Placement Pass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

	addBlockPlacement();
	Banner = std::string("After Machine Block Placement Pass");
	DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createControlDepenceGraph(), false);
    Banner = std::string("After Machine CDG Pass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createLPUCvtCFDFPass(), false);
    Banner = std::string("After LPUCvtCFDFPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createLPUOptDFPass(), false);
    Banner = std::string("After LPUOptDFPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createLPUStatisticsPass(), false);
#else
    Banner = std::string("Before LPUConvertControlPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));
    addPass(createLPUConvertControlPass(), false);
    Banner = std::string("After LPUConvertControlPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    Banner = std::string("Before LPUOptDFPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createLPUOptDFPass(), false);
    Banner = std::string("After LPUOptDFPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));
#endif

}

  void addPostRegAlloc() override {
    addPass(createLPUAllocUnitPass(), false);
  }

};
} // namespace

TargetPassConfig *LPUTargetMachine::createPassConfig(PassManagerBase &PM) {
  LPUPassConfig *PassConfig = new LPUPassConfig(this, PM);
  return PassConfig;
}
