//===-- CSATargetMachine.cpp - Define TargetMachine for CSA ---------------===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// Top-level implementation for the CSA target.
//
//===----------------------------------------------------------------------===//

#include "CSATargetMachine.h"
#include "CSAAsmWrapOstream.h"
#include "CSAFortranIntrinsics.h"
#include "CSAIROpt.h"
#include "CSAIntrinsicCleaner.h"
#include "CSALoopIntrinsicExpander.h"
#include "CSALowerAggrCopies.h"
#include "CSAUtils.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/CodeGen/TargetLowering.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/LoopSimplify.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Transforms/Utils/UnifyFunctionExitNodes.h"

using namespace llvm;

static cl::opt<int> RunCSAStatistics(
  "csa-run-statistics", cl::Hidden,
  cl::desc("CSA Specific: collect statistics for DF instructions"),
  cl::init(0));

static cl::opt<int>
  CSAStructurizeCFG("csa-structurize-cfg", cl::Hidden,
                    cl::desc("CSA Specific: leverage llvm StructurizeCFG"),
                    cl::init(1));

// Helper function to build a DataLayout string
static std::string computeDataLayout() { return "e-m:e-i64:64-n32:64"; }

namespace llvm {
void initializeCSALowerAggrCopiesPass(PassRegistry &);
void initializeCSAFortranIntrinsicsPass(PassRegistry &);
void initializeCSAInnerLoopPrepPass(PassRegistry &);
} // namespace llvm

extern "C" void LLVMInitializeCSATarget() {
  // Register the target.
  RegisterTargetMachine<CSATargetMachine> X(getTheCSATarget());

  // The original comment in the CSA target says this optimization
  // is placed here because it is too target-specific.
  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeCSAOptDFPassPass(PR);
  initializeCSAInnerLoopPrepPass(PR);
  initializeCSALowerAggrCopiesPass(PR);
  initializeCSAFortranIntrinsicsPass(PR);
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::Static;
  return *RM;
}

static CodeModel::Model getEffectiveCodeModel(Optional<CodeModel::Model> CM) {
  if (CM)
    return *CM;
  return CodeModel::Small;
}

CSATargetMachine::CSATargetMachine(const Target &T, const Triple &TT,
                                   StringRef CPU, StringRef FS,
                                   const TargetOptions &Options,
                                   Optional<Reloc::Model> RM,
                                   Optional<CodeModel::Model> CM,
                                   CodeGenOpt::Level OL,
                                   bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(), TT, CPU, FS, Options,
                        getEffectiveRelocModel(RM), getEffectiveCodeModel(CM), OL),
      TLOF(make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, CPU, FS, *this) {

  // Although it's still not clear from a performance point of view whether or
  // not we need 'setRequiresStructuredCFG', we're enabling it because it
  // disables certain machine-level transformations in MachineBlockPlacement.
  // At The problematic transformation which prompted us to enable this again
  // was tail merging, but this disables other transformations as well.
  setRequiresStructuredCFG(true);
  initAsmInfo();
  // setAsmVerbosityDefault(true);
}

CSATargetMachine::~CSATargetMachine() {}

