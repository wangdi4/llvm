//===-- CSATargetMachine.cpp - Define TargetMachine for CSA ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Top-level implementation for the CSA target.
//
//===----------------------------------------------------------------------===//

#include "CSATargetMachine.h"
#include "CSATargetTransformInfo.h"
#include "CSALowerAggrCopies.h"
#include "CSAFortranIntrinsics.h"
#include "CSA.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/IR/LegacyPassManager.h"
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

static cl::opt<int>
RunCSAStatistics("csa-run-statistics", cl::Hidden,
              cl::desc("CSA Specific: collect statistics for DF instructions"),
              cl::init(0));

static cl::opt<int>
CSAStructurizeCFG("csa-structurize-cfg", cl::Hidden,
  cl::desc("CSA Specific: leverage llvm StructurizeCFG"),
  cl::init(1));

// Helper function to build a DataLayout string
static std::string computeDataLayout() {
  return "e-m:e-i64:64-n32:64";
}


namespace llvm {
  void initializeCSALowerAggrCopiesPass(PassRegistry &);
  void initializeCSAFortranIntrinsicsPass(PassRegistry &);
}

extern "C" void LLVMInitializeCSATarget() {
  // Register the target.
  RegisterTargetMachine<CSATargetMachine> X(getTheCSATarget());

  // The original comment in the CSA target says this optimization
  // is placed here because it is too target-specific.
  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeCSALowerAggrCopiesPass(PR);
  initializeCSAFortranIntrinsicsPass(PR);
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::Static;
  return *RM;
}

CSATargetMachine::CSATargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         Optional<Reloc::Model> RM,
                                         CodeModel::Model CM,
                                         CodeGenOpt::Level OL)
    : LLVMTargetMachine(T, computeDataLayout(), TT, CPU, FS, Options,
                        getEffectiveRelocModel(RM), CM, OL),
      TLOF(make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, FS, *this) {

  // Although it's still not clear from a performance point of view whether or
  // not we need 'setRequiresStructuredCFG', we're enabling it because it
  // disables certain machine-level transformations in MachineBlockPlacement.
  // At The problematic transformation which prompted us to enable this again
  // was tail merging, but this disables other transformations as well.
  setRequiresStructuredCFG(true);
  initAsmInfo();
  //setAsmVerbosityDefault(true);
}


TargetIRAnalysis CSATargetMachine::getTargetIRAnalysis() {
  return TargetIRAnalysis([this](const Function &F) {
    return TargetTransformInfo(CSATTIImpl(this, F));
  });
}



CSATargetMachine::~CSATargetMachine() {}

namespace {
/// CSA Code Generator Pass Configuration Options.
class CSAPassConfig : public TargetPassConfig {
public:
  CSAPassConfig(CSATargetMachine *TM, legacy::PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  CSATargetMachine &getCSATargetMachine() const {
    return getTM<CSATargetMachine>();
  }

  bool addInstSelector() override {

    // Add the pass to lower memset/memmove/memcpy
    addPass(createLowerAggrCopies());

    // Add the pass to convert Fortran "builtin" calls
    addPass(createFortranIntrinsics());

    // Install an instruction selector.
    addPass(createCSAISelDag(getCSATargetMachine(), getOptLevel()));

    // Add the pass to expand inline assembly.
    addPass(createCSAExpandInlineAsmPass(), false, true);

    return false;
  }



  bool addPreISel() override {
    //addPass(createUnifyFunctionExitNodesPass());
    addPass(createLowerSwitchPass());
    if (CSAStructurizeCFG) {
      addPass(createStructurizeCFGPass(false));
      //remove the single input phi and constant branch created from StructurizeCFG
      addPass(createInstructionCombiningPass());
    }
    return false;
  }


#define DEBUG_TYPE "csa-convert-control"
  void addPreRegAlloc() override {
    std::string Banner;
#if 1
    Banner = std::string("Before Machine CDG Pass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createControlDepenceGraph(), false);
    Banner = std::string("After Machine CDG Pass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createCSAMemopOrderingPass(), false);
    Banner = std::string("After CSAMemopOrderingPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createCSACvtCFDFPass(), false);
    Banner = std::string("After CSACvtCFDFPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createCSADFParLoopPass(), false);
    Banner = std::string("After CSADFParLoopPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createCSAOptDFPass(), false);
    Banner = std::string("After CSAOptDFPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createCSARedundantMovElimPass(), false);
    Banner = std::string("After CSARedundantMovElim");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createCSADeadInstructionElimPass(), false);
    Banner = std::string("After CSADeadInstructionElim");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createCSANormalizeDebugPass(), false);
    Banner = std::string("After CSANormalizeDebug");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    if (RunCSAStatistics) {
      addPass(createCSAStatisticsPass(), false);
    }
#else
    Banner = std::string("Before CSAConvertControlPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));
    addPass(createCSAConvertControlPass(), false);
    Banner = std::string("After CSAConvertControlPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    Banner = std::string("Before CSAOptDFPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));

    addPass(createCSAOptDFPass(), false);
    Banner = std::string("After CSAOptDFPass");
    DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));
#endif

  }

  void addPostRegAlloc() override {
    addPass(createCSAAllocUnitPass(), false);
  }

  void addIRPasses() override {
    // Pass call onto parent
    TargetPassConfig::addIRPasses();
  }

}; // class CSAPassConfig

} // namespace

TargetPassConfig *CSATargetMachine::createPassConfig(legacy::PassManagerBase &PM) {
  CSAPassConfig *PassConfig = new CSAPassConfig(this, PM);
  return PassConfig;
}