namespace {
/// CSA Code Generator Pass Configuration Options.
class CSAPassConfig : public TargetPassConfig {
public:
  CSAPassConfig(CSATargetMachine &TM, legacy::PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {
    disablePass(&MachineLICMID);
    disablePass(&MachineCopyPropagationID);
    // TailDuplication destroys natural loop form, don't do it for CSA.
    disablePass(&EarlyTailDuplicateID);
  }

  CSATargetMachine &getCSATargetMachine() const {
    return getTM<CSATargetMachine>();
  }

  bool addInstSelector() override {

    // Add the pass to lower memset/memmove/memcpy
    addPass(createLowerAggrCopies());

    // Install an instruction selector.
    addPass(createCSAISelDag(getCSATargetMachine(), getOptLevel()));

    // Add the pass to expand inline assembly.
    addPass(createCSAExpandInlineAsmPass(), false, true);

    return false;
  }

  bool addPreISel() override {
    // addPass(createUnifyFunctionExitNodesPass());
    addPass(createLowerSwitchPass());
    // Add a pass to generate more candidates for reduction operations
    addPass(createCSAIRReductionOptPass());
    if (CSAStructurizeCFG) {
      addPass(createStructurizeCFGPass(false));
      // remove the single input phi and constant branch created from
      // StructurizeCFG
      addPass(createInstructionCombiningPass());
      addPass(createCFGSimplificationPass());
    }
    // Add a pass to identify and prepare inner loops for pipelinling. This
    // only happens at O1+ so as to avoid requiring excessive additional
    // analyses at O0.
    if (getOptLevel() != CodeGenOpt::None) {
      addPass(createCSAInnerLoopPrepPass());
      // Add streaming memory reductions.
      addPass(createCSAStreamingMemoryPrepPass());
    }

    // Remove any remaining intrinsics which should not go through instruction
    // selection
    addPass(createCSAIntrinsicCleanerPass());
    // Add pass to replace alloca instructions
    addPass(createCSAReplaceAllocaWithMallocPass(getCSATargetMachine()));
    addPass(createGlobalDCEPass());
    // simplify loop has to be run last, data flow converter assume natural loop
    // format, with prehdr etc...
    addPass(createLoopSimplifyPass());
    return false;
  }

#define DEBUG_TYPE "csa-convert-control"
  void addPreRegAlloc() override {
    std::string Banner;
    Banner = std::string("Before Machine CDG Pass");
    LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                       false));

    addPass(createControlDepenceGraph(), false);
    Banner = std::string("After Machine CDG Pass");
    LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                       false));

    if (getOptLevel() != CodeGenOpt::None) {
      //
      // TODO (vzakhari 3/1/2018): decide what to do at O0.
      //       Currently, RegAllocFast used at O0 does not work properly
      //       for CI register classes.  This workaround mimics
      //       the pathfinding compiler behavior and disables cv-to-df
      //       conversion (which would introduce CI).
      //
      addPass(createCSAMemopOrderingPass());
      Banner = std::string("After CSAMemopOrderingPass");
      LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                         false));
    }

    addPass(createCSARASReplayableLoadsDetectionPass(), false);
    Banner = std::string("After CSARASReplayableLoadsDetection");
    LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                       false));

    addPass(createCSANameLICsPass(), false);

    if (getOptLevel() != CodeGenOpt::None) {
      addPass(createCSACvtCFDFPass(), false);
      Banner = std::string("After CSACvtCFDFPass");
      LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                         false));
    }

    if (RunCSAStatistics) {
      addPass(createCSAStatisticsPass(), false);
    }

    addPass(createCSAOptDFPass(), false);
    Banner = std::string("After CSAOptDFPass");
    LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                       false));

    addPass(createCSADataflowCanonicalizationPass(), false);
    Banner = std::string("After CSADataflowCanonicalizationPass");
    LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                       false));

    addPass(createCSAStreamingMemoryConversionPass(), false);
    Banner = std::string("After CSAStreamingMemoryConversionPass");
    LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                       false));

    addPass(createCSAMultiSeqPass(), false);
    Banner = std::string("After CSAMultiSeqPass");
    LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                       false));

    
    addPass(createCSARedundantMovElimPass(), false);
    Banner = std::string("After CSARedundantMovElim");
    LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                       false));

    addPass(createCSADeadInstructionElimPass(), false);
    Banner = std::string("After CSADeadInstructionElim");
    LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                       false));

    addPass(createCSAReassocReducPass(), false);
    Banner = std::string("After CSAReassocReducPass");
    LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                       false));

    addPass(createCSANormalizeDebugPass(), false);
    Banner = std::string("After CSANormalizeDebug");
    LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner),
                       false));

    // Register coalescing causes issues with our def-after-use nature of
    // dataflow.
    disablePass(&RegisterCoalescerID);
  }

// Last call in TargetPassConfig.cpp to disable passes.  
// If we don't disable here some other passes may add after disable
  void addPostRegAlloc() override {
    addPass(createCSAAllocUnitPass(), false);

    // These functions don't like vregs.
    disablePass(&ShrinkWrapID);
//RAVI    disablePass(&MachineCopyPropagationID);
    disablePass(&PostRASchedulerID);
    disablePass(&FuncletLayoutID);
    disablePass(&StackMapLivenessID);
    disablePass(&LiveDebugValuesID);
    disablePass(&PatchableFunctionID);
  }

  void addPreEmitPass2() override {
    if (csa_utils::isAlwaysDataFlowLinkageSet()) {
      addPass(createCSAProcCallsPass(), false);
      std::string Banner = std::string("After CSAProcCallsPass");
      LLVM_DEBUG(addPass(createMachineFunctionPrinterPass(errs(), Banner), false));
    }

  }

  void addIRPasses() override {
    // Do any necessary atomic expansion according to Subtarget features.
    addPass(createAtomicExpandPass());

    // Pass call onto parent
    TargetPassConfig::addIRPasses();
  }

}; // class CSAPassConfig

} // namespace

TargetPassConfig *
CSATargetMachine::createPassConfig(legacy::PassManagerBase &PM) {
  CSAPassConfig *PassConfig = new CSAPassConfig(*this, PM);
  return PassConfig;
}

void CSATargetMachine::adjustPassManager(PassManagerBuilder &PMB) {
  PMB.addExtension(PassManagerBuilder::EP_EarlyAsPossible,
                   [](const PassManagerBuilder &, legacy::PassManagerBase &PM) {

                     // Add the pass to convert Fortran "builtin" calls
                     PM.add(createFortranIntrinsics());

                     // Add the pass to expand loop intrinsics
                     PM.add(createSROAPass());
                     PM.add(createLoopSimplifyPass());
                     PM.add(createLICMPass());
                     PM.add(createCSALoopIntrinsicExpanderPass());
                   });
}

// This function is copied from lib/CodeGen/LLVMTargetMachine.cpp because it is
// not accessible here - feel free to update this if the original changes.
// TODO (dwoodwor 6/29/2018): It would be good if we didn't have to duplicate
//       this code but it isn't clear how that would happen without modifying
//       the interface of LLVMTargetMachine. Ideally we'd just want a virtual
//       function that we could override to change how the formatted_raw_ostream
//       passed on to the MCAsmStreamer is constructed, though this is
//       admittedly a pretty niche thing to need to do. We'll also be able to
//       get rid of all of this once we have a proper binary format, so I don't
//       think it's worth getting something into common code just for this use
//       case.
static MCContext *
addPassesToGenerateCode(LLVMTargetMachine *TM, PassManagerBase &PM,
                        bool DisableVerify, bool &WillCompleteCodeGenPipeline,
                        raw_pwrite_stream &Out, MachineModuleInfo *MMI) {
  // Targets may override createPassConfig to provide a target-specific
  // subclass.
  TargetPassConfig *PassConfig = TM->createPassConfig(PM);
  // Set PassConfig options provided by TargetMachine.
  PassConfig->setDisableVerify(DisableVerify);
  WillCompleteCodeGenPipeline = PassConfig->willCompleteCodeGenPipeline();
  PM.add(PassConfig);
  if (!MMI)
    MMI = new MachineModuleInfo(TM);
  PM.add(MMI);

  if (PassConfig->addISelPasses())
    return nullptr;
  PassConfig->addMachinePasses();
  PassConfig->setInitialized();
  if (!WillCompleteCodeGenPipeline)
    PM.add(createPrintMIRPass(Out));

  return &MMI->getContext();
}

// This function is also mostly copied from lib/CodeGen/LLVMTargetMachine.cpp.
bool CSATargetMachine::addAsmPrinterWithAsmWrapping(PassManagerBase &PM,
                                                    raw_pwrite_stream &Out,
                                                    raw_pwrite_stream *DwoOut,
                                                    CodeGenFileType FileType,
                                                    MCContext &Context) {
  if (Options.MCOptions.MCSaveTempLabels)
    Context.setAllowTemporaryLabels(false);

  const MCSubtargetInfo &STI = *getMCSubtargetInfo();
  const MCAsmInfo &MAI       = *getMCAsmInfo();
  const MCRegisterInfo &MRI  = *getMCRegisterInfo();
  const MCInstrInfo &MII     = *getMCInstrInfo();

  std::unique_ptr<MCStreamer> AsmStreamer;

  switch (FileType) {
  case CGFT_AssemblyFile: {
    MCInstPrinter *InstPrinter = getTarget().createMCInstPrinter(
      getTargetTriple(), MAI.getAssemblerDialect(), MAI, MII, MRI);

    // Create a code emitter if asked to show the encoding.
    std::unique_ptr<MCCodeEmitter> MCE;
    if (Options.MCOptions.ShowMCEncoding)
      MCE.reset(getTarget().createMCCodeEmitter(MII, MRI, Context));

    std::unique_ptr<MCAsmBackend> MAB(
      getTarget().createMCAsmBackend(STI, MRI, Options.MCOptions));
    auto FOut     = wrapStreamForCSAAsmWrapping(Out);
    MCStreamer *S = getTarget().createAsmStreamer(
      Context, std::move(FOut), Options.MCOptions.AsmVerbose,
      Options.MCOptions.MCUseDwarfDirectory, InstPrinter, std::move(MCE),
      std::move(MAB), Options.MCOptions.ShowMCInst);
    AsmStreamer.reset(S);
    break;
  }
  case CGFT_ObjectFile: {
    // This isn't going to work for CSA yet - print a warning to give the user
    // a better error message while we're here.
    errs().changeColor(raw_ostream::RED, true);
    errs() << "BINARY OUTPUT ISN'T SUPPORTED FOR CSA YET; DID YOU PASS -S?";
    errs().resetColor();
    errs() << "\n";

    // Create the code emitter for the target if it exists.  If not, .o file
    // emission fails.
    MCCodeEmitter *MCE = getTarget().createMCCodeEmitter(MII, MRI, Context);
    MCAsmBackend *MAB =
      getTarget().createMCAsmBackend(STI, MRI, Options.MCOptions);
    if (!MCE || !MAB)
      return true;

    // Don't waste memory on names of temp labels.
    Context.setUseNamesOnTempLabels(false);

    Triple T(getTargetTriple().str());
    AsmStreamer.reset(getTarget().createMCObjectStreamer(
      T, Context, std::unique_ptr<MCAsmBackend>(MAB),
      DwoOut ? MAB->createDwoObjectWriter(Out, *DwoOut)
             : MAB->createObjectWriter(Out),
      std::unique_ptr<MCCodeEmitter>(MCE), STI, Options.MCOptions.MCRelaxAll,
      Options.MCOptions.MCIncrementalLinkerCompatible,
      /*DWARFMustBeAtTheEnd*/ true));
    break;
  }
  case CGFT_Null:
    // The Null output is intended for use for performance analysis and testing,
    // not real users.
    AsmStreamer.reset(getTarget().createNullStreamer(Context));
    break;
  }

  // Create the AsmPrinter, which takes ownership of AsmStreamer if successful.
  FunctionPass *Printer =
    getTarget().createAsmPrinter(*this, std::move(AsmStreamer));
  if (!Printer)
    return true;

  PM.add(Printer);
  return false;
}

// This function is also mostly copied from lib/CodeGen/LLVMTargetMachine.cpp.
bool CSATargetMachine::addPassesToEmitFile(
  PassManagerBase &PM, raw_pwrite_stream &Out, raw_pwrite_stream *DwoOut,
  CodeGenFileType FileType, bool DisableVerify, MachineModuleInfo *MMI) {
  // Add common CodeGen passes.
  bool WillCompleteCodeGenPipeline = true;
  MCContext *Context               = addPassesToGenerateCode(
    this, PM, DisableVerify, WillCompleteCodeGenPipeline, Out, MMI);
  if (!Context)
    return true;

  if (WillCompleteCodeGenPipeline &&
      addAsmPrinterWithAsmWrapping(PM, Out, DwoOut, FileType, *Context))
    return true;

  PM.add(createFreeMachineFunctionPass());
  return false;
}
